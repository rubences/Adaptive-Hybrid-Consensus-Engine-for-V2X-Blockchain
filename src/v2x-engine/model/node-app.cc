/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 */

#include "node-app.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NodeApp");

NS_OBJECT_ENSURE_REGISTERED (NodeApp);

TypeId
NodeApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NodeApp")
    .SetParent<Application> ()
    .SetGroupName ("V2xEngine")
    .AddConstructor<NodeApp> ();
  return tid;
}

NodeApp::NodeApp ()
  : m_socket (0),
    m_vanetEngine (0),
    m_nodeId (0),
    m_port (9999),
    m_packetInterval (0.1), // 100ms
    m_packetSize (200),     // 200 bytes for CAM
    m_txCounter (0)
{
  NS_LOG_FUNCTION (this);
}

NodeApp::~NodeApp ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
}

void
NodeApp::SetVanetEngine (Ptr<VanetEngineHelper> engine)
{
  m_vanetEngine = engine;
}

uint32_t
NodeApp::GetNodeId () const
{
  return m_nodeId;
}

void
NodeApp::SetPacketInterval (double interval)
{
  m_packetInterval = interval;
}

void
NodeApp::SetPort (uint16_t port)
{
  m_port = port;
}

Transaction
NodeApp::GenerateTransaction (const std::string& type)
{
  NS_LOG_FUNCTION (this << type);
  
  Transaction tx;
  tx.txId = m_txCounter++;
  tx.sourceNodeId = m_nodeId;
  tx.timestamp = Simulator::Now ().GetSeconds ();
  
  // Create transaction data (CAM or DENM)
  std::ostringstream oss;
  oss << type << "_" << m_nodeId << "_" << tx.txId;
  
  if (type == "CAM")
    {
      // CAM: Cooperative Awareness Message with position info
      Vector pos = GetPosition ();
      oss << "_pos(" << pos.x << "," << pos.y << "," << pos.z << ")";
    }
  else if (type == "DENM")
    {
      // DENM: Decentralized Environmental Notification Message (event-based)
      oss << "_event(hazard_detected)";
    }
  
  tx.data = oss.str ();
  tx.delay = 0.0; // Will be calculated when processing
  
  return tx;
}

Vector
NodeApp::GetPosition () const
{
  Ptr<MobilityModel> mobility = GetNode ()->GetObject<MobilityModel> ();
  if (mobility)
    {
      return mobility->GetPosition ();
    }
  return Vector (0, 0, 0);
}

std::vector<Transaction>
NodeApp::GetPendingTransactions () const
{
  return m_pendingTxs;
}

void
NodeApp::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  
  m_nodeId = GetNode ()->GetId ();
  
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&NodeApp::ReceivePacket, this));
    }
  
  // Schedule first transmission
  ScheduleTransmit ();
  
  NS_LOG_INFO ("NodeApp started on node " << m_nodeId);
}

void
NodeApp::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  
  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }
  
  if (m_socket)
    {
      m_socket->Close ();
    }
  
  NS_LOG_INFO ("NodeApp stopped on node " << m_nodeId);
}

void
NodeApp::ScheduleTransmit ()
{
  NS_LOG_FUNCTION (this);
  
  m_sendEvent = Simulator::Schedule (Seconds (m_packetInterval), 
                                      &NodeApp::SendPacket, 
                                      this);
}

void
NodeApp::SendPacket ()
{
  NS_LOG_FUNCTION (this);
  
  // Generate a CAM transaction (periodic beacons)
  Transaction tx = GenerateTransaction ("CAM");
  m_pendingTxs.push_back (tx);
  
  // Occasionally generate DENM (5% chance)
  if (m_txCounter % 20 == 0)
    {
      Transaction denmTx = GenerateTransaction ("DENM");
      m_pendingTxs.push_back (denmTx);
    }
  
  // Update engine state with current position and pending transactions
  UpdateEngineState ();
  
  // Create and send packet (broadcast to neighbors)
  std::ostringstream oss;
  oss << "CAM_" << m_nodeId << "_" << tx.txId;
  std::string msg = oss.str ();
  
  Ptr<Packet> packet = Create<Packet> ((uint8_t*) msg.c_str (), msg.length ());
  
  // Broadcast to all nodes (in real scenario, this would be constrained by radio range)
  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), m_port);
  m_socket->SendTo (packet, 0, remote);
  
  NS_LOG_DEBUG ("Node " << m_nodeId << " sent CAM packet. Pending txs: " << m_pendingTxs.size ());
  
  // Schedule next transmission
  ScheduleTransmit ();
}

void
NodeApp::ReceivePacket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  
  Ptr<Packet> packet;
  Address from;
  
  while ((packet = socket->RecvFrom (from)))
    {
      uint8_t buffer[1024];
      uint32_t size = packet->CopyData (buffer, sizeof (buffer));
      std::string msg ((char*) buffer, size);
      
      NS_LOG_DEBUG ("Node " << m_nodeId << " received packet: " << msg);
      
      // In a real implementation, this would parse and process the received data
      // For simulation purposes, we assume blocks are received and validated
    }
}

void
NodeApp::ProcessReceivedBlock (const BlockCandidate& block)
{
  NS_LOG_FUNCTION (this << block.blockId);
  
  // Validate the block using the VANET engine
  if (m_vanetEngine && m_vanetEngine->GetActiveConsensusModule ())
    {
      bool valid = m_vanetEngine->GetActiveConsensusModule ()->ValidateBlockCandidate (block);
      
      if (valid)
        {
          NS_LOG_INFO ("Node " << m_nodeId << " validated block " << block.blockId);
          
          // Remove transactions that are now in the block
          for (const auto& tx : block.transactions)
            {
              auto it = std::find_if (m_pendingTxs.begin (), m_pendingTxs.end (),
                                      [&tx](const Transaction& t) { return t.txId == tx.txId; });
              if (it != m_pendingTxs.end ())
                {
                  m_pendingTxs.erase (it);
                }
            }
        }
      else
        {
          NS_LOG_WARN ("Node " << m_nodeId << " rejected invalid block " << block.blockId);
        }
    }
}

void
NodeApp::UpdateEngineState ()
{
  if (m_vanetEngine)
    {
      Vector position = GetPosition ();
      m_vanetEngine->UpdateNodeState (m_nodeId, position, m_pendingTxs);
    }
}

} // namespace ns3

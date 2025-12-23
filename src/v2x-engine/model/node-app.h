/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#ifndef NODE_APP_H
#define NODE_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ptr.h"
#include "ns3/vector.h"
#include "vanet-engine-helper.h"
#include "consensus-module.h"

namespace ns3 {

/**
 * \brief NS-3 Application for V2X blockchain nodes
 * 
 * This application runs on each vehicle node and:
 * 1. Generates CAM (Cooperative Awareness Messages) and DENM (Decentralized Environmental Notification Messages)
 * 2. Receives blocks from other nodes
 * 3. Calls the consensus engine to validate and propagate blocks
 * 4. Integrates with IEEE 802.11p (WAVE) for V2V communication
 */
class NodeApp : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  NodeApp ();

  /**
   * \brief Destructor
   */
  virtual ~NodeApp ();

  /**
   * \brief Set the VANET engine helper
   * \param engine Pointer to the VANET engine
   */
  void SetVanetEngine (Ptr<VanetEngineHelper> engine);

  /**
   * \brief Get the node ID
   * \return Node identifier
   */
  uint32_t GetNodeId () const;

  /**
   * \brief Set packet send interval for CAM messages
   * \param interval Interval in seconds (default 0.1 for 100ms beacons)
   */
  void SetPacketInterval (double interval);

  /**
   * \brief Set the port for receiving messages
   * \param port Port number
   */
  void SetPort (uint16_t port);

  /**
   * \brief Generate a transaction (CAM or DENM)
   * \param type Message type ("CAM" or "DENM")
   * \return Generated transaction
   */
  Transaction GenerateTransaction (const std::string& type);

  /**
   * \brief Get current node position
   * \return Position vector
   */
  Vector GetPosition () const;

  /**
   * \brief Get pending transactions at this node
   * \return Vector of pending transactions
   */
  std::vector<Transaction> GetPendingTransactions () const;

protected:
  /**
   * \brief Application start callback
   */
  virtual void StartApplication (void) override;

  /**
   * \brief Application stop callback
   */
  virtual void StopApplication (void) override;

private:
  /**
   * \brief Schedule next CAM transmission
   */
  void ScheduleTransmit ();

  /**
   * \brief Send a CAM/DENM packet
   */
  void SendPacket ();

  /**
   * \brief Receive a packet (block or transaction)
   * \param socket Socket that received the packet
   */
  void ReceivePacket (Ptr<Socket> socket);

  /**
   * \brief Process received block
   * \param block Block to process
   */
  void ProcessReceivedBlock (const BlockCandidate& block);

  /**
   * \brief Update node state in the VANET engine
   */
  void UpdateEngineState ();

  Ptr<Socket> m_socket;                 ///< Socket for communication
  Ptr<VanetEngineHelper> m_vanetEngine; ///< Reference to VANET engine
  
  uint32_t m_nodeId;                    ///< Node identifier
  uint16_t m_port;                      ///< Communication port
  double m_packetInterval;              ///< Interval between CAM transmissions (s)
  uint32_t m_packetSize;                ///< Size of CAM packets (bytes)
  
  EventId m_sendEvent;                  ///< Event for next transmission
  std::vector<Transaction> m_pendingTxs; ///< Local pending transactions
  uint32_t m_txCounter;                 ///< Transaction counter
};

} // namespace ns3

#endif /* NODE_APP_H */

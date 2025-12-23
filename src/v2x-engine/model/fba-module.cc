/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 */

#include "fba-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <sstream>
#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FBAModule");

NS_OBJECT_ENSURE_REGISTERED (FBAModule);

TypeId
FBAModule::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FBAModule")
    .SetParent<ConsensusModule> ()
    .SetGroupName ("V2xEngine")
    .AddConstructor<FBAModule> ();
  return tid;
}

FBAModule::FBAModule ()
  : m_energyPerSignature (1e-6), // 1 micro-Joule per signature
    m_quorumThreshold (0.67)     // 2/3 Byzantine fault tolerance
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
}

FBAModule::~FBAModule ()
{
  NS_LOG_FUNCTION (this);
}

BlockCandidate
FBAModule::GenerateBlockCandidate (const std::vector<Transaction>& transactions,
                                    uint32_t nodeId,
                                    const std::string& previousHash)
{
  NS_LOG_FUNCTION (this << nodeId);
  
  double startTime = Simulator::Now ().GetSeconds ();
  
  BlockCandidate block;
  block.blockId = m_random->GetInteger (0, 0xFFFFFFFF);
  block.proposerNodeId = nodeId;
  block.transactions = transactions;
  block.timestamp = startTime;
  block.consensusType = "FBA";
  block.difficulty = 0; // FBA doesn't use difficulty
  block.previousBlockHash = previousHash;
  
  // Create block data string
  std::ostringstream oss;
  oss << block.blockId << nodeId << previousHash << block.timestamp;
  for (const auto& tx : transactions)
    {
      oss << tx.txId;
    }
  std::string blockData = oss.str ();
  
  // Simulate quorum voting
  bool quorumReached = SimulateQuorumVoting (blockData);
  
  // Create block hash with quorum signatures
  std::hash<std::string> hasher;
  size_t hashValue = hasher (blockData);
  std::ostringstream hashOss;
  hashOss << "FBA_" << std::hex << std::setfill ('0') << std::setw (16) << hashValue;
  block.blockHash = hashOss.str ();
  
  // Energy consumption: E_crypto = e_sig * n_quorum_members
  uint32_t quorumSize = static_cast<uint32_t> (m_quorumMembers.size () * m_quorumThreshold);
  m_energyConsumption = m_energyPerSignature * quorumSize;
  
  // FBA latency includes network round-trips for quorum agreement
  // Typically higher than PoS but lower than PoW
  m_latency = 5e-3 + (quorumSize * 1e-4); // Base 5ms + per-node overhead
  
  NS_LOG_INFO ("FBA block generated: " << block.blockId 
               << " with quorum of " << quorumSize << " nodes"
               << ", energy: " << m_energyConsumption << " J"
               << ", latency: " << m_latency << " s");
  
  return block;
}

bool
FBAModule::ValidateBlockCandidate (const BlockCandidate& block)
{
  NS_LOG_FUNCTION (this);
  
  if (block.consensusType != "FBA")
    {
      NS_LOG_WARN ("Block is not FBA type");
      return false;
    }
  
  // Create block data for verification
  std::ostringstream oss;
  oss << block.blockId << block.proposerNodeId << block.previousBlockHash << block.timestamp;
  for (const auto& tx : block.transactions)
    {
      oss << tx.txId;
    }
  std::string blockData = oss.str ();
  
  // Verify quorum signatures
  bool valid = SimulateQuorumVoting (blockData);
  
  // Validation energy: verify quorum signatures
  uint32_t quorumSize = static_cast<uint32_t> (m_quorumMembers.size () * m_quorumThreshold);
  m_energyConsumption = m_energyPerSignature * quorumSize;
  m_latency = 2e-3; // 2 milliseconds for verification
  
  NS_LOG_INFO ("FBA block validation: " << (valid ? "VALID" : "INVALID"));
  
  return valid;
}

std::string
FBAModule::GetConsensusType () const
{
  return "FBA";
}

void
FBAModule::SetEnergyPerSignature (double energyPerSig)
{
  m_energyPerSignature = energyPerSig;
}

double
FBAModule::GetEnergyPerSignature () const
{
  return m_energyPerSignature;
}

void
FBAModule::SetQuorumThreshold (double threshold)
{
  m_quorumThreshold = threshold;
}

double
FBAModule::GetQuorumThreshold () const
{
  return m_quorumThreshold;
}

void
FBAModule::AddQuorumMember (uint32_t nodeId)
{
  m_quorumMembers.insert (nodeId);
}

uint32_t
FBAModule::GetQuorumSize () const
{
  return m_quorumMembers.size ();
}

bool
FBAModule::SimulateQuorumVoting (const std::string& blockData)
{
  if (m_quorumMembers.empty ())
    {
      // If no quorum defined, assume single-node approval
      return true;
    }
  
  // Simulate voting: each node votes with probability based on network conditions
  // In real FBA, this would involve actual network communication
  uint32_t votesReceived = 0;
  uint32_t requiredVotes = static_cast<uint32_t> (m_quorumMembers.size () * m_quorumThreshold);
  
  for (uint32_t nodeId : m_quorumMembers)
    {
      // Simulate vote with 95% success rate (5% Byzantine/network failures)
      if (m_random->GetValue () < 0.95)
        {
          votesReceived++;
        }
    }
  
  return votesReceived >= requiredVotes;
}

std::string
FBAModule::CreateVoteSignature (uint32_t nodeId, const std::string& blockData)
{
  // Simulate vote signature
  std::ostringstream oss;
  oss << nodeId << blockData << "vote";
  
  std::hash<std::string> hasher;
  size_t hashValue = hasher (oss.str ());
  
  std::ostringstream sigOss;
  sigOss << "VOTE_" << std::hex << std::setfill ('0') << std::setw (16) << hashValue;
  return sigOss.str ();
}

} // namespace ns3

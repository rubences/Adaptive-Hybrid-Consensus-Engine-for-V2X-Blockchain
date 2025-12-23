/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 */

#include "pos-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <sstream>
#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PoSModule");

NS_OBJECT_ENSURE_REGISTERED (PoSModule);

TypeId
PoSModule::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PoSModule")
    .SetParent<ConsensusModule> ()
    .SetGroupName ("V2xEngine")
    .AddConstructor<PoSModule> ();
  return tid;
}

PoSModule::PoSModule ()
  : m_energyPerSignature (1e-6) // 1 micro-Joule per signature
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
}

PoSModule::~PoSModule ()
{
  NS_LOG_FUNCTION (this);
}

BlockCandidate
PoSModule::GenerateBlockCandidate (const std::vector<Transaction>& transactions,
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
  block.consensusType = "PoS";
  block.difficulty = 0; // PoS doesn't use difficulty
  block.previousBlockHash = previousHash;
  
  // Create block data string
  std::ostringstream oss;
  oss << block.blockId << nodeId << previousHash << block.timestamp;
  for (const auto& tx : transactions)
    {
      oss << tx.txId;
    }
  std::string blockData = oss.str ();
  
  // Create signature
  std::string signature = CreateSignature (blockData);
  block.blockHash = signature;
  
  // Energy consumption: E_crypto = e_sig * n_sig
  // One signature for block creation
  m_energyConsumption = m_energyPerSignature;
  
  // PoS latency is much lower than PoW (no mining required)
  m_latency = 1e-3; // 1 millisecond for signature creation
  
  NS_LOG_INFO ("PoS block generated: " << block.blockId 
               << ", energy: " << m_energyConsumption << " J"
               << ", latency: " << m_latency << " s");
  
  return block;
}

bool
PoSModule::ValidateBlockCandidate (const BlockCandidate& block)
{
  NS_LOG_FUNCTION (this);
  
  if (block.consensusType != "PoS")
    {
      NS_LOG_WARN ("Block is not PoS type");
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
  
  // Verify signature
  bool valid = VerifySignature (blockData, block.blockHash);
  
  // Validation energy: one signature verification
  m_energyConsumption = m_energyPerSignature;
  m_latency = 5e-4; // 0.5 milliseconds for verification
  
  NS_LOG_INFO ("PoS block validation: " << (valid ? "VALID" : "INVALID"));
  
  return valid;
}

std::string
PoSModule::GetConsensusType () const
{
  return "PoS";
}

void
PoSModule::SetEnergyPerSignature (double energyPerSig)
{
  m_energyPerSignature = energyPerSig;
}

double
PoSModule::GetEnergyPerSignature () const
{
  return m_energyPerSignature;
}

void
PoSModule::SetNodeStake (uint32_t nodeId, double stake)
{
  m_stakes[nodeId] = stake;
}

double
PoSModule::GetNodeStake (uint32_t nodeId) const
{
  auto it = m_stakes.find (nodeId);
  if (it != m_stakes.end ())
    {
      return it->second;
    }
  return 0.0;
}

uint32_t
PoSModule::SelectValidator ()
{
  // Stake-weighted random selection
  double totalStake = 0.0;
  for (const auto& pair : m_stakes)
    {
      totalStake += pair.second;
    }
  
  if (totalStake == 0.0)
    {
      // If no stakes defined, select random node
      if (!m_stakes.empty ())
        {
          auto it = m_stakes.begin ();
          std::advance (it, m_random->GetInteger (0, m_stakes.size () - 1));
          return it->first;
        }
      return 0;
    }
  
  // Select based on stake proportion
  double rand = m_random->GetValue (0.0, totalStake);
  double cumulative = 0.0;
  
  for (const auto& pair : m_stakes)
    {
      cumulative += pair.second;
      if (rand <= cumulative)
        {
          return pair.first;
        }
    }
  
  // Fallback
  return m_stakes.begin ()->first;
}

std::string
PoSModule::CreateSignature (const std::string& data)
{
  // Simulate digital signature (in reality would use ECDSA or similar)
  std::hash<std::string> hasher;
  size_t hashValue = hasher (data + "signature");
  
  std::ostringstream oss;
  oss << "SIG_" << std::hex << std::setfill ('0') << std::setw (16) << hashValue;
  return oss.str ();
}

bool
PoSModule::VerifySignature (const std::string& data, const std::string& signature)
{
  // Simulate signature verification
  std::string expectedSig = CreateSignature (data);
  
  // In simulation, we assume signatures are valid with high probability
  // To simulate occasional invalid signatures, we can use random check
  return (signature.substr (0, 4) == "SIG_" || m_random->GetValue () > 0.01);
}

} // namespace ns3

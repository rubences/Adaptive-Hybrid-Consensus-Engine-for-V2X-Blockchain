/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 */

#include "pow-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PoWModule");

NS_OBJECT_ENSURE_REGISTERED (PoWModule);

TypeId
PoWModule::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PoWModule")
    .SetParent<ConsensusModule> ()
    .SetGroupName ("V2xEngine")
    .AddConstructor<PoWModule> ();
  return tid;
}

PoWModule::PoWModule ()
  : m_energyPerHash (5e-6) // 5 micro-Joules per hash
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
}

PoWModule::~PoWModule ()
{
  NS_LOG_FUNCTION (this);
}

BlockCandidate
PoWModule::GenerateBlockCandidate (const std::vector<Transaction>& transactions,
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
  block.consensusType = "PoW";
  block.difficulty = m_difficulty;
  block.previousBlockHash = previousHash;
  
  // Create block data string
  std::ostringstream oss;
  oss << block.blockId << nodeId << previousHash << block.timestamp;
  for (const auto& tx : transactions)
    {
      oss << tx.txId;
    }
  std::string blockData = oss.str ();
  
  // Mine the block (simulate finding valid nonce)
  uint32_t hashOperations = MineBlock (blockData, m_difficulty);
  
  // Calculate hash
  block.blockHash = CalculateHash (blockData + std::to_string (hashOperations));
  
  // Calculate energy consumption: E_crypto = e_h * n_hash
  m_energyConsumption = m_energyPerHash * hashOperations;
  
  // Calculate latency
  double endTime = Simulator::Now ().GetSeconds ();
  m_latency = endTime - startTime + (hashOperations * 1e-6); // Add simulated mining time
  
  NS_LOG_INFO ("PoW block generated: " << block.blockId 
               << " with " << hashOperations << " hash operations"
               << ", energy: " << m_energyConsumption << " J"
               << ", latency: " << m_latency << " s");
  
  return block;
}

bool
PoWModule::ValidateBlockCandidate (const BlockCandidate& block)
{
  NS_LOG_FUNCTION (this);
  
  if (block.consensusType != "PoW")
    {
      NS_LOG_WARN ("Block is not PoW type");
      return false;
    }
  
  // Check if hash meets difficulty requirement
  bool valid = CheckDifficulty (block.blockHash, block.difficulty);
  
  // Validation consumes minimal energy (just verification)
  m_energyConsumption = m_energyPerHash * 10; // Approximate 10 hash operations for verification
  m_latency = 1e-5; // 10 microseconds for verification
  
  NS_LOG_INFO ("PoW block validation: " << (valid ? "VALID" : "INVALID"));
  
  return valid;
}

std::string
PoWModule::GetConsensusType () const
{
  return "PoW";
}

void
PoWModule::SetEnergyPerHash (double energyPerHash)
{
  m_energyPerHash = energyPerHash;
}

double
PoWModule::GetEnergyPerHash () const
{
  return m_energyPerHash;
}

uint32_t
PoWModule::MineBlock (const std::string& blockData, uint32_t difficulty)
{
  // Simulate mining: number of attempts follows exponential distribution
  // Average attempts = 2^difficulty
  double avgAttempts = std::pow (2.0, difficulty);
  
  // Use exponential distribution for realistic mining simulation
  double lambda = 1.0 / avgAttempts;
  double u = m_random->GetValue (0.0, 1.0);
  uint32_t attempts = static_cast<uint32_t> (-std::log (1.0 - u) / lambda);
  
  // Ensure at least some attempts
  attempts = std::max (attempts, static_cast<uint32_t> (1));
  
  return attempts;
}

std::string
PoWModule::CalculateHash (const std::string& data) const
{
  // Simple hash simulation using string manipulation
  // In real implementation, this would be SHA-256 or similar
  std::hash<std::string> hasher;
  size_t hashValue = hasher (data);
  
  std::ostringstream oss;
  oss << std::hex << std::setfill ('0') << std::setw (16) << hashValue;
  return oss.str ();
}

bool
PoWModule::CheckDifficulty (const std::string& hash, uint32_t difficulty) const
{
  // Check if hash has required number of leading zeros
  if (hash.length () < difficulty)
    {
      return false;
    }
  
  // Simplified check: count leading zeros in hex string
  uint32_t leadingZeros = 0;
  for (char c : hash)
    {
      if (c == '0')
        {
          leadingZeros++;
        }
      else
        {
          break;
        }
    }
  
  return leadingZeros >= difficulty;
}

} // namespace ns3

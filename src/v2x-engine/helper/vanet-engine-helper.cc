/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 */

#include "vanet-engine-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <cmath>
#include <algorithm>
#include <map>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VanetEngineHelper");

NS_OBJECT_ENSURE_REGISTERED (VanetEngineHelper);

TypeId
VanetEngineHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VanetEngineHelper")
    .SetParent<Object> ()
    .SetGroupName ("V2xEngine")
    .AddConstructor<VanetEngineHelper> ();
  return tid;
}

VanetEngineHelper::VanetEngineHelper ()
  : m_currentMode (LOW_COST_MODE),
    m_informationalEntropyThreshold (0.5),
    m_spatialEntropyThreshold (0.6),
    m_blockSize (10),
    m_beaconInterval (0.1), // 100ms
    m_qoiDelayThreshold (0.5), // 500ms
    m_logFileInitialized (false)
{
  NS_LOG_FUNCTION (this);
}

VanetEngineHelper::~VanetEngineHelper ()
{
  NS_LOG_FUNCTION (this);
  if (m_logFile.is_open ())
    {
      m_logFile.close ();
    }
}

void
VanetEngineHelper::Initialize ()
{
  NS_LOG_FUNCTION (this);
  
  // Create consensus modules
  m_powModule = CreateObject<PoWModule> ();
  m_posModule = CreateObject<PoSModule> ();
  m_fbaModule = CreateObject<FBAModule> ();
  
  // Start with Low-Cost mode (PoW)
  m_activeModule = m_powModule;
  m_currentMode = LOW_COST_MODE;
  
  // Initialize log file
  InitializeLogFile ();
  
  NS_LOG_INFO ("VanetEngineHelper initialized with Low-Cost mode (PoW)");
}

void
VanetEngineHelper::ControlLoop (double currentTime)
{
  NS_LOG_FUNCTION (this << currentTime);
  
  // Step 1: Calculate entropies
  double infoEntropy = CalculateInformationalEntropy ();
  double spatialEntropy = CalculateSpatialEntropy ();
  
  NS_LOG_INFO ("Time: " << currentTime 
               << ", S(t): " << infoEntropy 
               << ", H_spatial(t): " << spatialEntropy);
  
  // Step 2: Decide consensus mode based on thresholds
  ConsensusMode newMode = DecideConsensusMode (infoEntropy, spatialEntropy);
  
  // Step 3: Switch mode if necessary
  if (newMode != m_currentMode)
    {
      NS_LOG_INFO ("Switching consensus mode from " 
                   << (m_currentMode == LOW_COST_MODE ? "LOW_COST" : "HIGH_RIGOR")
                   << " to "
                   << (newMode == LOW_COST_MODE ? "LOW_COST" : "HIGH_RIGOR"));
      SwitchConsensusMode (newMode);
    }
  
  // Step 4: Process information cycle
  ProcessInformationCycle (currentTime);
}

double
VanetEngineHelper::CalculateInformationalEntropy ()
{
  NS_LOG_FUNCTION (this);
  
  if (m_nodeStates.empty ())
    {
      return 0.0;
    }
  
  // Calculate total pending transactions
  uint32_t totalTxs = 0;
  for (const auto& pair : m_nodeStates)
    {
      totalTxs += pair.second.pendingTxs.size ();
    }
  
  if (totalTxs == 0)
    {
      return 0.0;
    }
  
  // Calculate entropy: S = -Σ p_i * log2(p_i)
  // where p_i is the fraction of pending transactions at node i
  double entropy = 0.0;
  
  for (const auto& pair : m_nodeStates)
    {
      if (pair.second.pendingTxs.size () > 0)
        {
          double p_i = static_cast<double> (pair.second.pendingTxs.size ()) / totalTxs;
          entropy -= p_i * std::log2 (p_i);
        }
    }
  
  return entropy;
}

double
VanetEngineHelper::CalculateSpatialEntropy (double gridSize)
{
  NS_LOG_FUNCTION (this << gridSize);
  
  if (m_nodeStates.empty ())
    {
      return 0.0;
    }
  
  // Create spatial grid and count vehicles per cell
  std::map<std::pair<int, int>, uint32_t> grid;
  
  for (const auto& pair : m_nodeStates)
    {
      Vector pos = pair.second.position;
      int gridX = static_cast<int> (pos.x / gridSize);
      int gridY = static_cast<int> (pos.y / gridSize);
      
      std::pair<int, int> cell (gridX, gridY);
      grid[cell]++;
    }
  
  // Calculate spatial entropy: H_spatial = -Σ (n_i/N) * log2(n_i/N)
  // where n_i is number of vehicles in cell i, N is total vehicles
  double entropy = 0.0;
  uint32_t totalNodes = m_nodeStates.size ();
  
  for (const auto& pair : grid)
    {
      double p_i = static_cast<double> (pair.second) / totalNodes;
      entropy -= p_i * std::log2 (p_i);
    }
  
  return entropy;
}

ConsensusMode
VanetEngineHelper::DecideConsensusMode (double informationalEntropy, 
                                         double spatialEntropy)
{
  NS_LOG_FUNCTION (this << informationalEntropy << spatialEntropy);
  
  // Logic from paper: If S > S_th OR H_spatial > H_th, use High-Rigor mode
  // Otherwise, use Low-Cost mode
  if (informationalEntropy > m_informationalEntropyThreshold ||
      spatialEntropy > m_spatialEntropyThreshold)
    {
      return HIGH_RIGOR_MODE;
    }
  else
    {
      return LOW_COST_MODE;
    }
}

void
VanetEngineHelper::SwitchConsensusMode (ConsensusMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  
  m_currentMode = mode;
  
  if (mode == LOW_COST_MODE)
    {
      // Use PoW for low-cost operations
      m_activeModule = m_powModule;
      NS_LOG_INFO ("Switched to LOW_COST mode (PoW)");
    }
  else // HIGH_RIGOR_MODE
    {
      // Alternate between PoS and FBA for high-rigor operations
      // In practice, you might choose based on additional criteria
      // Here we use PoS as default for high-rigor
      m_activeModule = m_posModule;
      NS_LOG_INFO ("Switched to HIGH_RIGOR mode (PoS)");
    }
}

void
VanetEngineHelper::UpdateNodeState (uint32_t nodeId, 
                                     const Vector& position,
                                     const std::vector<Transaction>& transactions)
{
  NS_LOG_FUNCTION (this << nodeId);
  
  NodeState state;
  state.nodeId = nodeId;
  state.position = position;
  state.pendingTxs = transactions;
  state.lastUpdateTime = Simulator::Now ().GetSeconds ();
  
  m_nodeStates[nodeId] = state;
}

std::vector<Transaction>
VanetEngineHelper::ApplyQoIFiltering (const std::vector<Transaction>& transactions,
                                       double delayThreshold)
{
  NS_LOG_FUNCTION (this << delayThreshold);
  
  std::vector<Transaction> filtered;
  double currentTime = Simulator::Now ().GetSeconds ();
  
  for (const auto& tx : transactions)
    {
      double delay = currentTime - tx.timestamp;
      
      // Filter out transactions with excessive delay
      if (delay <= delayThreshold)
        {
          filtered.push_back (tx);
        }
      else
        {
          NS_LOG_DEBUG ("Filtering transaction " << tx.txId << " due to high delay: " << delay);
        }
    }
  
  NS_LOG_INFO ("QoI Filtering: " << transactions.size () << " -> " << filtered.size () << " transactions");
  
  return filtered;
}

void
VanetEngineHelper::ProcessInformationCycle (double currentTime)
{
  NS_LOG_FUNCTION (this << currentTime);
  
  // Step 1: Injection - Collect transactions from all nodes
  std::vector<Transaction> allTransactions;
  for (const auto& pair : m_nodeStates)
    {
      for (const auto& tx : pair.second.pendingTxs)
        {
          allTransactions.push_back (tx);
        }
    }
  
  if (allTransactions.empty ())
    {
      NS_LOG_DEBUG ("No transactions to process");
      return;
    }
  
  // Step 2: Validation - Apply QoI filtering
  std::vector<Transaction> filteredTxs = ApplyQoIFiltering (allTransactions, m_qoiDelayThreshold);
  
  if (filteredTxs.empty ())
    {
      NS_LOG_DEBUG ("All transactions filtered out");
      return;
    }
  
  // Take only blockSize transactions for this block
  if (filteredTxs.size () > m_blockSize)
    {
      filteredTxs.resize (m_blockSize);
    }
  
  // Select a proposer node (first node with transactions, or random)
  uint32_t proposerNode = 0;
  if (!m_nodeStates.empty ())
    {
      proposerNode = m_nodeStates.begin ()->first;
    }
  
  // Generate block
  std::string previousHash = "0000000000000000"; // Genesis or previous block hash
  if (!m_blockchain.empty ())
    {
      previousHash = m_blockchain.back ().blockHash;
    }
  
  BlockCandidate block = GenerateBlock (filteredTxs, proposerNode);
  
  // Step 3: Commit - Validate and add to ledger
  if (m_activeModule->ValidateBlockCandidate (block))
    {
      CommitBlock (block);
      
      // Remove processed transactions from node states
      for (auto& pair : m_nodeStates)
        {
          pair.second.pendingTxs.clear ();
        }
    }
}

BlockCandidate
VanetEngineHelper::GenerateBlock (const std::vector<Transaction>& transactions,
                                   uint32_t proposerNodeId)
{
  NS_LOG_FUNCTION (this << proposerNodeId);
  
  std::string previousHash = "0000000000000000";
  if (!m_blockchain.empty ())
    {
      previousHash = m_blockchain.back ().blockHash;
    }
  
  return m_activeModule->GenerateBlockCandidate (transactions, proposerNodeId, previousHash);
}

void
VanetEngineHelper::CommitBlock (const BlockCandidate& block)
{
  NS_LOG_FUNCTION (this << block.blockId);
  
  m_blockchain.push_back (block);
  
  NS_LOG_INFO ("Block " << block.blockId << " committed to ledger. Total blocks: " << m_blockchain.size ());
}

void
VanetEngineHelper::LogMetrics (double time, uint32_t nodeId, double infoEntropy,
                                double spatialEntropy, ConsensusMode mode,
                                double radioEnergy, double cryptoEnergy,
                                double latency, bool forkEvent)
{
  NS_LOG_FUNCTION (this << time << nodeId);
  
  if (!m_logFile.is_open ())
    {
      return;
    }
  
  std::string modeStr = (mode == LOW_COST_MODE) ? "LOW_COST" : "HIGH_RIGOR";
  
  m_logFile << time << ","
            << nodeId << ","
            << infoEntropy << ","
            << spatialEntropy << ","
            << modeStr << ","
            << radioEnergy << ","
            << cryptoEnergy << ","
            << latency << ","
            << (forkEvent ? "1" : "0") << std::endl;
}

void
VanetEngineHelper::SetLogFile (const std::string& filepath)
{
  m_logFilePath = filepath;
}

ConsensusMode
VanetEngineHelper::GetCurrentMode () const
{
  return m_currentMode;
}

Ptr<ConsensusModule>
VanetEngineHelper::GetActiveConsensusModule () const
{
  return m_activeModule;
}

void
VanetEngineHelper::SetInformationalEntropyThreshold (double threshold)
{
  m_informationalEntropyThreshold = threshold;
}

void
VanetEngineHelper::SetSpatialEntropyThreshold (double threshold)
{
  m_spatialEntropyThreshold = threshold;
}

void
VanetEngineHelper::SetBlockSize (uint32_t size)
{
  m_blockSize = size;
}

void
VanetEngineHelper::SetBeaconInterval (double interval)
{
  m_beaconInterval = interval;
}

void
VanetEngineHelper::InitializeLogFile ()
{
  if (m_logFilePath.empty ())
    {
      m_logFilePath = "v2x-metrics.csv";
    }
  
  m_logFile.open (m_logFilePath);
  
  if (m_logFile.is_open ())
    {
      // Write CSV header
      m_logFile << "Time,NodeID,InformationalEntropy,SpatialEntropy,Mode,"
                << "RadioEnergy,CryptoEnergy,Latency,ForkEvent" << std::endl;
      m_logFileInitialized = true;
      NS_LOG_INFO ("Log file initialized: " << m_logFilePath);
    }
  else
    {
      NS_LOG_ERROR ("Failed to open log file: " << m_logFilePath);
    }
}

} // namespace ns3

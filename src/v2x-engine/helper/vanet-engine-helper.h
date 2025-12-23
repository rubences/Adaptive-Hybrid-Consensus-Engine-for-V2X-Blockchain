/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#ifndef VANET_ENGINE_HELPER_H
#define VANET_ENGINE_HELPER_H

#include "consensus-module.h"
#include "pow-module.h"
#include "pos-module.h"
#include "fba-module.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/vector.h"
#include <map>
#include <vector>
#include <fstream>

namespace ns3 {

/**
 * \brief Enumeration for consensus modes
 */
enum ConsensusMode {
  LOW_COST_MODE,    ///< Low-cost mode (PoW) - used when entropy is low
  HIGH_RIGOR_MODE   ///< High-rigor mode (PoS/FBA) - used when entropy is high
};

/**
 * \brief Structure for storing node state information
 */
struct NodeState {
  uint32_t nodeId;                      ///< Node identifier
  Vector position;                      ///< Current position
  std::vector<Transaction> pendingTxs;  ///< Pending transactions
  double lastUpdateTime;                ///< Last state update time
};

/**
 * \brief Main control loop for the Adaptive Hybrid Consensus Engine
 * 
 * This class implements the core control logic that:
 * 1. Measures Informational Entropy (S) based on transaction distribution
 * 2. Measures Spatial Entropy (H_spatial) based on vehicle dispersion
 * 3. Switches between Low-Cost (PoW) and High-Rigor (PoS/FBA) modes
 *    based on entropy thresholds (S_th = 0.5, H_th = 0.6)
 * 4. Manages the "Ideal Information Cycle": Injection -> Validation -> Commit
 * 5. Implements QoI (Quality of Information) filtering
 */
class VanetEngineHelper : public Object
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
  VanetEngineHelper ();
  
  /**
   * \brief Destructor
   */
  virtual ~VanetEngineHelper ();

  /**
   * \brief Initialize the VANET engine with consensus modules
   */
  void Initialize ();

  /**
   * \brief Main control loop - called periodically
   * \param currentTime Current simulation time
   */
  void ControlLoop (double currentTime);

  /**
   * \brief Calculate Informational Entropy (S) based on transaction distribution
   * 
   * Formula: S = -Σ p_i * log2(p_i)
   * where p_i is the fraction of pending transactions at node i
   * 
   * \return Informational entropy value
   */
  double CalculateInformationalEntropy ();

  /**
   * \brief Calculate Spatial Entropy (H_spatial) based on vehicle dispersion
   * 
   * Formula: H_spatial = -Σ (n_i/N) * log2(n_i/N)
   * where n_i is the number of vehicles in grid cell i, N is total vehicles
   * 
   * \param gridSize Size of grid cells for spatial binning (meters)
   * \return Spatial entropy value
   */
  double CalculateSpatialEntropy (double gridSize = 100.0);

  /**
   * \brief Decide consensus mode based on entropy thresholds
   * 
   * Logic: If S > S_th (0.5) OR H_spatial > H_th (0.6), use High-Rigor mode
   *        Otherwise, use Low-Cost mode
   * 
   * \param informationalEntropy Current informational entropy
   * \param spatialEntropy Current spatial entropy
   * \return Selected consensus mode
   */
  ConsensusMode DecideConsensusMode (double informationalEntropy, 
                                      double spatialEntropy);

  /**
   * \brief Switch to the appropriate consensus module
   * \param mode Target consensus mode
   */
  void SwitchConsensusMode (ConsensusMode mode);

  /**
   * \brief Update node state (position and pending transactions)
   * \param nodeId Node identifier
   * \param position Current position
   * \param transactions Pending transactions at this node
   */
  void UpdateNodeState (uint32_t nodeId, 
                        const Vector& position,
                        const std::vector<Transaction>& transactions);

  /**
   * \brief Apply QoI (Quality of Information) filtering
   * 
   * Discards transactions with high delay (Δt > threshold)
   * 
   * \param transactions Input transactions
   * \param delayThreshold Maximum acceptable delay (seconds)
   * \return Filtered transactions
   */
  std::vector<Transaction> ApplyQoIFiltering (
    const std::vector<Transaction>& transactions,
    double delayThreshold);

  /**
   * \brief Process the "Ideal Information Cycle"
   * 
   * Steps: 1) Injection - collect transactions
   *        2) Validation - apply QoI filtering and consensus
   *        3) Commit - add block to ledger
   * 
   * \param currentTime Current simulation time
   */
  void ProcessInformationCycle (double currentTime);

  /**
   * \brief Generate a new block using current consensus module
   * \param transactions Transactions to include
   * \param proposerNodeId Node proposing the block
   * \return Generated block candidate
   */
  BlockCandidate GenerateBlock (const std::vector<Transaction>& transactions,
                                 uint32_t proposerNodeId);

  /**
   * \brief Commit a validated block to the ledger
   * \param block Block to commit
   */
  void CommitBlock (const BlockCandidate& block);

  /**
   * \brief Log metrics to CSV file
   * \param time Current time
   * \param nodeId Node ID
   * \param infoEntropy Informational entropy
   * \param spatialEntropy Spatial entropy
   * \param mode Current consensus mode
   * \param radioEnergy Radio energy consumption
   * \param cryptoEnergy Crypto energy consumption
   * \param latency Consensus latency
   * \param forkEvent Whether a fork occurred
   */
  void LogMetrics (double time, uint32_t nodeId, double infoEntropy,
                   double spatialEntropy, ConsensusMode mode,
                   double radioEnergy, double cryptoEnergy,
                   double latency, bool forkEvent);

  /**
   * \brief Set CSV log file path
   * \param filepath Path to CSV log file
   */
  void SetLogFile (const std::string& filepath);

  /**
   * \brief Get current consensus mode
   * \return Current mode
   */
  ConsensusMode GetCurrentMode () const;

  /**
   * \brief Get current active consensus module
   * \return Pointer to active consensus module
   */
  Ptr<ConsensusModule> GetActiveConsensusModule () const;

  /**
   * \brief Set informational entropy threshold
   * \param threshold Threshold value (default 0.5)
   */
  void SetInformationalEntropyThreshold (double threshold);

  /**
   * \brief Set spatial entropy threshold
   * \param threshold Threshold value (default 0.6)
   */
  void SetSpatialEntropyThreshold (double threshold);

  /**
   * \brief Set block size (number of transactions per block)
   * \param size Block size (default 10)
   */
  void SetBlockSize (uint32_t size);

  /**
   * \brief Set beacon interval
   * \param interval Interval in seconds (default 0.1 for 100ms)
   */
  void SetBeaconInterval (double interval);

private:
  /**
   * \brief Initialize CSV log file with headers
   */
  void InitializeLogFile ();

  Ptr<PoWModule> m_powModule;           ///< PoW consensus module
  Ptr<PoSModule> m_posModule;           ///< PoS consensus module
  Ptr<FBAModule> m_fbaModule;           ///< FBA consensus module
  Ptr<ConsensusModule> m_activeModule;  ///< Currently active consensus module

  ConsensusMode m_currentMode;          ///< Current consensus mode
  double m_informationalEntropyThreshold; ///< Threshold for S (default 0.5)
  double m_spatialEntropyThreshold;     ///< Threshold for H_spatial (default 0.6)

  std::map<uint32_t, NodeState> m_nodeStates; ///< State of all nodes
  std::vector<BlockCandidate> m_blockchain;   ///< Local blockchain ledger

  uint32_t m_blockSize;                 ///< Number of transactions per block
  double m_beaconInterval;              ///< Beacon interval (seconds)
  double m_qoiDelayThreshold;           ///< QoI delay threshold (seconds)

  std::string m_logFilePath;            ///< Path to CSV log file
  std::ofstream m_logFile;              ///< Output stream for logging
  bool m_logFileInitialized;            ///< Whether log file is initialized
};

} // namespace ns3

#endif /* VANET_ENGINE_HELPER_H */

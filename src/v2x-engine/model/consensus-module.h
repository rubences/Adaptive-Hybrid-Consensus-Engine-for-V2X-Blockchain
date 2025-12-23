/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Adaptive Hybrid Consensus Engine for V2X Blockchain
 * Reference: "Adaptive Hybrid Consensus Engine for V2X Blockchain: 
 *             Real-Time Entropy-Driven Control for High Energy Efficiency 
 *             and Sub-100 ms Latency"
 */

#ifndef CONSENSUS_MODULE_H
#define CONSENSUS_MODULE_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include <vector>
#include <string>

namespace ns3 {

/**
 * \brief Structure representing a blockchain transaction
 */
struct Transaction {
    uint32_t txId;           ///< Transaction identifier
    uint32_t sourceNodeId;   ///< Source node ID
    double timestamp;        ///< Timestamp of transaction creation
    std::string data;        ///< Transaction data (CAM/DENM)
    double delay;            ///< Network delay (for QoI filtering)
};

/**
 * \brief Structure representing a blockchain block candidate
 */
struct BlockCandidate {
    uint32_t blockId;                    ///< Block identifier
    uint32_t proposerNodeId;             ///< Node that proposed the block
    std::vector<Transaction> transactions; ///< Transactions in the block
    double timestamp;                    ///< Block creation timestamp
    std::string consensusType;           ///< Consensus mechanism used
    uint32_t difficulty;                 ///< Difficulty level (for PoW)
    std::string previousBlockHash;       ///< Hash of previous block
    std::string blockHash;               ///< Hash of current block
};

/**
 * \brief Abstract base class for consensus mechanisms
 * 
 * This class defines the interface that all consensus implementations must follow.
 * Implementations include PoW (Proof-of-Work), PoS (Proof-of-Stake), 
 * and FBA (Federated Byzantine Agreement).
 */
class ConsensusModule : public Object
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
  ConsensusModule ();
  
  /**
   * \brief Destructor
   */
  virtual ~ConsensusModule ();

  /**
   * \brief Generate a block candidate from pending transactions
   * \param transactions Vector of pending transactions
   * \param nodeId ID of the node generating the block
   * \param previousHash Hash of the previous block
   * \return BlockCandidate structure with proposed block
   */
  virtual BlockCandidate GenerateBlockCandidate (
    const std::vector<Transaction>& transactions,
    uint32_t nodeId,
    const std::string& previousHash) = 0;

  /**
   * \brief Validate a block candidate
   * \param block The block to validate
   * \return true if the block is valid, false otherwise
   */
  virtual bool ValidateBlockCandidate (const BlockCandidate& block) = 0;

  /**
   * \brief Get the energy consumed for the last consensus operation (Joules)
   * \return Energy consumption in Joules
   */
  virtual double GetEnergyConsumption () const;

  /**
   * \brief Get the latency of the last consensus operation (seconds)
   * \return Latency in seconds
   */
  virtual double GetLatency () const;

  /**
   * \brief Get the name of the consensus mechanism
   * \return String representing the consensus type
   */
  virtual std::string GetConsensusType () const = 0;

  /**
   * \brief Set the difficulty parameter (for PoW)
   * \param difficulty Difficulty level
   */
  virtual void SetDifficulty (uint32_t difficulty);

  /**
   * \brief Reset consensus module state
   */
  virtual void Reset ();

protected:
  double m_energyConsumption;  ///< Energy consumed in last operation (J)
  double m_latency;            ///< Latency of last operation (s)
  uint32_t m_difficulty;       ///< Difficulty level
};

} // namespace ns3

#endif /* CONSENSUS_MODULE_H */

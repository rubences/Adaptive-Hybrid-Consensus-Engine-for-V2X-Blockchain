/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#ifndef POW_MODULE_H
#define POW_MODULE_H

#include "consensus-module.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \brief Proof-of-Work (PoW) consensus implementation
 * 
 * This class implements the PoW consensus mechanism with hash-based mining.
 * Energy consumption is calculated based on the number of hash operations
 * required to find a valid nonce: E_crypto = e_h * n_hash
 * where e_h is the energy per hash operation (default: 5e-6 J/hash)
 */
class PoWModule : public ConsensusModule
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
  PoWModule ();
  
  /**
   * \brief Destructor
   */
  virtual ~PoWModule ();

  /**
   * \brief Generate a block candidate using PoW mining
   * \param transactions Vector of pending transactions
   * \param nodeId ID of the node generating the block
   * \param previousHash Hash of the previous block
   * \return BlockCandidate structure with mined block
   */
  virtual BlockCandidate GenerateBlockCandidate (
    const std::vector<Transaction>& transactions,
    uint32_t nodeId,
    const std::string& previousHash) override;

  /**
   * \brief Validate a PoW block candidate
   * \param block The block to validate
   * \return true if the block is valid (correct nonce), false otherwise
   */
  virtual bool ValidateBlockCandidate (const BlockCandidate& block) override;

  /**
   * \brief Get the name of the consensus mechanism
   * \return "PoW"
   */
  virtual std::string GetConsensusType () const override;

  /**
   * \brief Set the energy cost per hash operation
   * \param energyPerHash Energy in Joules per hash
   */
  void SetEnergyPerHash (double energyPerHash);

  /**
   * \brief Get the energy cost per hash operation
   * \return Energy in Joules per hash
   */
  double GetEnergyPerHash () const;

private:
  /**
   * \brief Simulate mining process (finding valid nonce)
   * \param blockData Data to hash
   * \param difficulty Target difficulty
   * \return Number of hash operations performed
   */
  uint32_t MineBlock (const std::string& blockData, uint32_t difficulty);

  /**
   * \brief Calculate hash of block data
   * \param data Input data
   * \return Hash string
   */
  std::string CalculateHash (const std::string& data) const;

  /**
   * \brief Check if hash meets difficulty requirement
   * \param hash Hash to check
   * \param difficulty Target difficulty (number of leading zeros)
   * \return true if hash meets difficulty
   */
  bool CheckDifficulty (const std::string& hash, uint32_t difficulty) const;

  double m_energyPerHash;               ///< Energy per hash operation (J)
  Ptr<UniformRandomVariable> m_random;  ///< Random number generator
};

} // namespace ns3

#endif /* POW_MODULE_H */

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#ifndef POS_MODULE_H
#define POS_MODULE_H

#include "consensus-module.h"
#include "ns3/random-variable-stream.h"
#include <map>

namespace ns3 {

/**
 * \brief Proof-of-Stake (PoS) consensus implementation
 * 
 * This class implements the PoS consensus mechanism where validators
 * are selected based on their stake. Energy consumption is based on
 * signature operations: E_crypto = e_sig * n_sig
 * where e_sig is the energy per signature (default: 1e-6 J/sig)
 */
class PoSModule : public ConsensusModule
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
  PoSModule ();
  
  /**
   * \brief Destructor
   */
  virtual ~PoSModule ();

  /**
   * \brief Generate a block candidate using PoS
   * \param transactions Vector of pending transactions
   * \param nodeId ID of the node generating the block
   * \param previousHash Hash of the previous block
   * \return BlockCandidate structure with proposed block
   */
  virtual BlockCandidate GenerateBlockCandidate (
    const std::vector<Transaction>& transactions,
    uint32_t nodeId,
    const std::string& previousHash) override;

  /**
   * \brief Validate a PoS block candidate
   * \param block The block to validate
   * \return true if the block is valid (valid signature), false otherwise
   */
  virtual bool ValidateBlockCandidate (const BlockCandidate& block) override;

  /**
   * \brief Get the name of the consensus mechanism
   * \return "PoS"
   */
  virtual std::string GetConsensusType () const override;

  /**
   * \brief Set the energy cost per signature operation
   * \param energyPerSig Energy in Joules per signature
   */
  void SetEnergyPerSignature (double energyPerSig);

  /**
   * \brief Get the energy cost per signature operation
   * \return Energy in Joules per signature
   */
  double GetEnergyPerSignature () const;

  /**
   * \brief Set the stake for a node
   * \param nodeId Node identifier
   * \param stake Stake amount
   */
  void SetNodeStake (uint32_t nodeId, double stake);

  /**
   * \brief Get the stake for a node
   * \param nodeId Node identifier
   * \return Stake amount
   */
  double GetNodeStake (uint32_t nodeId) const;

private:
  /**
   * \brief Select validator based on stake-weighted random selection
   * \return Selected validator node ID
   */
  uint32_t SelectValidator ();

  /**
   * \brief Simulate digital signature creation
   * \param data Data to sign
   * \return Signature string
   */
  std::string CreateSignature (const std::string& data);

  /**
   * \brief Verify digital signature
   * \param data Original data
   * \param signature Signature to verify
   * \return true if signature is valid
   */
  bool VerifySignature (const std::string& data, const std::string& signature);

  double m_energyPerSignature;          ///< Energy per signature operation (J)
  std::map<uint32_t, double> m_stakes;  ///< Node stakes
  Ptr<UniformRandomVariable> m_random;  ///< Random number generator
};

} // namespace ns3

#endif /* POS_MODULE_H */

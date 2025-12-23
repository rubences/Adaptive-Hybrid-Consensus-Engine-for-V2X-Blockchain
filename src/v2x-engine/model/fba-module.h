/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#ifndef FBA_MODULE_H
#define FBA_MODULE_H

#include "consensus-module.h"
#include "ns3/random-variable-stream.h"
#include <set>

namespace ns3 {

/**
 * \brief Federated Byzantine Agreement (FBA) consensus implementation
 * 
 * This class implements the FBA consensus mechanism using quorum-based
 * agreement. Energy consumption is based on signature operations for
 * quorum verification: E_crypto = e_sig * n_quorum_members
 * where e_sig is the energy per signature (default: 1e-6 J/sig)
 */
class FBAModule : public ConsensusModule
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
  FBAModule ();
  
  /**
   * \brief Destructor
   */
  virtual ~FBAModule ();

  /**
   * \brief Generate a block candidate using FBA
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
   * \brief Validate a FBA block candidate
   * \param block The block to validate
   * \return true if the block has quorum agreement, false otherwise
   */
  virtual bool ValidateBlockCandidate (const BlockCandidate& block) override;

  /**
   * \brief Get the name of the consensus mechanism
   * \return "FBA"
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
   * \brief Set the quorum threshold (fraction of nodes required)
   * \param threshold Quorum threshold (0.0 to 1.0), default 0.67
   */
  void SetQuorumThreshold (double threshold);

  /**
   * \brief Get the quorum threshold
   * \return Quorum threshold
   */
  double GetQuorumThreshold () const;

  /**
   * \brief Add node to the quorum set
   * \param nodeId Node identifier
   */
  void AddQuorumMember (uint32_t nodeId);

  /**
   * \brief Get the number of quorum members
   * \return Number of nodes in quorum
   */
  uint32_t GetQuorumSize () const;

private:
  /**
   * \brief Simulate quorum voting process
   * \param blockData Block data to vote on
   * \return true if quorum threshold is reached
   */
  bool SimulateQuorumVoting (const std::string& blockData);

  /**
   * \brief Create vote signature for a node
   * \param nodeId Node identifier
   * \param blockData Block data being voted on
   * \return Signature string
   */
  std::string CreateVoteSignature (uint32_t nodeId, const std::string& blockData);

  double m_energyPerSignature;          ///< Energy per signature operation (J)
  double m_quorumThreshold;             ///< Quorum threshold (default 0.67)
  std::set<uint32_t> m_quorumMembers;   ///< Set of quorum member node IDs
  Ptr<UniformRandomVariable> m_random;  ///< Random number generator
};

} // namespace ns3

#endif /* FBA_MODULE_H */

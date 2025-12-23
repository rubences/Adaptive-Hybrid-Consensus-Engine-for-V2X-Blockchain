/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 */

#include "consensus-module.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConsensusModule");

NS_OBJECT_ENSURE_REGISTERED (ConsensusModule);

TypeId
ConsensusModule::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ConsensusModule")
    .SetParent<Object> ()
    .SetGroupName ("V2xEngine");
  return tid;
}

ConsensusModule::ConsensusModule ()
  : m_energyConsumption (0.0),
    m_latency (0.0),
    m_difficulty (4)
{
  NS_LOG_FUNCTION (this);
}

ConsensusModule::~ConsensusModule ()
{
  NS_LOG_FUNCTION (this);
}

double
ConsensusModule::GetEnergyConsumption () const
{
  return m_energyConsumption;
}

double
ConsensusModule::GetLatency () const
{
  return m_latency;
}

void
ConsensusModule::SetDifficulty (uint32_t difficulty)
{
  m_difficulty = difficulty;
}

void
ConsensusModule::Reset ()
{
  m_energyConsumption = 0.0;
  m_latency = 0.0;
}

} // namespace ns3

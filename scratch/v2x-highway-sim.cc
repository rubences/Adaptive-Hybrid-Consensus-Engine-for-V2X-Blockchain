/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * Highway Scenario for Adaptive Hybrid Consensus Engine
 * 
 * This simulation creates a highway environment with:
 * - Multiple lanes
 * - Higher vehicle speeds (25-35 m/s / 90-126 km/h)
 * - Linear mobility pattern
 * - IEEE 802.11p (WAVE) communication
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/applications-module.h"
#include "ns3/v2x-engine-module.h"
#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("V2xHighwaySimulation");

/**
 * \brief Periodic callback to run the VANET engine control loop
 */
void
RunControlLoop (Ptr<VanetEngineHelper> engine, double interval)
{
  double currentTime = Simulator::Now ().GetSeconds ();
  engine->ControlLoop (currentTime);
  
  // Schedule next control loop
  Simulator::Schedule (Seconds (interval), &RunControlLoop, engine, interval);
}

int
main (int argc, char *argv[])
{
  // Simulation parameters
  uint32_t nNodes = 80;              // Number of vehicle nodes
  double simTime = 100.0;            // Simulation time (seconds)
  double highwayLength = 5000.0;     // Highway length (meters)
  uint32_t nLanes = 3;               // Number of lanes
  double laneWidth = 4.0;            // Lane width (meters)
  double nodeSpeed = 30.0;           // Average vehicle speed (m/s) ~ 108 km/h
  double beaconInterval = 0.1;       // CAM beacon interval (100ms)
  double controlLoopInterval = 1.0;  // Control loop interval (1 second)
  std::string logFile = "v2x-highway-metrics.csv";
  
  // Parse command line arguments
  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of vehicle nodes", nNodes);
  cmd.AddValue ("simTime", "Simulation time in seconds", simTime);
  cmd.AddValue ("highwayLength", "Highway length in meters", highwayLength);
  cmd.AddValue ("nLanes", "Number of highway lanes", nLanes);
  cmd.AddValue ("nodeSpeed", "Average vehicle speed in m/s", nodeSpeed);
  cmd.AddValue ("beaconInterval", "CAM beacon interval in seconds", beaconInterval);
  cmd.AddValue ("logFile", "Output CSV log file", logFile);
  cmd.Parse (argc, argv);
  
  NS_LOG_INFO ("======================================");
  NS_LOG_INFO ("V2X Highway Simulation");
  NS_LOG_INFO ("Number of nodes: " << nNodes);
  NS_LOG_INFO ("Simulation time: " << simTime << " s");
  NS_LOG_INFO ("Highway length: " << highwayLength << " m");
  NS_LOG_INFO ("Number of lanes: " << nLanes);
  NS_LOG_INFO ("======================================");
  
  // Create nodes
  NodeContainer nodes;
  nodes.Create (nNodes);
  
  // Setup mobility model (Highway with multiple lanes)
  MobilityHelper mobility;
  
  // Position vehicles along the highway in different lanes
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint32_t i = 0; i < nNodes; i++)
    {
      double x = (i * highwayLength / nNodes); // Distribute along highway
      uint32_t lane = i % nLanes;
      double y = lane * laneWidth;
      positionAlloc->Add (Vector (x, y, 0.0));
    }
  mobility.SetPositionAllocator (positionAlloc);
  
  // Constant velocity mobility along highway
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (nodes);
  
  // Set velocities for each node
  for (uint32_t i = 0; i < nNodes; i++)
    {
      Ptr<ConstantVelocityMobilityModel> cvmm = nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ();
      // Add some variance to speed
      double speed = nodeSpeed + (i % 10 - 5) * 2.0; // ±10 m/s variance
      cvmm->SetVelocity (Vector (speed, 0.0, 0.0)); // Moving along X axis
    }
  
  // Setup WiFi PHY and MAC for IEEE 802.11p (WAVE)
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211p);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue ("OfdmRate6MbpsBW10MHz"),
                                 "ControlMode", StringValue ("OfdmRate6MbpsBW10MHz"));
  
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);
  
  // Install Internet stack
  InternetStackHelper internet;
  internet.Install (nodes);
  
  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);
  
  // Setup energy model for radio
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (10000.0)); // 10kJ initial energy
  EnergySourceContainer energySources = basicSourceHelper.Install (nodes);
  
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174)); // 17.4 mA
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0197)); // 19.7 mA
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, energySources);
  
  // Create and initialize VANET engine
  Ptr<VanetEngineHelper> vanetEngine = CreateObject<VanetEngineHelper> ();
  vanetEngine->SetLogFile (logFile);
  vanetEngine->SetBlockSize (10);
  vanetEngine->SetBeaconInterval (beaconInterval);
  vanetEngine->SetInformationalEntropyThreshold (0.5);
  vanetEngine->SetSpatialEntropyThreshold (0.6);
  vanetEngine->Initialize ();
  
  // Install NodeApp on all nodes
  for (uint32_t i = 0; i < nNodes; i++)
    {
      Ptr<NodeApp> app = CreateObject<NodeApp> ();
      app->SetVanetEngine (vanetEngine);
      app->SetPacketInterval (beaconInterval);
      app->SetPort (9999);
      nodes.Get (i)->AddApplication (app);
      app->SetStartTime (Seconds (1.0 + i * 0.01)); // Staggered start
      app->SetStopTime (Seconds (simTime));
    }
  
  // Schedule control loop
  Simulator::Schedule (Seconds (2.0), &RunControlLoop, vanetEngine, controlLoopInterval);
  
  // Enable packet capture (optional)
  // wifiPhy.EnablePcap ("v2x-highway", devices.Get (0));
  
  NS_LOG_INFO ("Starting simulation...");
  
  // Run simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  
  NS_LOG_INFO ("Simulation completed.");
  NS_LOG_INFO ("Results saved to: " << logFile);
  
  // Cleanup
  Simulator::Destroy ();
  
  return 0;
}

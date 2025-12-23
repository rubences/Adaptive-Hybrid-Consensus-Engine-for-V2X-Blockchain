/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2024 V2X Blockchain Research
 *
 * Urban Grid Scenario for Adaptive Hybrid Consensus Engine
 * 
 * This simulation creates an urban grid environment with:
 * - 50-100 vehicle nodes
 * - Random waypoint mobility or BonnMotion traces
 * - IEEE 802.11p (WAVE) communication
 * - Adaptive consensus switching based on entropy
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

NS_LOG_COMPONENT_DEFINE ("V2xUrbanSimulation");

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
  uint32_t nNodes = 50;              // Number of vehicle nodes
  double simTime = 100.0;            // Simulation time (seconds)
  double gridSize = 1000.0;          // Urban grid size (meters)
  double nodeSpeed = 15.0;           // Average vehicle speed (m/s) ~ 54 km/h
  double nodePause = 2.0;            // Pause time for random waypoint (seconds)
  double beaconInterval = 0.1;       // CAM beacon interval (100ms)
  double controlLoopInterval = 1.0;  // Control loop interval (1 second)
  std::string logFile = "v2x-urban-metrics.csv";
  
  // Parse command line arguments
  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of vehicle nodes", nNodes);
  cmd.AddValue ("simTime", "Simulation time in seconds", simTime);
  cmd.AddValue ("gridSize", "Size of urban grid in meters", gridSize);
  cmd.AddValue ("nodeSpeed", "Average vehicle speed in m/s", nodeSpeed);
  cmd.AddValue ("beaconInterval", "CAM beacon interval in seconds", beaconInterval);
  cmd.AddValue ("logFile", "Output CSV log file", logFile);
  cmd.Parse (argc, argv);
  
  NS_LOG_INFO ("======================================");
  NS_LOG_INFO ("V2X Urban Grid Simulation");
  NS_LOG_INFO ("Number of nodes: " << nNodes);
  NS_LOG_INFO ("Simulation time: " << simTime << " s");
  NS_LOG_INFO ("Grid size: " << gridSize << " m");
  NS_LOG_INFO ("======================================");
  
  // Create nodes
  NodeContainer nodes;
  nodes.Create (nNodes);
  
  // Setup mobility model (Random Waypoint in urban grid)
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + 
                                                     std::to_string (gridSize) + "]"),
                                  "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=" + 
                                                     std::to_string (gridSize) + "]"));
  
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                             "Speed", StringValue ("ns3::UniformRandomVariable[Min=" + 
                                                   std::to_string (nodeSpeed * 0.5) + "|Max=" +
                                                   std::to_string (nodeSpeed * 1.5) + "]"),
                             "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=" + 
                                                   std::to_string (nodePause) + "]"),
                             "PositionAllocator", PointerValue (CreateObject<RandomRectanglePositionAllocator> ()));
  mobility.Install (nodes);
  
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
  // wifiPhy.EnablePcap ("v2x-urban", devices.Get (0));
  
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

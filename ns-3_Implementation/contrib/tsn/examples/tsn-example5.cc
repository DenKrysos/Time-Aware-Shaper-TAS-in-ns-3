/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This simulation shows how the tas-queue-disc can be configured and used
 *
 * 2020-08 (Aug 2020)
 * Author(s):
 *  - Implementation:
 *       Luca Wendling <lwendlin@rhrk.uni-kl.de>
 *  - Design & Implementation:
 *       Dennis Krummacker <dennis.krummacker@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/tsn-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/config-store-module.h"

#include "stdio.h"
#include <array>
#include <string>
#include <chrono>

#define NUMBER_OF_NODES 4
#define DATA_PAYLOADE_SIZE 100
#define TRANSMISON_SPEED "50Mbps"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tsn-exemple-5");

Time callbackfunc();

int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item);

int
main (int argc, char *argv[])
{
  //LogComponentEnable ("TasQueueDisc", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("TransmissonGateQdisc", LOG_LEVEL_LOGIC);
  CommandLine cmd;
  cmd.Parse (argc, argv);
  NodeContainer nodes;
  nodes.Create (NUMBER_OF_NODES);

  Time::SetResolution (Time::NS);
  Time sendPeriod,scheduleDuration,simulationDuration;
  scheduleDuration = Seconds(1);
  simulationDuration = 6*10*scheduleDuration;
  sendPeriod = scheduleDuration/4;
  sendPeriod -= MicroSeconds(1);
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue (TRANSMISON_SPEED));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer devices = csma.Install(nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  CallbackValue timeSource = MakeCallback(&callbackfunc);

  TsnHelper tsnHelperServer,tsnHelperClient;
  NetDeviceListConfig schedulePlanServer,schedulePlanClient;

  schedulePlanClient.Add(scheduleDuration,{1,1,1,1,1,1,1,1});
  schedulePlanClient.Add(scheduleDuration,{0,0,0,0,0,0,0,0});

  schedulePlanServer.Add(scheduleDuration,{0,0,0,0,0,0,0,0});
  schedulePlanServer.Add(scheduleDuration,{1,1,1,1,1,1,1,1});

  tsnHelperClient.SetRootQueueDisc("ns3::TasQueueDisc", "NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanClient), "TimeSource", timeSource,"DataRate", StringValue (TRANSMISON_SPEED));
  tsnHelperClient.AddPacketFilter(0,"ns3::TsnIpv4PacketFilter","Classify",CallbackValue(MakeCallback(&ipv4PacketFilter)));

  tsnHelperServer.SetRootQueueDisc("ns3::TasQueueDisc", "NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanServer), "TimeSource", timeSource,"DataRate", StringValue (TRANSMISON_SPEED));

  QueueDiscContainer qdiscsClient1 = tsnHelperClient.Install (devices.Get(0));
  QueueDiscContainer qdiscsServer = tsnHelperServer.Install (devices.Get(1));

  Ipv4AddressHelper address;

  address.SetBase ("0.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  Ipv4Address multicastSource ("10.1.1.1");
  Ipv4Address multicastGroup ("225.1.2.4");

  UdpEchoClientHelper echoClient1 (interfaces.GetAddress (1), 9);

  echoClient1.SetAttribute ("MaxPackets", UintegerValue (simulationDuration.GetInteger()/sendPeriod.GetInteger()));
  echoClient1.SetAttribute ("Interval", TimeValue (sendPeriod));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (DATA_PAYLOADE_SIZE));

  ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));

  clientApps1.Start (Seconds(0));
  clientApps1.Stop (simulationDuration);


  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));

  serverApps.Start (Seconds (0));
  serverApps.Stop (simulationDuration);

  csma.EnablePcap ("tas-exemple-5", nodes.Get (1)->GetId(),0);
  csma.EnablePcap ("tas-exemple-5", nodes.Get (0)->GetId(),0);
  //csma.EnablePcapAll("tas-test4");

  std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
  Simulator::Run ();
  std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();

  Simulator::Destroy ();
  std::cout << "Tsn-exemple-5 Csma" << std::endl;
  std::cout << NUMBER_OF_NODES << " Nodes " << std::endl;
  std::cout << "Total simulated Time: "<< simulationDuration.GetSeconds() << "s" << std::endl;
  std::cout << "Expectated number of packages in pcap: " << 2*simulationDuration.GetInteger()/sendPeriod.GetInteger() << std::endl;
  std::cout << "Schedule duration: "<< scheduleDuration.GetSeconds() <<" Sec Execution Time " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << " ms" << std::endl;
  std::cout << "Simulation Rate: "<< simulationDuration.GetMilliSeconds() / std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count()<< std::endl;
  return 0;
}

int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item){
  return 4;
}

Time callbackfunc(){
  return Simulator::Now();
}

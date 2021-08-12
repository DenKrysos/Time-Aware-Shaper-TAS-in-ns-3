/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This simulation shows how the tas-queue-disc can be configured and used
 */
/*
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
#define DATA_PAYLOADE_SIZE 150
#define TRANSMISON_SPEED "1Gbps"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tsn-exemple-4");

Time callbackfunc();

int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item);

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  NodeContainer nodes;
  nodes.Create (NUMBER_OF_NODES);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue (TRANSMISON_SPEED));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (40)));

  NetDeviceContainer devices = csma.Install(nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Time::SetResolution (Time::NS);
  Time sendPeriod,scheduleDuration,simulationDuration;
  scheduleDuration = MilliSeconds(100);
  simulationDuration = Seconds(60);
  sendPeriod = MilliSeconds(20);
 // sendPeriod -= MicroSeconds(1);

  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("TasQueueDisc", LOG_LEVEL_LOGIC);
  CallbackValue timeSource = MakeCallback(&callbackfunc);

  TsnHelper tsnHelperServer,tsnHelperClient;
  NetDeviceListConfig schedulePlanServer,schedulePlanClient;

  schedulePlanClient.Add(scheduleDuration,{1,1,1,1,1,1,1,1});//1
  schedulePlanClient.Add(scheduleDuration,{0,0,0,0,0,0,0,0});

  schedulePlanServer.Add(scheduleDuration,{0,0,0,0,0,0,0,0});//1
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

  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient1.SetAttribute ("Interval", TimeValue (sendPeriod));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (DATA_PAYLOADE_SIZE));

  ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));

  clientApps1.Start (Seconds(0));
  clientApps1.Stop (simulationDuration);

  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));

  serverApps.Start (Seconds (0));
  serverApps.Stop (simulationDuration);

  csma.EnablePcap ("Tsn-exemple-4", nodes.Get (1)->GetId(),0);

  std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
  Simulator::Run ();
  std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();
  Simulator::Destroy ();
  std::cout << "Tsn-exemple-4 Csma" << std::endl;
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

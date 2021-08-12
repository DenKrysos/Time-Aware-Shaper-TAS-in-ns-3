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

#include "stdio.h"
#include <array>
#include <string>
#include <chrono>

#define NUMBER_OF_SCHEDULE_ENTRYS 10

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tsn-exemple-1");

Time callbackfunc();

int32_t ipv4PacketFilter(Ptr<QueueDiscItem> item);

int
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);


  //LogComponentEnable ("TasQueueDisc", LOG_LEVEL_LOGIC);
  //LogComponentEnable ("TransmissonGateQdisc", LOG_LEVEL_LOGIC);

  Time::SetResolution (Time::NS);
  Time sendPeriod,scheduleDuration,simulationDuration;
  scheduleDuration = Seconds(1);
  simulationDuration = 2*scheduleDuration*NUMBER_OF_SCHEDULE_ENTRYS;
  sendPeriod = MilliSeconds(250);

  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  CallbackValue timeSource = MakeCallback(&callbackfunc);
  NodeContainer nodes;

  nodes.Create (2);
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  TsnHelper tsnHelperServer,tsnHelperClient;
  NetDeviceListConfig schedulePlanServer,schedulePlanClient;

  for (int i = 0; i < NUMBER_OF_SCHEDULE_ENTRYS/2; i++)
  {
    schedulePlanClient.Add(scheduleDuration,{1,1,1,1,1,1,1,1});
    schedulePlanClient.Add(scheduleDuration,{0,0,0,0,0,0,0,0});

    schedulePlanServer.Add(scheduleDuration,{0,0,0,0,0,0,0,0});
    schedulePlanServer.Add(scheduleDuration,{1,1,1,1,1,1,1,1});
  }

  tsnHelperClient.SetRootQueueDisc("ns3::TasQueueDisc", "NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanClient), "TimeSource", timeSource,"DataRate", StringValue ("5Mbps"));
  tsnHelperClient.AddPacketFilter(0,"ns3::TsnIpv4PacketFilter","Classify",CallbackValue(MakeCallback(&ipv4PacketFilter)));

  tsnHelperServer.SetRootQueueDisc("ns3::TasQueueDisc", "NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanServer), "TimeSource", timeSource,"DataRate", StringValue ("5Mbps"));

  QueueDiscContainer qdiscsClient1 = tsnHelperClient.Install (devices.Get(0));
  QueueDiscContainer qdiscsServer = tsnHelperServer.Install (devices.Get(1));

  Ipv4AddressHelper address;

  address.SetBase ("0.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  UdpEchoClientHelper echoClient1 (interfaces.GetAddress (1), 9);

  echoClient1.SetAttribute ("MaxPackets", UintegerValue (simulationDuration.GetInteger()/sendPeriod.GetInteger()));
  echoClient1.SetAttribute ("Interval", TimeValue (sendPeriod));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (64));

  ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));

  clientApps1.Start (Seconds(0));
  clientApps1.Stop (simulationDuration);

  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));

  serverApps.Start (Seconds (0));
  serverApps.Stop (simulationDuration);

  pointToPoint.EnablePcapAll("Tsn-exemple-1");

  std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
  Simulator::Run ();
  std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();
  
  Simulator::Destroy ();
  
  std::cout << "Tsn-exemple-1 pointToPoint" << std::endl;
  std::cout << 2 << " Nodes " << std::endl;
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

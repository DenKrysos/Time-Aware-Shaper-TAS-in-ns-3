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
#include "ns3/ptr.h"
#include "ns3/tsn-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/config-store-module.h"

#include "stdio.h"
#include <array>
#include <string>
#include <chrono>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tas-test");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  std::string path = "";

  cmd.AddValue("configPath", "Path to configuration of this test", path);

  cmd.Parse (argc, argv);

//  LogComponentEnable ("TasQueueDisc", LOG_LEVEL_LOGIC);
//  LogComponentEnable("CsmaNetDevice", LOG_LEVEL_LOGIC);
//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  Time::SetResolution (Time::NS);
  Time sendPeriod,scheduleDuration,simulationDuration;
  scheduleDuration = Seconds(1);
  simulationDuration = 6*10*scheduleDuration;
  sendPeriod = scheduleDuration/4;
  sendPeriod -= MicroSeconds(1);

  Config::SetDefault("ns3::CsmaChannel::DataRate",StringValue ("50Mbps"));
  Config::SetDefault("ns3::CsmaChannel::Delay",TimeValue (MilliSeconds (2)));

  Config::SetDefault ("ns3::TasQueueDisc::NetDeviceListConfig", NetDeviceListConfigValue(NetDeviceListConfig ()));
  Config::SetDefault ("ns3::TasQueueDisc::DataRate",StringValue ("50Mbps"));
  Config::SetDefault ("ns3::TasQueueDisc::Mtu", IntegerValue (1500));

  Config::SetDefault("ns3::UdpEchoClient::MaxPackets",  UintegerValue(simulationDuration.GetInteger()/sendPeriod.GetInteger()));
  Config::SetDefault("ns3::UdpEchoClient::Interval",TimeValue (sendPeriod));
  Config::SetDefault("ns3::UdpEchoClient::PacketSize",UintegerValue (100));

  NodeContainer nodes;
  nodes.Create (2);
  //GetState
  CsmaHelper csma;

  NetDeviceContainer devices = csma.Install(nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  TsnHelper tsnHelperServer,tsnHelperClient;

  NetDeviceListConfig schedulePlanServer,schedulePlanClient;

  schedulePlanClient.Add(scheduleDuration,{1,1,1,1,1,1,1,1});
  schedulePlanClient.Add(scheduleDuration,{0,0,0,0,0,0,0,0});

  schedulePlanServer.Add(scheduleDuration,{0,0,0,0,0,0,0,0});
  schedulePlanServer.Add(scheduleDuration,{1,1,1,1,1,1,1,1});

  Ptr<CsmaChannel> CsmaClient = DynamicCast<CsmaChannel>(nodes.Get(0)->GetDevice(0)->GetObject<CsmaNetDevice>()->GetChannel());
  Ptr<CsmaChannel> CsmaServer = DynamicCast<CsmaChannel>(nodes.Get(1)->GetDevice(0)->GetObject<CsmaNetDevice>()->GetChannel());

  tsnHelperServer.SetRootQueueDisc("ns3::TasQueueDisc", "CsmaChannel", PointerValue(CsmaClient),"NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanServer),"DataRate", StringValue ("50Mbps"));
  tsnHelperClient.SetRootQueueDisc("ns3::TasQueueDisc", "CsmaChannel", PointerValue(CsmaServer),"NetDeviceListConfig", NetDeviceListConfigValue(schedulePlanClient),"DataRate", StringValue ("50Mbps"));

  tsnHelperServer.AddPacketFilter(0,"ns3::Ipv4Filter2");
  tsnHelperClient.AddPacketFilter(0,"ns3::Ipv4Filter2");

  QueueDiscContainer qdiscsClient1 = tsnHelperClient.Install (devices.Get(0));
  QueueDiscContainer qdiscsServer = tsnHelperServer.Install (devices.Get(1));

  Ipv4AddressHelper address;

  address.SetBase ("0.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  Ipv4Address multicastSource ("10.1.1.1");
  Ipv4Address multicastGroup ("225.1.2.4");

  UdpEchoClientHelper echoClient1 (interfaces.GetAddress (1), 9);

  ApplicationContainer clientApps1 = echoClient1.Install (nodes.Get (0));
  clientApps1.Start (Seconds(0));
  clientApps1.Stop (simulationDuration);

  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (0));
  serverApps.Stop (simulationDuration);

  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (path));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults();
  outputConfig.ConfigureAttributes ();

  csma.EnablePcap ("tas-test6", nodes.Get (1)->GetId(),0);
  csma.EnablePcap ("tas-test6", nodes.Get (0)->GetId(),0);

  std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
  Simulator::Run ();

  std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();

  Simulator::Destroy ();

  std::cout << "Simulation Rate: "<< simulationDuration.GetMilliSeconds() / std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count()<< std::endl;

  std::cout << "RUN" << "\t" << path << std::endl;
  return 0;
}



/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020
 *
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
#include "net-device-list-config.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NetDeviceListConfig");

ATTRIBUTE_HELPER_CPP (NetDeviceListConfig);

NetDeviceListConfig::NetDeviceListConfig ()
{
  m_pandingStatus = PANDING;
  m_configChangeTime = Time(0);
  m_length = Time(0);
  m_epoch = Time(0);
  NS_LOG_FUNCTION (this);
}

NetDeviceListConfig::~NetDeviceListConfig ()
{
  Clear();
  NS_LOG_FUNCTION (this);
}

void NetDeviceListConfig::Copy(NetDeviceListConfig pandingConfig)
{
  m_pandingStatus = pandingConfig.GetPandingStatus();
  m_configChangeTime = pandingConfig.GetConfigChangeTime();
  m_length = pandingConfig.GetLength();
  for(unsigned int i = 0; i < 8; i++)
    {
      m_netDeviceListConfig.at(i).Copy(pandingConfig.GetGateListConfig(i));
    }
}
void NetDeviceListConfig::Clear()
{
  m_pandingStatus = PANDING;
  m_configChangeTime = Time(0);
  m_length = Time(0);
  for(unsigned int i = 0; i < 8; i++)
    {
      m_netDeviceListConfig.at(i).Clear();
    }
}

void NetDeviceListConfig::SetPandingStatus(PandingStatus status)
{
  m_pandingStatus = status;
}
NetDeviceListConfig::PandingStatus NetDeviceListConfig::GetPandingStatus() const
{
  return m_pandingStatus;
}

void NetDeviceListConfig::SetConfigChangeTime(Time offset)
{
  m_configChangeTime = offset;
}
Time NetDeviceListConfig::GetConfigChangeTime() const
{
  return m_configChangeTime;
}

void NetDeviceListConfig::SetLength(Time length)
{
  m_length = length;
}
Time NetDeviceListConfig::GetLength() const
{
  return m_length;
}

void NetDeviceListConfig::SetEpoch(Time epoch)
{
  m_epoch = epoch;
}
Time NetDeviceListConfig::GetEpoch() const
{
  return m_epoch;
}

void NetDeviceListConfig::SetGateListConfig(GateListConfig config, unsigned int queueIndex)
{
  m_netDeviceListConfig.at(queueIndex).Copy(config);
}

NetDeviceListConfig::GateListConfig NetDeviceListConfig::GetGateListConfig(unsigned int queueIndex) const
{
  return m_netDeviceListConfig.at(queueIndex);
}

void NetDeviceListConfig::Add(Time duration, NetDeviceListConfig::GateStateMap gateStateMap, Time startOffset, Time stopOffset)
{

  for(unsigned int i = 0; i < 8; i++)
    {
      if(gateStateMap.at(i)){
          m_netDeviceListConfig.at(i).Add(m_length,m_length+duration,startOffset,stopOffset);
      }
    }
  m_length += duration;
}

/**
 * Serialize the TasConfig to the given ostream
 */

std::ostream &operator << (std::ostream &os, const NetDeviceListConfig::PandingStatus &status)
{
  os << static_cast<std::underlying_type<NetDeviceListConfig::PandingStatus>::type>(status) << "|";
  return os;
}
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig::GateStateMap &gateStateMap)
{
  if( (unsigned int) gateStateMap.size() != 8 ){
      NS_FATAL_ERROR ("Incomplete schedule QostagsMap specification (" << (unsigned int) gateStateMap.size() << " values provided, " << 8 << " required)");
    }

    for(int i = 0; i < 8; i++){
     os << gateStateMap[i] << "|";
    }

    return os;
}
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig::GateListConfig &gateListConfig)
{
  if(gateListConfig.m_openingTimes.size() != gateListConfig.m_closingTimes.size())
    {
      NS_FATAL_ERROR ("Error in lookUpElement");
    }
    unsigned int vectorSize = gateListConfig.m_openingTimes.size();
    os << vectorSize << "|";

    auto openTimesItr = gateListConfig.m_openingTimes.begin();
    auto closeTimesItr = gateListConfig.m_closingTimes.begin();
    auto openOffsetItr = gateListConfig.m_openOffset.begin();
    auto closeOffsetItr = gateListConfig.m_closeOffset.begin();

    for(unsigned int i= 0; i < vectorSize; i++)
    {
      os << "[" << *(openTimesItr+i) << ";" << *(closeTimesItr+i) << ";" << *(openOffsetItr+i) << ";" << *(closeOffsetItr+i) << "]";
    }

    os << "|";

    return os;
}
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig &netDeviceListConfig)
{
  os << netDeviceListConfig.GetPandingStatus();
  os << netDeviceListConfig.GetConfigChangeTime() << "|";
  os << netDeviceListConfig.GetLength() << "|";
  os << netDeviceListConfig.GetEpoch() << "|";

  for(unsigned int i = 0; i < 8; i++)
    {
      os << netDeviceListConfig.GetGateListConfig(i);
    }
  return os;
}

/**
 * Serialize from the given istream to this TasConfig.
 */

std::string cutOff(std::istream &is, char charater)
{
  std::string cutted;
  char temp;
  while(is >> temp && temp != charater)
   {
      cutted += temp;
   }

  return cutted;
}

std::istream &operator >> (std::istream &is, NetDeviceListConfig::PandingStatus &status)
{
  unsigned int temp = 0;
    if (is >> temp)
      status = static_cast<NetDeviceListConfig::PandingStatus>(temp);
  return is;
}
std::istream &operator >> (std::istream &is, NetDeviceListConfig::GateStateMap &gateStateMap)
{
  char c;
  for (int i = 0; i < 8; i++)
   {
     if (!(is >> gateStateMap[i] >> c))
     {
       NS_FATAL_ERROR ("Incomplete QostagsMap specification (" << i << " values provided, " << 8 << " required)");
     }
   }
  return is;
}
std::istream &operator >> (std::istream &is, NetDeviceListConfig::GateListConfig &gateListConfig)
{
  unsigned int vectorSize = 0;
  char endCarakter;
  is >> vectorSize >> endCarakter;
  NS_ASSERT(endCarakter == '|');

  for(unsigned int i = 0; i < vectorSize; i++)
  {
    is >> endCarakter;
    NS_ASSERT(endCarakter == '[');
    Time openTime,closeTime,openOffset,closeOffset;
    std::istringstream(cutOff(is,';')) >> openTime;
    std::istringstream(cutOff(is,';')) >> closeTime;
    std::istringstream(cutOff(is,';')) >> openOffset;
    std::istringstream(cutOff(is,']')) >> closeOffset;
    gateListConfig.Add(openTime,closeTime,openOffset,closeOffset);
  }
  is >> endCarakter;
  NS_ASSERT(endCarakter == '|');
  return is;
}
std::istream &operator >> (std::istream &is, NetDeviceListConfig &netDeviceListConfig)
{
  char c;
  NetDeviceListConfig::PandingStatus status;
  is >> status >> c;

  netDeviceListConfig.SetPandingStatus(status);
  Time temp;

  std::istringstream(cutOff(is,'|')) >> temp;
  netDeviceListConfig.SetConfigChangeTime(temp);
  std::istringstream(cutOff(is,'|')) >> temp;
  netDeviceListConfig.SetLength(temp);
  std::istringstream(cutOff(is,'|')) >> temp;
  netDeviceListConfig.SetEpoch(temp);

  for(unsigned int i = 0; i < 8; i++)
    {
      NetDeviceListConfig::GateListConfig temp;
      is >> temp;
      netDeviceListConfig.SetGateListConfig(temp , i);
    }

  is.peek(); // Needed to set EOF flag
  return is;
}

} /* namespace ns3 */

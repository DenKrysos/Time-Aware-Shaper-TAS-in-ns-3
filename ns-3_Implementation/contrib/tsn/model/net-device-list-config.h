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

#ifndef CONTRIB_TSN_MODEL_NET_DEVICE_LIST_CONFIG_H_
#define CONTRIB_TSN_MODEL_NET_DEVICE_LIST_CONFIG_H_

#include <string>
#include <iostream>
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include <array>
#include <vector>

namespace ns3 {

class NetDeviceListConfig
{
public:
  enum PandingStatus{PANDING,ACTIVE,TERMINATED};

  typedef std::array<bool,8> GateStateMap;

  struct GateListConfig
  {
    void Add(Time opens, Time closes, Time openOffset, Time closeOffset)
    {
      unsigned int vectorSize = m_closingTimes.size(); //They all have the same size
      Time openTime,closesTime = closes-closeOffset;

      if(vectorSize-- > 0)
      {
        if(m_closingTimes.at(vectorSize) + m_closeOffset.at(vectorSize) == opens)
          {
            openTime = m_openingTimes.at(vectorSize);
            if(openTime > closesTime)
              {
                NS_FATAL_ERROR("Canot open bevor close");
              }
            m_closingTimes.at(vectorSize) = closesTime;
            m_closeOffset.at(vectorSize) = closeOffset;
            return;
          }
      }
      openTime = opens+openOffset;

      if(openTime > closesTime)
        {
          NS_FATAL_ERROR("Canot open bevor close");
        }
      m_openingTimes.push_back(openTime);
      m_closingTimes.push_back(closesTime);
      m_openOffset.push_back(openOffset);
      m_closeOffset.push_back(closeOffset);
    }
    void Remove(uint32_t index)
    {
      if( index <  m_openingTimes.size() && index < m_closingTimes.size())
      {
          m_openingTimes.erase(m_openingTimes.begin() + index);
          m_closingTimes.erase(m_closingTimes.begin() + index);
          m_openOffset.erase(m_openOffset.begin() + index);
          m_closeOffset.erase(m_closeOffset.begin() + index);
      }
    }
    void Copy(GateListConfig gateListConfig)
    {
      Clear();
      auto startItr = gateListConfig.m_openingTimes.begin();
      auto closItr = gateListConfig.m_closingTimes.begin();
      auto startOffsetItr = gateListConfig.m_openOffset.begin();
      auto closOffsetItr = gateListConfig.m_closeOffset.begin();

      unsigned int length = gateListConfig.m_openingTimes.size();

      for(unsigned int i = 0; i < length; i++){
          Add(*(startItr+i),*(closItr+i),*(startOffsetItr+i),*(closOffsetItr+i));
      }
    }
    void Clear(void)
    {
      m_openingTimes.clear();
      m_closingTimes.clear();
      m_openOffset.clear();
      m_closeOffset.clear();
    }

    std::vector<Time> m_openingTimes;
    std::vector<Time> m_closingTimes;
    std::vector<Time> m_openOffset;
    std::vector<Time> m_closeOffset;
  };

  NetDeviceListConfig ();

  virtual ~NetDeviceListConfig();

  void Copy(NetDeviceListConfig pandingConfig);
  void Clear();

  void SetPandingStatus(PandingStatus status);
  PandingStatus GetPandingStatus() const;

  void SetConfigChangeTime(Time offset);
  Time GetConfigChangeTime() const;

  void SetLength(Time length);
  Time GetLength() const;

  void SetEpoch(Time length);
  Time GetEpoch() const;

  void SetGateListConfig(GateListConfig config, unsigned int queueIndex);
  GateListConfig GetGateListConfig(unsigned int queueIndex) const;

  void Add(Time duration, GateStateMap gateStateMap, Time startOffset = Time(0), Time stopOffset = Time(0));

private:
  PandingStatus m_pandingStatus;
  Time m_configChangeTime;
  Time m_length;
  Time m_epoch;
  std::array<GateListConfig,8> m_netDeviceListConfig;
};

/**
 * Serialize the TasConfig to the given ostream
 */
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig::PandingStatus &status);
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig::GateStateMap &gateStateMap);
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig::GateListConfig &gateListConfig);
std::ostream &operator << (std::ostream &os, const NetDeviceListConfig &netDeviceListConfig);

/**
 * Serialize from the given istream to this TasConfig.
 */
std::istream &operator >> (std::istream &is, NetDeviceListConfig::PandingStatus &status);
std::istream &operator >> (std::istream &is, NetDeviceListConfig::GateStateMap &gateStateMap);
std::istream &operator >> (std::istream &is, NetDeviceListConfig::GateListConfig &gateListConfig);
std::istream &operator >> (std::istream &is, NetDeviceListConfig &netDeviceListConfig);

//NetDeviceListConfig &operator+ (NetDeviceListConfig lht,NetDeviceListConfig rht);

ATTRIBUTE_HELPER_HEADER (NetDeviceListConfig);

} /* namespace ns3 */

#endif /* CONTRIB_TSN_MODEL_NET_DEVICE_LIST_CONFIG_H_ */

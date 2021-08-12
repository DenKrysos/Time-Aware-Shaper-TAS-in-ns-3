/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) YEAR COPYRIGHTHOLDER
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

#ifndef SRC_TSN_HELPER_TSN_HELPER_H_
#define SRC_TSN_HELPER_TSN_HELPER_H_

#include "ns3/traffic-control-helper.h"
#include "ns3/net-device-list-config.h"

namespace ns3
{

class TsnHelper: public TrafficControlHelper
{
public:

  void addSchedule(Time duration, NetDeviceListConfig::GateStateMap gateStateMap, Time startOffset, Time stopOffset);

  TsnHelper();
  virtual ~TsnHelper();
private:
  NetDeviceListConfig m_schudle;
};

} /* namespace ns3 */

#endif /* SRC_TSN_HELPER_TSN_HELPER_H_ */

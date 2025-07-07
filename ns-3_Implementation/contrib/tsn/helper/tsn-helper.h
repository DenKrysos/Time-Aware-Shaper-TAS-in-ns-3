/*
 * Copyright (c) YEAR COPYRIGHTHOLDER
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
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

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

#include "ns3/log.h"
#include "tsn-helper.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("TsnHelper");

TsnHelper::TsnHelper()
{
}

TsnHelper::~TsnHelper()
{
}

void
TsnHelper::addSchedule(Time duration, NetDeviceListConfig::GateStateMap gateStateMap, Time startOffset, Time stopOffset){
  m_schudle.Add(duration,gateStateMap,startOffset,stopOffset);
}

} /* namespace ns3 */

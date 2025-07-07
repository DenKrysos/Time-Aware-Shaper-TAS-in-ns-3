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

#ifndef SRC_TSN_MODEL_TSNIPV6PACKETFILTER_H_
#define SRC_TSN_MODEL_TSNIPV6PACKETFILTER_H_

#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/queue-disc.h"
#include "ns3/packet-filter.h"

namespace ns3
{

class TsnIpv6PacketFilter: public PacketFilter
{
public:
  static TypeId GetTypeId ();

  TsnIpv6PacketFilter();
  virtual ~TsnIpv6PacketFilter();

private:
  virtual bool CheckProtocol (Ptr<QueueDiscItem> item) const;
  int32_t DoClassify (Ptr<QueueDiscItem> item) const;
  Callback <int32_t,Ptr<QueueDiscItem>> m_DoClassify;
};

} /* namespace ns3 */

#endif /* SRC_TSN_MODEL_TSNIPV6PACKETFILTER_H_ */

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef SRC_TSN_MODEL_TSNIPV4PACKETFILTER_H_
#define SRC_TSN_MODEL_TSNIPV4PACKETFILTER_H_

#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/queue-disc.h"
#include "ns3/packet-filter.h"
#include "ns3/ipv4-l3-protocol.h"

namespace ns3
{

class TsnIpv4PacketFilter: public PacketFilter
{
public:
  static TypeId GetTypeId ();

  TsnIpv4PacketFilter();
  virtual ~TsnIpv4PacketFilter();

private:
  virtual bool CheckProtocol (Ptr<QueueDiscItem> item) const;
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const;
  Callback <int32_t,Ptr<QueueDiscItem>> m_DoClassify;
};

} /* namespace ns3 */

#endif /* SRC_TSN_MODEL_TSNIPV4PACKETFILTER_H_ */

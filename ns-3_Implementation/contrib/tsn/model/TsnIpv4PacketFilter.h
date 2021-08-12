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
  static TypeId GetTypeId (void);

  TsnIpv4PacketFilter();
  virtual ~TsnIpv4PacketFilter();

private:
  virtual bool CheckProtocol (Ptr<QueueDiscItem> item) const;
  virtual int32_t DoClassify (Ptr<QueueDiscItem> item) const;
  Callback <int32_t,Ptr<QueueDiscItem>> m_DoClassify;
};

} /* namespace ns3 */

#endif /* SRC_TSN_MODEL_TSNIPV4PACKETFILTER_H_ */

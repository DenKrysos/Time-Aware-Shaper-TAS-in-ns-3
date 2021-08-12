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

#include "ns3/log.h"
#include "TsnIpv4PacketFilter.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("TsnIpv4PacketFilter");

NS_OBJECT_ENSURE_REGISTERED (TsnIpv4PacketFilter);

TypeId
TsnIpv4PacketFilter::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TsnIpv4PacketFilter")
    .SetParent<PacketFilter> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<TsnIpv4PacketFilter> ()
    .AddAttribute("Classify",
                   "function callback to Classify packet ",
                   CallbackValue (),
                   MakeCallbackAccessor(&TsnIpv4PacketFilter::m_DoClassify),
                   MakeCallbackChecker()
               )
  ;
  return tid;
}

TsnIpv4PacketFilter::TsnIpv4PacketFilter()
{
  NS_LOG_FUNCTION (this);
}

TsnIpv4PacketFilter::~TsnIpv4PacketFilter()
{
  NS_LOG_FUNCTION (this);
}

bool
TsnIpv4PacketFilter::CheckProtocol (Ptr<QueueDiscItem> item) const
{
  NS_LOG_FUNCTION (this << item);
  return (DynamicCast<Ipv4QueueDiscItem> (item) != 0);
}

int32_t
TsnIpv4PacketFilter::DoClassify(Ptr<QueueDiscItem> item) const
{

  if(!m_DoClassify.IsNull() && CheckProtocol(item)){
    return m_DoClassify(item);
  }

  return PacketFilter::PF_NO_MATCH;
}

} /* namespace ns3 */

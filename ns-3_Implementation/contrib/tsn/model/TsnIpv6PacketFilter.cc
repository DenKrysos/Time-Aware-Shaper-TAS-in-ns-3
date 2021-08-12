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
#include "TsnIpv6PacketFilter.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("TsnIpv6PacketFilter");

NS_OBJECT_ENSURE_REGISTERED (TsnIpv6PacketFilter);

TypeId
TsnIpv6PacketFilter::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TsnIpv6PacketFilter")
    .SetParent<PacketFilter> ()
    .SetGroupName ("Internet")
    .AddAttribute("Classify",
                     "function callback to Classify packet ",
                     CallbackValue (),
                     MakeCallbackAccessor(&TsnIpv6PacketFilter::m_DoClassify),
                     MakeCallbackChecker()
                 )
  ;
  return tid;
}

TsnIpv6PacketFilter::TsnIpv6PacketFilter()
{
  NS_LOG_FUNCTION (this);
}

TsnIpv6PacketFilter::~TsnIpv6PacketFilter()
{
  NS_LOG_FUNCTION (this);
}

bool
TsnIpv6PacketFilter::CheckProtocol (Ptr<QueueDiscItem> item) const
{
  NS_LOG_FUNCTION (this << item);
  return (DynamicCast<Ipv6QueueDiscItem> (item) != 0);
}

int32_t
TsnIpv6PacketFilter::DoClassify(Ptr<QueueDiscItem> item) const
{
  if(!m_DoClassify.IsNull()){
     return m_DoClassify(item);
   }

  return PacketFilter::PF_NO_MATCH;
}

} /* namespace ns3 */

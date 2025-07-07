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
#include "TsnIpv6PacketFilter.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("TsnIpv6PacketFilter");

NS_OBJECT_ENSURE_REGISTERED (TsnIpv6PacketFilter);

TypeId
TsnIpv6PacketFilter::GetTypeId ()
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
  return (DynamicCast<Ipv6QueueDiscItem> (item) != nullptr);
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

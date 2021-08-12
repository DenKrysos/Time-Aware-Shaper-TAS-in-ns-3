/*
 * Ipv4Filter.cc
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
#include "Ipv4Filter.h"
#include "ns3/arp-queue-disc-item.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipv4Filter");

NS_OBJECT_ENSURE_REGISTERED (Ipv4Filter);

TypeId
Ipv4Filter::GetTypeId (void)
{

  static TypeId tid = TypeId ("ns3::Ipv4Filter")
     .SetParent<PacketFilter> ()
     .SetGroupName ("TrafficControl")
     .AddConstructor<Ipv4Filter>()
   ;
  return tid;
}

Ipv4Filter::Ipv4Filter()
{
  NS_LOG_FUNCTION (this);
}

Ipv4Filter::~Ipv4Filter()
{
  NS_LOG_FUNCTION (this);
}

bool
Ipv4Filter::CheckProtocol (Ptr<QueueDiscItem> item) const
{
  NS_LOG_FUNCTION (this << item);
  return item->GetProtocol() == 0x0806;
}

int32_t
Ipv4Filter::DoClassify(Ptr<QueueDiscItem> item) const
{
  if(CheckProtocol(item)){
      return 7;

  }
  return PacketFilter::PF_NO_MATCH;
}

}/* namespace ns3 */

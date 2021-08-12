/*
 * Ipv4Filter.h
 */
/*
 * 2020-08 (Aug 2020)
 * Author(s):
 *  - Implementation:
 *       Luca Wendling <lwendlin@rhrk.uni-kl.de>
 *  - Design & Implementation:
 *       Dennis Krummacker <dennis.krummacker@gmail.com>
 */

#ifndef CONTRIB_TSN_MODEL_IPV4FILTER_H_
#define CONTRIB_TSN_MODEL_IPV4FILTER_H_

#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/queue-disc.h"
#include "ns3/packet-filter.h"
#include "ns3/ipv4-l3-protocol.h"

namespace ns3 {

class Ipv4Filter : public PacketFilter{
public:
  static TypeId GetTypeId (void);
  Ipv4Filter();
  virtual ~Ipv4Filter();

private:
  bool CheckProtocol (Ptr<QueueDiscItem> item) const;

  int32_t  DoClassify (Ptr<QueueDiscItem> item) const;
};

} /* namespace ns3 */

#endif /* CONTRIB_TSN_MODEL_IPV4FILTER_H_ */

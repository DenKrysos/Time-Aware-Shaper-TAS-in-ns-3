/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020
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

#ifndef CONTRIB_TSN_MODEL_TRANSMISSON_GATE_QDISC_H_
#define CONTRIB_TSN_MODEL_TRANSMISSON_GATE_QDISC_H_

#include "ns3/queue-disc.h"
#include "ns3/data-rate.h"
#include <array>
#include <vector>
#include <math.h>
#include <utility>
#include <algorithm>
#include <stdexcept>

#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/net-device-queue-interface.h"

namespace ns3 {

typedef struct GateState{
  bool state;
  Time interval;
  GateState(){
    state = true;
    interval = Time(-1);
  }
  void copy(const GateState newState)
  {
    state = newState.state;
    interval = newState.interval;
  }
}GateState;

class TransmissonGateQdisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief TasQueueDisc constructor
   *
   */
  TransmissonGateQdisc ();

  virtual ~TransmissonGateQdisc();

  bool SetGateState(GateState newState);
  GateState GetGateState() const;

  static constexpr const char* LIMIT_EXCEEDED_DROP = "Queue disc limit exceeded";  //!< Packet dropped due to queue disc limit exceeded
  static constexpr const char* PAKET_LIVE_EXCEEDED_DROP = "Paket live time exceeded";  //!< Packet dropped due to live time exceeded


private:
  virtual void InitializeParams (void);

  virtual bool CheckConfig (void);
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);

  virtual Time GetTransmissionDuration(Ptr<const QueueDiscItem> paketref);

  virtual void CheckLiveTimes();

  Time GetDeviceTime();

  DataRate m_linkBandwidth; //Link Data Rate
  GateState m_gateState;
  Callback <Time> m_getNow;
  Time m_keepAliveTime;
};

std::ostream &operator << (std::ostream &os, const GateState &state);
std::istream &operator >> (std::istream &is, GateState &state);

ATTRIBUTE_HELPER_HEADER (GateState);

}
#endif /* CONTRIB_TSN_MODEL_TRANSMISSON_GATE_QDISC_H_ */

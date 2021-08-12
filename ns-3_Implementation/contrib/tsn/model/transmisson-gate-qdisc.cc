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

#include "transmisson-gate-qdisc.h"

#include "ns3/queue.h"
#include "ns3/socket.h"

#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "ns3/simulator.h"
#include "ns3/tag.h"
#include "ns3/pointer.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TransmissonGateQdisc");

ATTRIBUTE_HELPER_CPP (GateState);

std::ostream &operator << (std::ostream &os, const GateState &state)
{
  os << state.state;
  os << state.interval;
 return os;
}

std::istream &operator >> (std::istream &is, GateState &state)
{
  is >> state.state;
  is >> state.interval;
  return is;
}

NS_OBJECT_ENSURE_REGISTERED (TransmissonGateQdisc);

TypeId TransmissonGateQdisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TransmissonGateQdisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("tsn")
    .AddConstructor<TransmissonGateQdisc> ()
   .AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("800p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ()
                   )
   .AddAttribute("GateState",
                 "State of the Gate",
                 GateStateValue(GateState()),
                 MakeGateStateAccessor(&TransmissonGateQdisc::SetGateState,
                                     &TransmissonGateQdisc::GetGateState),
                 MakeGateStateChecker()
                 )
   .AddAttribute ("DataRate",
                  "DataRate of conected link",
                  DataRateValue (DataRate ("1.5Mbps")),
                  MakeDataRateAccessor (&TransmissonGateQdisc::m_linkBandwidth),
                  MakeDataRateChecker ())
   .AddAttribute("TimeSource",
                 "function callback to get Current Time ",
                 CallbackValue (),
                 MakeCallbackAccessor(&TransmissonGateQdisc::m_getNow),
                 MakeCallbackChecker()
             )
     .AddAttribute("PaketLiveTime",
                   "the time a packed stays enqueued bevor it gets dropped if it coudlt be send",
                   TimeValue(Time(-1)),
                   MakeTimeAccessor(&TransmissonGateQdisc::m_keepAliveTime),
                   MakeTimeChecker()
                   )
  ;
  return tid;
}

TransmissonGateQdisc::TransmissonGateQdisc ()
  : QueueDisc (QueueDiscSizePolicy::MULTIPLE_QUEUES)
{
  NS_LOG_FUNCTION (this);
}

TransmissonGateQdisc::~TransmissonGateQdisc ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<const QueueDiscItem>
TransmissonGateQdisc::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  return GetInternalQueue(0)->Peek();
}

bool
TransmissonGateQdisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  CheckLiveTimes();

  if(GetInternalQueue(0)->GetMaxSize() == QueueSize(QueueSizeUnit::PACKETS,GetInternalQueue(0)->GetNPackets()))
    {
      DropBeforeEnqueue(item,LIMIT_EXCEEDED_DROP);
      return false;
    }
  bool retval = GetInternalQueue(0)->Enqueue (item);

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue(0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue(0)->GetNBytes ());

  return retval;
}

Time
TransmissonGateQdisc::GetTransmissionDuration(Ptr<const QueueDiscItem> paketref){
  return m_linkBandwidth.CalculateBytesTxTime(paketref->GetSize());
}

Ptr<QueueDiscItem>
TransmissonGateQdisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  CheckLiveTimes();

  auto queuePointer = GetInternalQueue(0);

  Ptr<const QueueDiscItem> paketref = queuePointer->Peek();

  if(paketref == 0)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  if(m_gateState.state)
    {
      if(m_gateState.interval.IsStrictlyPositive())
        {
          Time transmissionDuration = GetTransmissionDuration(paketref);
          Time window = m_gateState.interval - GetDeviceTime();

          if(transmissionDuration > window)
            {
              NS_LOG_LOGIC("Dequeueing susbendet to time constrain");
              return 0;
            }
        }
      return queuePointer->Dequeue();
    }
    NS_LOG_LOGIC("Queue closed number of items stored : " << GetInternalQueue(0)->GetNPackets());
  return 0;
}


Time
TransmissonGateQdisc::GetDeviceTime()
{

  if(!m_getNow.IsNull()){
    return m_getNow();
  }
  return Simulator::Now();
}

bool
TransmissonGateQdisc::SetGateState(GateState newState)
{
  m_gateState.copy(newState);
  return true;
}

GateState
TransmissonGateQdisc::GetGateState() const
{
  return m_gateState;
}

void
TransmissonGateQdisc::CheckLiveTimes()
{
  auto queuePointer = GetInternalQueue(0);
  Ptr<const QueueDiscItem> paketref;

  bool drop = false;
  Time now = GetDeviceTime();
  do
    {
      paketref = queuePointer->Peek();
      drop = false;

      if(m_keepAliveTime.IsStrictlyPositive() && paketref != 0 && paketref->GetTimeStamp() < (now - m_keepAliveTime))
        {
          DropAfterDequeue(queuePointer->Dequeue(),PAKET_LIVE_EXCEEDED_DROP);
          drop = true;
        }
    }while(drop);

}

bool
TransmissonGateQdisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("TransmissonGateQdisc cannot have classes");
      return false;
    }
  if(GetNInternalQueues())
    {
      NS_LOG_ERROR ("TransmissonGateQdisc cannot have internal queues");
      return false;
    }
  if (GetNInternalQueues () == 0)
      {
        AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                        ("MaxSize", QueueSizeValue (GetMaxSize())));
        AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                                ("MaxSize", QueueSizeValue (QueueSize(QueueSizeUnit::PACKETS,1))));
      }
  return true;
}

void
TransmissonGateQdisc::InitializeParams (void)
{
  m_gateState.state = true;
  m_gateState.interval = Time(-1);
  NS_LOG_FUNCTION (this);
}

}//End of NameSpace

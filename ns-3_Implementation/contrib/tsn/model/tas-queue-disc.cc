/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 
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

#include "ns3/queue.h"
#include "ns3/socket.h"

#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "ns3/transmisson-gate-qdisc.h"
#include "ns3/simulator.h"
#include "ns3/tag.h"
#include "ns3/pointer.h"
#include "tas-queue-disc.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TasQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (TasQueueDisc);

TypeId TasQueueDisc::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TasQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<TasQueueDisc> ()
    .AddAttribute ("DataRate",
                   "DataRate of conected link",
                   DataRateValue (DataRate ("1.5Mbps")),
                   MakeDataRateAccessor (&TasQueueDisc::SetDatarate),
                   MakeDataRateChecker ()
                   )
     .AddAttribute ("Mtu",
                    "Max Transmisson Unit in Bytes",
                    IntegerValue(1500),
                    MakeIntegerAccessor(&TasQueueDisc::m_Mtu),
                    MakeIntegerChecker<unsigned int> ()
                     )
     .AddAttribute("TimeSource",
                   "function callback to get Current Time ",
                   CallbackValue (),
                   MakeCallbackAccessor (&TasQueueDisc::SetTimeSource,
                                         &TasQueueDisc::GetTimeSource),
                   MakeCallbackChecker ()
                   )
    .AddAttribute ("NetDeviceListConfig",
                    "Defines if the Quality of Service should be trusted",
                    NetDeviceListConfigValue(NetDeviceListConfig()),
                    MakeNetDeviceListConfigAccessor(&TasQueueDisc::SetNetDeviceListConfig,
                                                    &TasQueueDisc::GetNetDeviceListConfig),
                    MakeNetDeviceListConfigChecker()
                   )
   .AddAttribute ("MaxSize",
                   "The maximum number of packets accepted by this queue disc",
                   QueueSizeValue (QueueSize ("800p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ()
                   )
   .AddAttribute ("TrustQostag",
                       "Defines if the Quality of Service should be trusted",
                       BooleanValue(false),
                       MakeBooleanAccessor(&TasQueueDisc::m_trustQostag),
                       MakeBooleanChecker()
                  )
    .AddAttribute("Delay",
                  "Propagation Delay",
                  TimeValue(Time(-1)),
                  MakeTimeAccessor(&TasQueueDisc::m_propagetiondelay),
                  MakeTimeChecker()
                  )
    .AddAttribute("CsmaChannel",
                  "CsmaChannel Ptr",
                  PointerValue(),
                  MakePointerAccessor(&TasQueueDisc::m_channel),
                  MakePointerChecker<CsmaChannel>()
                  )
    .AddAttribute("PaketLiveTime",
                  "the time a packed stays enqueued bevor it gets dropped if it coudlt be send",
                  TimeValue(Time(-1)),
                  MakeTimeAccessor(&TasQueueDisc::m_keepAliveTime),
                  MakeTimeChecker()
                  )
  ;
  return tid;
}

TasQueueDisc::TasQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::MULTIPLE_QUEUES)
{
  NS_LOG_FUNCTION (this);
}

TasQueueDisc::~TasQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<const QueueDiscItem>
TasQueueDisc::DoPeek ()
{
  NS_LOG_FUNCTION (this);

  Ptr<const QueueDiscItem> item;

  Time now = GetDeviceTime();

  for(int32_t i = GetNQueueDiscClasses(); i > 0; i--)
    {
      unsigned char prio = i-1;

      if(m_gateListConfig.at(prio).GetNextEvent().IsPositive() && !(m_gateListConfig.at(prio).GetNextEvent() < now))
        {
          ListExecute(); //Update Queues
        }

       if(m_gateListConfig.at(prio).GetGateState() && ( (item = GetQueueDiscClass (prio)->GetQueueDisc ()->Peek ()) != nullptr ) )
        {
           NS_LOG_LOGIC ("Peeked from band " << (prio) << ": " << item);
           return item;
        }
    }
  NS_LOG_LOGIC ("Queue empty");
  return 0;
}

bool
TasQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  int32_t childclass = 0; // 0 Best effort Queue
  SocketPriorityTag qosTag;

  if(item->GetSize() > m_Mtu)
    {
      DropBeforeEnqueue(item,PAKET_SIZE_EXCEEDED_DROP);
      return false;
    }

  if(m_trustQostag && item->GetPacket ()->PeekPacketTag (qosTag))
    {
      childclass = qosTag.GetPriority () & 0x7;
    }
  else
    {
      childclass = Classify(item);
      if (childclass == PacketFilter::PF_NO_MATCH)
          {
             //NS_LOG_DEBUG ("No filter has been able to classify this packet.");
             childclass = 0;
          }
        else
          {
            //NS_LOG_DEBUG ("Packet filters returned " << ret);
            childclass = childclass & 0x7;
          }
    }

  qosTag.SetPriority(childclass);
  item->GetPacket()->RemovePacketTag(qosTag);
  item->GetPacket()->AddPacketTag(qosTag);

  bool retval = GetQueueDiscClass(m_prioMap[childclass])->GetQueueDisc ()->Enqueue (item);

  if(retval && !m_gateListConfig.at(m_prioMap[childclass]).GetGateState() && m_gateListConfig.at(m_prioMap[childclass]).reRunEvent.IsExpired())
    {
      ListExecute(); //To Update the Dequeue Event list
    }
  NS_LOG_LOGIC ("Paket enqueued in " << (int)m_prioMap[childclass] << " Pakets in Queue: " << GetQueueDiscClass(m_prioMap[childclass])->GetQueueDisc ()->GetNPackets ());

  return retval;
}

Ptr<QueueDiscItem>
TasQueueDisc::DoDequeue ()
{

  NS_LOG_FUNCTION (this);
  Time now = GetDeviceTime();

   if(m_transmitting.IsExpired())
    {
       if (m_channel != nullptr && m_channel->IsBusy()){
            m_transmitting = Simulator::Schedule(m_linkBandwidth.CalculateBytesTxTime(m_Mtu/2),&TasQueueDisc::Run,this);
            return 0;
       }

       Ptr<QueueDiscItem> returnVal;
       Ptr<QueueDisc> queuePtr;
       unsigned int prio = 0;
       for(int32_t i =  GetNQueueDiscClasses(); i > 0; i--)
        {
          prio = i-1;
          if(m_gateListConfig.at(prio).GetNextEvent().IsPositive() && !(m_gateListConfig.at(prio).GetNextEvent() > now) )
            {
              ListExecute(); //Update Queues
            }

          queuePtr = GetQueueDiscClass(prio)->GetQueueDisc();
          returnVal = queuePtr->Dequeue();

          if (returnVal != nullptr)
            {
              SetTransmitting(returnVal);
              NS_LOG_LOGIC("Paket Dequeued " << now );
              return returnVal;
            }
        }
    }
   else
     {
       NS_LOG_LOGIC("Transmitting " << now);
     }
   NS_LOG_LOGIC("No pakets to dequeue");
  return 0;
}

void
TasQueueDisc::UpdateNetDeviceListConfig()
{
  m_currentConfig.Copy(m_pandingConfig);
  m_currentConfig.SetPandingStatus(NetDeviceListConfig::ACTIVE);
  m_pandingConfig.Clear();
  m_pandingConfig.SetPandingStatus(NetDeviceListConfig::ACTIVE);
  m_StoppAllQueues = false;

  if(IsInitialized())
    {
      ListExecute();
      Run();
    }
}

void
TasQueueDisc::StopAllQueues(Time duration)
{
  if(IsInitialized())
    {
      for(unsigned int i = 0; i < 8 ; i++)
        {
          UpdateTransmisionGate(i,false,Time(0));
          m_gateListConfig.at(i).reRunEvent.Cancel();
          m_gateListConfig.at(i).reRunEventTimeStamp = Time(-1);
        }
      m_StoppAllQueues = true;
    }
}

void
TasQueueDisc::SetNetDeviceListConfig(NetDeviceListConfig pandingConfig)
{
  m_pandingConfig.Copy(pandingConfig);
  m_pandingConfig.SetPandingStatus(NetDeviceListConfig::PANDING);

  m_stopAllQEvent.Cancel();
  m_updateNDLCEvent.Cancel();

  Time delta = m_pandingConfig.GetConfigChangeTime() - GetDeviceTime();

  //Calculate Transmison time result is in Picoseconds
  Time transmissionTime = m_linkBandwidth.CalculateBytesTxTime(m_Mtu);
  delta -=  transmissionTime;

  if(delta <= Time(0))
    {
      StopAllQueues();

      if(m_currentConfig.GetPandingStatus() == NetDeviceListConfig::PANDING || m_currentConfig.GetLength() == Time(0))
        {
          UpdateNetDeviceListConfig();
        }
      else
        {
          m_updateNDLCEvent = ns3::Simulator::Schedule(transmissionTime,&TasQueueDisc::UpdateNetDeviceListConfig, this);
        }

       return;
    }

  m_stopAllQEvent = ns3::Simulator::Schedule(delta, &TasQueueDisc::StopAllQueues, this, Time(0));
  delta += transmissionTime;
  m_updateNDLCEvent = ns3::Simulator::Schedule(delta,&TasQueueDisc::UpdateNetDeviceListConfig, this);
}

void
TasQueueDisc::SetDatarate(DataRate newDataRate)
{
  m_linkBandwidth = newDataRate;
  for(int i = GetNQueueDiscClasses(); i > 0 ; i--)
    {
      GetQueueDiscClass(i-1)->GetQueueDisc()->SetAttribute("DataRate", DataRateValue(newDataRate));
    }
}

void
TasQueueDisc::UpdateTransmisionGate(unsigned int index, bool gateState, Time nextEvent)
{
 if(GetNQueueDiscClasses() > index)
   {
     Ptr<QueueDisc> ptr;
     ptr= GetQueueDiscClass(index)->GetQueueDisc();

     //Update Gates
     if(m_gateListConfig.at(index).GetGateState() != gateState || m_gateListConfig.at(index).GetNextEvent() != nextEvent)
       {
         m_gateListConfig.at(index).SetGateState(gateState);
         m_gateListConfig.at(index).SetNextEvent(nextEvent);

         GateState newState;
         newState.state = m_gateListConfig.at(index).GetGateState();
         newState.interval = m_gateListConfig.at(index).GetNextEvent();

         ptr->SetAttribute("GateState", GateStateValue(newState));
         NS_LOG_LOGIC(this << " Queue: " << index <<" NewState: " << newState.state << " Interval: " << newState.interval.GetMilliSeconds());

       }

     //Update Run Event
     if(nextEvent.IsStrictlyPositive() && ptr->GetNPackets() > 0)
       {
         ScheduleRun(index);
        }
     else if(m_gateListConfig.at(index).reRunEvent.IsPending())
       {
         CancelRun(index);
       }
   }
}


void
TasQueueDisc::ListExecute()
{
  Time deviceTime = GetDeviceTime();
  NS_LOG_FUNCTION (this << deviceTime);

  if(m_StoppAllQueues) return;

  Time relativeDeviceTime = (deviceTime- m_currentConfig.GetConfigChangeTime() -  m_currentConfig.GetEpoch());

  NS_LOG_LOGIC("TasQueueDisc::ListExecute(" << this << ")" << " time: " << deviceTime.GetMilliSeconds());

  if(relativeDeviceTime.IsStrictlyNegative())
   {
     NS_LOG_ERROR ("Negative Time clock skew");
     return;
   }

  int64_t lengthInFemto = m_currentConfig.GetLength().GetFemtoSeconds();

  if(lengthInFemto == 0)
    {
      return;
    }

  Time relativeNow = FemtoSeconds((int64_t)relativeDeviceTime.GetFemtoSeconds() % lengthInFemto);

   /* Calculate the
    *
    *
  */
  for(unsigned int i = 0; i < 8; i++)
     {
        NetDeviceListConfig::GateListConfig config = m_currentConfig.GetGateListConfig(i);
        int64_t nextBiggerIndex = GetNextBiggerEntry(config.m_closingTimes, relativeNow);

        if(nextBiggerIndex < 0)
          {
            //No Entry Found
            NS_LOG_DEBUG("No startpoint for Queue Priority: " + i );
            UpdateTransmisionGate(i,false,Time(0));
            continue;
          }

        bool newgateState;
        Time nextEvent = Time(-1), open = config.m_openingTimes.at(nextBiggerIndex), close = config.m_closingTimes.at(nextBiggerIndex);

        if(open <= relativeNow && relativeNow < close)
          {
            newgateState = true;
            //Get next close eventTime
            nextEvent = close;
            if(  close == m_currentConfig.GetLength()
                &&
                (config.m_openingTimes.at(0) - config.m_openOffset.at(0) ) == Time(0))
              {
                nextEvent = config.m_closingTimes.at(0);
              }
          }
        else
          {
            newgateState = false;
            //Get next open eventTime
            nextEvent = open;
          }
        if(nextEvent < relativeNow)
          {
            nextEvent += m_currentConfig.GetLength();
          }

        UpdateTransmisionGate(i,newgateState,deviceTime - relativeNow + nextEvent);
     }
}

template <typename item>
int64_t
TasQueueDisc::GetNextBiggerEntry(std::vector<item> vector, item data)
{

  if(vector.size() == 0)
  {
    return -1;
  }

  typename std::vector<item>::iterator itr = vector.begin();

  for(; itr < vector.end(); itr++)
  {
    if( *(itr) > data)
    {
      return itr - vector.begin();
    }
  }

  return 0;
}

NetDeviceListConfig
TasQueueDisc::GetNetDeviceListConfig() const
{
  return m_currentConfig;
}

Time
TasQueueDisc::GetDeviceTime()
{

  if(!m_getNow.IsNull()){
    return m_getNow();
  }
  return Simulator::Now();
}

void
TasQueueDisc::ScheduleRun(uint32_t queue)
{
  Time timePoint = m_gateListConfig.at(queue).GetNextEvent();

  if(m_gateListConfig.at(queue).reRunEvent.IsPending() && (m_gateListConfig.at(queue).reRunEventTimeStamp != timePoint || timePoint.IsStrictlyNegative()))
    {
      CancelRun(queue);
      if(timePoint.IsStrictlyNegative()) return;
    }
  else if (m_gateListConfig.at(queue).reRunEvent.IsPending())
      {
        return;
      }

  for(unsigned int i = 0; i < 8; i++)
    {
      if(m_gateListConfig.at(i).reRunEvent.IsPending() && m_gateListConfig.at(i).reRunEventTimeStamp == timePoint)
        {
          m_gateListConfig.at(queue).reRunEvent = m_gateListConfig.at(i).reRunEvent;
          m_gateListConfig.at(queue).reRunEventTimeStamp = m_gateListConfig.at(i).reRunEventTimeStamp;
          return;
        }
    }

  m_gateListConfig.at(queue).reRunEvent = Simulator::Schedule(timePoint-GetDeviceTime(), &TasQueueDisc::Run, this);
  m_gateListConfig.at(queue).reRunEventTimeStamp = timePoint;

  NS_LOG_LOGIC(this << " Scheduled Run for queue: " << queue << " in time point: " << (timePoint).GetMilliSeconds() << "ms");
  return;
}

bool
TasQueueDisc::CheckConfig ()
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () == 0)
    {
       CallbackValue myEnqueueCallBack = MakeCallback(&TasQueueDisc::Enqueue,this);
       ObjectFactory factory;
       factory.SetTypeId("ns3::TransmissonGateQdisc");
       factory.Set("TimeSource", CallbackValue(m_getNow));
       factory.Set("DataRate", DataRateValue(m_linkBandwidth));
       factory.Set("PaketLiveTime",TimeValue(m_keepAliveTime));

       GateState defaultGateState;

       for (uint8_t i = 0; i < 8; i++)
         {
           defaultGateState.state = m_gateListConfig.at(i).GetGateState();
           defaultGateState.interval = m_gateListConfig.at(i).GetNextEvent();
           factory.Set("GateState", GateStateValue(defaultGateState));

           Ptr<QueueDisc> qd = factory.Create<QueueDisc> ();
           qd->Initialize ();
           Ptr<QueueDiscClass> c = CreateObject<QueueDiscClass> ();
           c->SetQueueDisc (qd);
           AddQueueDiscClass (c);
         }
    }
  else if(GetNQueueDiscClasses() > 8)
    {
      NS_LOG_ERROR ("TasQueueDisc cannot have more than 8 classes");
      return false;
    }

  if (GetNInternalQueues () > 0 )
    {
      NS_LOG_ERROR ("TasQueueDisc cannot have internal Queues ");
      return false;
    }

  return true;
}

void
TasQueueDisc::ResetGateStateList()
{
  const GateListEntry defaultEntry;
  for(unsigned int i = GetNQueueDiscClasses(); i > 0; i--)
    {
      m_gateListConfig.at(i-1).Copy(defaultEntry);
      UpdateTransmisionGate(i-1,defaultEntry.GetGateState(),defaultEntry.GetNextEvent());
      m_StoppAllQueues = false;
    }
}

Time
TasQueueDisc::GetTransmissionDuration(Ptr<const QueueDiscItem> paketref){
  return m_linkBandwidth.CalculateBytesTxTime(paketref->GetSize());
}

void
TasQueueDisc::SetTransmitting(Ptr<const QueueDiscItem> item)
{
  Time sleepTime = GetTransmissionDuration(item);
  if(m_propagetiondelay.IsStrictlyPositive()){
      sleepTime += m_propagetiondelay;
  }
  NS_LOG_LOGIC("Transmitting until " << Simulator::Now() + sleepTime);
  m_transmitting = Simulator::Schedule(sleepTime,&TasQueueDisc::Run,this);
}

void
TasQueueDisc::InitializeParams ()
{
  NS_LOG_FUNCTION (this);
  ListExecute();
  //m_transmitting = false;
  switch (GetNQueueDiscClasses()){
    case (1):m_prioMap={0,0,0,0,0,0,0,0}; break;
    case (2):m_prioMap={0,0,0,0,1,1,1,1}; break;
    case (3):m_prioMap={0,0,0,0,1,1,2,2}; break;
    case (4):m_prioMap={0,0,1,1,2,2,3,3}; break;
    case (5):m_prioMap={0,0,1,1,2,2,3,4}; break;
    case (6):m_prioMap={1,0,2,2,3,3,4,5}; break;
    case (7):m_prioMap={1,0,2,3,4,4,5,6}; break;
    case (8):m_prioMap={1,0,2,3,4,5,6,7}; break;
  }
  m_StoppAllQueues = false;
}

void
TasQueueDisc::CancelRun(uint32_t queue){
  if(m_gateListConfig.at(queue).reRunEvent.IsPending())
    {
      int rerunby = -1;
      for(unsigned int i = 0; i < 8; i++)
        {
          if( queue != i && (m_gateListConfig.at(queue).reRunEvent == m_gateListConfig.at(i).reRunEvent))
            {
              if(rerunby < 0)
                {
                  rerunby = i;
                  m_gateListConfig.at(i).reRunEvent = Simulator::Schedule(m_gateListConfig.at(i).GetNextEvent()-GetDeviceTime(), &TasQueueDisc::Run, this);
                }
              else
                {
                  m_gateListConfig.at(i).reRunEvent = m_gateListConfig.at(rerunby).reRunEvent;
                }
            }
        }
      m_gateListConfig.at(queue).reRunEvent.Cancel();
      NS_LOG_LOGIC(this << " Scheduled Run for queue: " << queue << " at " << m_gateListConfig.at(queue).GetNextEvent().GetMilliSeconds() << "ms canceld");
    }
  return;
}

void
TasQueueDisc::SetTimeSource(Callback <Time> newCallback){
  m_getNow = newCallback;
  Ptr<QueueDisc> ptr;
  for( unsigned int i = GetNQueueDiscClasses(); i > 0; i--)
    {
      ptr= GetQueueDiscClass(i-1)->GetQueueDisc();
      ptr->SetAttribute("TimeSource", CallbackValue(m_getNow));
    }
}

Callback <Time>
TasQueueDisc::GetTimeSource() const {
  return m_getNow;
}

} // namespace ns3

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

#ifndef SRC_TRAFFIC_CONTROL_MODEL_TAS_QUEUE_DISC_H_
#define SRC_TRAFFIC_CONTROL_MODEL_TAS_QUEUE_DISC_H_

#include "ns3/queue-disc.h"
#include "ns3/data-rate.h"
#include "ns3/net-device-list-config.h"
#include "ns3/callback.h"
#include "ns3/csma-module.h"
#include "ns3/ptr.h"
#include <array>
#include <vector>
#include <math.h>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include "transmisson-gate-qdisc.h"

namespace ns3 {

class TasQueueDisc : public QueueDisc {
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
  TasQueueDisc ();

  virtual ~TasQueueDisc();
  bool SetMaxSize (QueueSize size);
  QueueSize GetMaxSize2 (void) const;
  void SetNetDeviceListConfig(NetDeviceListConfig pandingConfig);

  void UpdateNetDeviceListConfig();
  void StopAllQueues(Time duration = Time(-1));

  NetDeviceListConfig GetNetDeviceListConfig(void) const;

  void SetTimeSource(Callback<Time> newCallback);
  Callback<Time> GetTimeSource(void) const;

  // Reasons for dropping packets
  static constexpr const char* LIMIT_EXCEEDED_DROP = "Queue disc limit exceeded";  //!< Packet dropped due to queue disc limit exceeded
  static constexpr const char* PAKET_SIZE_EXCEEDED_DROP = "Paket size exceeded";  //!< Packet dropped due to live time exceeded

private:

  virtual void InitializeParams (void);
  virtual bool CheckConfig (void);
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);

  /**
   * holds all information about a childqueue
   */
  struct GateListEntry{
    EventId reRunEvent;
    Time reRunEventTimeStamp; //In Simulation Time
    unsigned int lastExecutIndex;
    GateListEntry()
    {
      m_gateState.state = true;
      m_gateState.interval = Time(-1);
      reRunEventTimeStamp = Time(-1);
      lastExecutIndex = 0;
    }
    void Copy(const GateListEntry newEntry)
    {
      m_gateState.state = newEntry.GetGateState();
      m_gateState.interval = newEntry.GetNextEvent();
      reRunEvent.Cancel();
      reRunEvent = newEntry.reRunEvent;
      reRunEventTimeStamp = newEntry.reRunEventTimeStamp;
      lastExecutIndex = newEntry.lastExecutIndex;
    }
    void SetGateState(bool state)
      {
        m_gateState.state = state;
      }
    bool GetGateState() const
      {
       return m_gateState.state;
      }
    void SetNextEvent(Time timePoint)
      {
      m_gateState.interval = timePoint;
      }
    Time GetNextEvent() const
      {
       return m_gateState.interval;
      }
  private:
    GateState m_gateState;
  };

  std::array<GateListEntry,8> m_gateListConfig; // an array with the current State of an gate
  std::array<unsigned char,8> m_prioMap; // the prio to child queue map

  /**
   * Updates a specific childqueue.
   *
   *\param index number of the childqueue
   *\param gateState the new gateState
   *\param nextEvent how long the gateState will be active befor the next change
   */
  void UpdateTransmisionGate(unsigned int index, bool gateState, Time nextEvent); // updates the m_gateListConfig for a specific gate

  /**
   * The List Execute function is used to determine the current configuration of the different transmission gates from the given NetdeviceListConfig.
   */
  void ListExecute();

  /**
   * reopens all child queues
   */
  void ResetGateStateList(); //opens all transmissiongates as if no schedule would be configuerd

  /**
   * sets the datarate for all childqueues
   *
   * \param newDataRate the new data rate.
   */
  void SetDatarate(DataRate newDataRate);

  /**
   * prevents dequeue for the transmission period.
   *
   * \param item ptr to the QueueDiscItem witch will be dequeued.
   */
  void SetTransmitting(Ptr<const QueueDiscItem> item); // wake funktion after paket schould be transmittet

  /**
   * \return the current time.
   *
   */
  virtual Time GetDeviceTime(); //Gets the current time TODO connect Node device Time to Queue Disc

  /**
   *  creates a run event for the queue.
   *
   * \param queue number of childqueue.
   */
  void ScheduleRun(uint32_t queue); //schedulet a run event for the queueue.

  /**
   *  cancels the rerun event for a queueue.
   *
   * \param queue number of childqueue.
   */
  void CancelRun(uint32_t queue);//

  /**
   * \return time that a paket needs to be transmittet
   *
   * \param paketref a ptr to an QueueDiscItem
   */
  virtual Time GetTransmissionDuration(Ptr<const QueueDiscItem> paketref); // calculates the transmissionduration for a paket


  /**
   * \return the next bigger entry of an std vector.
   * If no one found return 0.
   * If the std vector has no entrys -1.
   *
   * \param vector std vector
   * \param data the value
   */
  template <typename item>
  int64_t GetNextBiggerEntry(std::vector<item> vector, item data);

  bool m_StoppAllQueues;    //Close all queues flag
  bool m_trustQostag;       //Trust the Qostag of incoming Packets or sort new
  DataRate m_linkBandwidth; //Link Data Rate
  unsigned int m_Mtu;       //The max transmission unit in byte
  Time m_propagetiondelay;  //the propagation delay of an channel
  Time m_keepAliveTime;

  NetDeviceListConfig m_pandingConfig; //panding NetDeviceListConfig
  NetDeviceListConfig m_currentConfig; //active NetDeviceListConfig

  EventId m_stopAllQEvent;
  EventId m_updateNDLCEvent; //event to an update NetDeviceListConfig event

  Callback<Time> m_getNow; //Time source callback if not set uses simulation time
  EventId m_transmitting; // An Event that renns out after Transmitting is Finsihed

  Ptr<CsmaChannel> m_channel; //If used the Callback to the CsmaChannel
  /* variables stored by TAS Queue Disc */
};

};// namespace ns3
#endif /* SRC_TRAFFIC_CONTROL_MODEL_TAS_QUEUE_DISC_H_ */

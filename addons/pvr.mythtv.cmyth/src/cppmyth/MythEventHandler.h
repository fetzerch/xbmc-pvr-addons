#pragma once
/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "platform/util/StdString.h"

#include <ctime>

#include <boost/shared_ptr.hpp>

extern "C" {
#include <cmyth/cmyth.h>
};

class MythRecorder;
class MythSignal;
class MythFile;
class MythProgramInfo;

#include "MythProgramInfo.h"

template <class T> class MythPointer;

class MythEventHandler
{
public:
  MythEventHandler();
  MythEventHandler(const CStdString &server, unsigned short port);

  bool TryReconnect();

  void PreventLiveChainUpdate();
  void AllowLiveChainUpdate();

  MythSignal GetSignal();

  void SetRecorder(const MythRecorder &recorder);
  void SetRecordingListener(const CStdString &recordid, const MythFile &file);
  void EnablePlayback();
  void DisablePlayback();
  bool IsPlaybackActive() const;

  //Recordings change events
  enum RecordingChangeType
  {
    CHANGE_ALL = 0,
    CHANGE_ADD = 1,
    CHANGE_UPDATE,
    CHANGE_DELETE
  };

  class RecordingChangeEvent
  {
  public:
    RecordingChangeEvent(RecordingChangeType type, int chanid, char *recstartts)
      : m_type(type)
      , m_chanid(chanid)
      , m_recstartts(0)
    {
      if (recstartts) {
        MythTimestamp time(recstartts, false);
        m_recstartts = time.UnixTime();
      }
    }

    RecordingChangeEvent(RecordingChangeType type, const MythProgramInfo &prog)
      : m_type(type)
      , m_chanid(0)
      , m_recstartts(0)
      , m_prog(prog)
    {
    }

    RecordingChangeEvent(RecordingChangeType type)
      : m_type(type)
      , m_chanid(0)
      , m_recstartts(0)
    {
    }

    RecordingChangeType Type() const { return m_type; }
    int ChannelID() const { return m_chanid; };
    time_t RecStartTs() const { return m_recstartts; }
    MythProgramInfo Program() const { return m_prog; }

  private:
    RecordingChangeType m_type;
    // Event ADD or DELETE returns program key
    int m_chanid; // Channel ID
    time_t m_recstartts; // Recording start timeslot
    // Event UPDATE returns updated program info
    MythProgramInfo m_prog;
  };

  bool HasRecordingChangeEvent() const;
  RecordingChangeEvent NextRecordingChangeEvent();
  void ClearRecordingChangeEvents();

private:
  class MythEventHandlerPrivate; // Needs to be within MythEventHandler to inherit friend permissions
  boost::shared_ptr<MythEventHandlerPrivate> m_imp; // Private Implementation
  int m_retryCount;
};

#pragma once
/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "MythRecordingRule.h"
#include "MythProgramInfo.h"
#include "MythConnection.h"
#include "MythDatabase.h"
#include "MythEPGInfo.h"

#include <platform/threads/mutex.h>

#include <vector>
#include <list>
#include <map>

/*
 * Tree is rather flat: either main rule (which might have overriden rules) or an overriden rule (which has main rule)
 */
typedef std::vector<MythRecordingRule> OverrideRuleList;

// Schedule element is pair < index of schedule , program info of schedule >
typedef std::vector<std::pair<unsigned int, MythProgramInfo*> > ScheduleList;

class MythRecordingRuleNode {
public:
  friend class MythScheduleManager; // Most likely only this needs to modify the node

  MythRecordingRuleNode(const MythRecordingRule &rule);

  bool IsOverrideRule() const;
  MythRecordingRule GetRule() const;
  MythRecordingRule GetMainRule() const;

  bool HasOverrideRules() const;
  OverrideRuleList GetOverrideRules() const;

  bool IsInactiveRule() const;

private:
  // MythRecordingRule wraps just a smart pointer to cmyth_recording_rule, so we can copy them.
  MythRecordingRule m_rule;
  MythRecordingRule m_mainRule;
  std::vector<MythRecordingRule> m_overrideRules;
};

// The class should operate on a cache as much as possible and only ask the db / mythprotocol if needed

class MythScheduleManager {
public:
  MythScheduleManager();
  MythScheduleManager(MythConnection &con, MythDatabase &db);

  // Called by GetTimers
  ScheduleList GetUpcomingRecordings();

  enum MSM_ERROR {
    MSM_ERROR_FAILED = -1,
    MSM_ERROR_NOT_IMPLEMENTED = 0,
    MSM_ERROR_SUCCESS = 1
  };

  // Called by AddTimer
  MSM_ERROR ScheduleRecording(const MythRecordingRule &rule);

  // Called by DeleteTimer
  MSM_ERROR DeleteRecording(unsigned int index);

  // For update Timer, pvrclient-mythtv has to keep a map PVR_TIMER mapping and handle state switch
  // even we keep track of 'old' pending program / rules
  MSM_ERROR DisableRecording(unsigned int index);
  MSM_ERROR EnableRecording(unsigned int index);
  MSM_ERROR UpdateRecording(unsigned int index, MythRecordingRule &newrule);

  MythRecordingRuleNode *FindRuleById(unsigned int recordID) const;
  ScheduleList FindUpComingByRuleId(unsigned int recordID) const;
  MythProgramInfo *FindUpComingByIndex(unsigned int index) const;

  // The ScheduleManager should be able to register itself at the MythEventHandler for schedule updates
  // to refill its internal state (observer pattern)
  void Update();

  class VersionHelper {
  public:
    friend class MythScheduleManager;

    VersionHelper() {
    }
    virtual ~VersionHelper();
    virtual bool SameTimeslot(MythRecordingRule &first, MythRecordingRule &second) const = 0;
    virtual MythRecordingRule NewFromTemplate(MythEPGInfo &epgInfo) = 0;
    virtual MythRecordingRule NewSingleRecord(MythEPGInfo &epgInfo) = 0;
    virtual MythRecordingRule NewDailyRecord(MythEPGInfo &epgInfo) = 0;
    virtual MythRecordingRule NewWeeklyRecord(MythEPGInfo &epgInfo) = 0;
    virtual MythRecordingRule NewChannelRecord(CStdString searchTitle) = 0;
    virtual MythRecordingRule NewOneRecord(CStdString searchTitle) = 0;
  };

  MythRecordingRule NewFromTemplate(MythEPGInfo &epgInfo);
  MythRecordingRule NewSingleRecord(MythEPGInfo &epgInfo);
  MythRecordingRule NewDailyRecord(MythEPGInfo &epgInfo);
  MythRecordingRule NewWeeklyRecord(MythEPGInfo &epgInfo);
  MythRecordingRule NewChannelRecord(CStdString searchTitle);
  MythRecordingRule NewOneRecord(CStdString searchTitle);

private:
  mutable PLATFORM::CMutex m_lock;
  MythConnection m_con;
  MythDatabase m_db;

  int m_dbSchemaVersion;
  boost::shared_ptr<VersionHelper> m_versionHelper;
  void Setup();

  unsigned int MakeIndex(MythProgramInfo &recording) const;

  // The list of rule nodes
  typedef std::list<boost::shared_ptr<MythRecordingRuleNode> > NodeList;
  // To find a rule node by its key (recordID)
  typedef std::map<unsigned int, boost::shared_ptr<MythRecordingRuleNode> > NodeById;
  // Store and find up coming recordings by index
  typedef std::map<unsigned int, boost::shared_ptr<MythProgramInfo> > RecordingList;
  // To find all indexes of schedule by rule Id : pair < Rule Id , index of schedule >
  typedef std::multimap<unsigned int, unsigned int> RecordingIndexByRuleId;

  NodeList m_rules;
  NodeById m_rulesById;
  RecordingList m_recordings;
  RecordingIndexByRuleId m_recordingIndexByRuleId;
};

///////////////////////////////////////////////////////////////////////////////
////
//// About overrides of VersionHelper
////

inline MythScheduleManager::VersionHelper::~VersionHelper() {
}

// No helper

class MythScheduleHelperNoHelper : public MythScheduleManager::VersionHelper {
public:
  virtual bool SameTimeslot(MythRecordingRule &first, MythRecordingRule &second) const;
  virtual MythRecordingRule NewFromTemplate(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewSingleRecord(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewDailyRecord(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewWeeklyRecord(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewChannelRecord(CStdString searchTitle);
  virtual MythRecordingRule NewOneRecord(CStdString searchTitle);
};

// Base 0.24

class MythScheduleHelper1226 : public MythScheduleHelperNoHelper {
public:

  MythScheduleHelper1226(MythDatabase &db) : m_db(db) {
  }
  virtual bool SameTimeslot(MythRecordingRule &first, MythRecordingRule &second) const;
  virtual MythRecordingRule NewFromTemplate(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewSingleRecord(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewDailyRecord(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewWeeklyRecord(MythEPGInfo &epgInfo);
  virtual MythRecordingRule NewChannelRecord(CStdString searchTitle);
  virtual MythRecordingRule NewOneRecord(CStdString searchTitle);
protected:
  MythDatabase m_db;
};

// News in 0.25

class MythScheduleHelper1278 : public MythScheduleHelper1226 {
public:

  MythScheduleHelper1278(MythDatabase &db) : MythScheduleHelper1226(db) {
  }
  virtual bool SameTimeslot(MythRecordingRule &first, MythRecordingRule &second) const;
  virtual MythRecordingRule NewFromTemplate(MythEPGInfo &epgInfo);
};

// News in 0.26

class MythScheduleHelper1302 : public MythScheduleHelper1278 {
public:

  MythScheduleHelper1302(MythDatabase &db) : MythScheduleHelper1278(db) {
  }
  virtual MythRecordingRule NewFromTemplate(MythEPGInfo &epgInfo);
};

// News in 0.27

class MythScheduleHelper1309 : public MythScheduleHelper1302 {
public:

  MythScheduleHelper1309(MythDatabase &db) : MythScheduleHelper1302(db) {
  }
  //virtual bool SameTimeslot(MythRecordingRule &first, MythRecordingRule &second) const;
  //virtual MythRecordingRule NewSingleRecord() const;
  //virtual MythRecordingRule NewDailyRecord() const;
  //virtual MythRecordingRule NewWeeklyRecord() const;
};

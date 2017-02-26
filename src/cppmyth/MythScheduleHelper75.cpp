/*
 *      Copyright (C) 2005-2015 Team XBMC
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

///////////////////////////////////////////////////////////////////////////////
////
//// Version helper for backend version 75 (0.26)
////

#include "MythScheduleHelper75.h"
#include "../client.h"
#include "../tools.h"

#include <cstdio>
#include <cassert>

using namespace ADDON;

MythTimerTypeList MythScheduleHelper75::GetTimerTypes() const
{
  P8PLATFORM::CLockObject lock(m_lock);
  if (!m_timerTypeListInit)
  {
    m_timerTypeListInit = true;
    // Empty list
    MythTimerType::AttributeList emptyList;
    // Simple expiration list for no repeating rule
    MythTimerType::AttributeList autoExpireList;
    int autoExpire0 = GetRuleExpirationId(RuleExpiration(false, 0, false));
    int autoExpire1 = GetRuleExpirationId(RuleExpiration(true, 0, false));
    autoExpireList.push_back(std::make_pair(autoExpire0, XBMC->GetLocalizedString(30506))); // Recordings never expire
    autoExpireList.push_back(std::make_pair(autoExpire1, XBMC->GetLocalizedString(30507))); // Allow recordings to expire

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_MANUAL_SEARCH,
            PVR_TIMER_TYPE_IS_MANUAL |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_TIME |
            PVR_TIMER_TYPE_SUPPORTS_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30460), // Manual
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            Myth::DM_CheckNone,
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_THIS_SHOWING,
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_TIME |
            PVR_TIMER_TYPE_SUPPORTS_END_TIME |
            PVR_TIMER_TYPE_REQUIRES_EPG_TAG_ON_CREATE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30465), // Record this
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            Myth::DM_CheckNone,
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_RECORD_ONE,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30461), // Record one
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_RECORD_WEEKLY,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_TIME |
            PVR_TIMER_TYPE_SUPPORTS_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_FIRST_DAY |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30462), // Record weekly
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            Myth::DM_CheckNone,
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_RECORD_DAILY,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_TIME |
            PVR_TIMER_TYPE_SUPPORTS_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30463), // Record daily
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_RECORD_ALL,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30464), // Record all
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_RECORD_SERIES,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_REQUIRES_EPG_SERIES_ON_CREATE |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30466), // Record series
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_SEARCH_TEXT,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH |
            PVR_TIMER_TYPE_SUPPORTS_FULLTEXT_EPG_MATCH |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30467), // Search keyword
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_SEARCH_PEOPLE,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30468), // Search people
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    ///////////////////////////////////////////////////////////////////////////
    //// KEEP LAST
    ///////////////////////////////////////////////////////////////////////////
    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_UNHANDLED,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30451), // Unhandled
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefaultId(),
            GetRuleExpirationNameList(),
            GetRuleExpirationDefaultId(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_RULE_INACTIVE,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30469), // Rule Disabled
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            0, // n&v
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_UPCOMING,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30452), // Upcoming
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            0, // n&v
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_UPCOMING_ALTERNATE,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30457), // Alternative
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            0, // n&v
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_UPCOMING_RECORDED,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30458), // Currently recorded
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            0, // n&v
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_UPCOMING_EXPIRED,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30459), // Expired recording
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            0, // n&v
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_OVERRIDE,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30453), // Overriden
            GetRulePriorityList(),
            GetRulePriorityDefaultId(),
            emptyList,
            0, // n&v
            autoExpireList,
            autoExpire1,
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefaultId())));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_DONT_RECORD,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_TIME |
            PVR_TIMER_TYPE_SUPPORTS_END_TIME,
            XBMC->GetLocalizedString(30454), // Don't record
            emptyList,
            0, // n&v
            emptyList,
            0, // n&v
            emptyList,
            0, // n&v
            emptyList,
            RECGROUP_DFLT_ID)));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_UPCOMING_MANUAL,
            PVR_TIMER_TYPE_IS_READONLY,
            XBMC->GetLocalizedString(30455), // Upcoming manual
            emptyList,
            0, // n&v
            emptyList,
            0, // n&v
            emptyList,
            0, // n&v
            emptyList,
            RECGROUP_DFLT_ID)));

    m_timerTypeList.push_back(MythTimerTypePtr(new MythTimerType(TIMER_TYPE_ZOMBIE,
            PVR_TIMER_TYPE_IS_READONLY,
            XBMC->GetLocalizedString(30456), // Zombie
            emptyList,
            0, // n&v
            emptyList,
            0, // n&v
            emptyList,
            0, // n&v
            emptyList,
            RECGROUP_DFLT_ID)));

  }
  return m_timerTypeList;
}

bool MythScheduleHelper75::SameTimeslot(const MythRecordingRule& first, const MythRecordingRule& second) const
{
  time_t first_st = first.StartTime();
  time_t second_st = second.StartTime();

  switch (first.Type())
  {
  case Myth::RT_NotRecording:
  case Myth::RT_SingleRecord:
  case Myth::RT_OverrideRecord:
  case Myth::RT_DontRecord:
    return
    second_st == first_st &&
            second.EndTime() == first.EndTime() &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_OneRecord: // FindOneRecord
    return
    second.Title() == first.Title() &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_DailyRecord: // TimeslotRecord
    return
    second.Title() == first.Title() &&
            daytime(&first_st) == daytime(&second_st) &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_WeeklyRecord: // WeekslotRecord
    return
    second.Title() == first.Title() &&
            daytime(&first_st) == daytime(&second_st) &&
            weekday(&first_st) == weekday(&second_st) &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_FindDailyRecord:
    return
    second.Title() == first.Title() &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_FindWeeklyRecord:
    return
    second.Title() == first.Title() &&
            weekday(&first_st) == weekday(&second_st) &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_ChannelRecord:
    return
    second.Title() == first.Title() &&
            second.ChannelID() == first.ChannelID() &&
            second.Filter() == first.Filter();

  case Myth::RT_AllRecord:
    return
    second.Title() == first.Title() &&
            second.Filter() == first.Filter();

  default:
    break;
  }
  return false;
}

bool MythScheduleHelper75::FillTimerEntryWithRule(MythTimerEntry& entry, const MythRecordingRuleNode& node) const
{
  // Assign timer type regarding rule attributes. The match SHOULD be opposite to
  // that which is applied in function 'RuleFromMythTimer'

  MythRecordingRule rule = node.GetRule();
  if (g_bExtraDebug)
    XBMC->Log(LOG_DEBUG, "75::%s: Rule %u", __FUNCTION__, rule.RecordID());

  switch (rule.Type())
  {
    case Myth::RT_SingleRecord:
      {
        // Fill recording status from its upcoming.
        MythScheduleList recordings = m_manager->FindUpComingByRuleId(rule.RecordID());
        MythScheduleList::const_reverse_iterator it = recordings.rbegin();
        if (it != recordings.rend())
          entry.recordingStatus = it->second->Status();
        else
          return false; // Don't transfer single without upcoming
      }
      if (rule.SearchType() == Myth::ST_ManualSearch)
        entry.timerType = TIMER_TYPE_MANUAL_SEARCH;
      else
        entry.timerType = TIMER_TYPE_THIS_SHOWING;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;

    case Myth::RT_OneRecord:
      entry.timerType = TIMER_TYPE_RECORD_ONE;
      break;

    case Myth::RT_FindDailyRecord:
      entry.timerType = TIMER_TYPE_RECORD_DAILY;
      break;

    case Myth::RT_DailyRecord:
      entry.timerType = TIMER_TYPE_RECORD_DAILY;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;

    case Myth::RT_FindWeeklyRecord:
      entry.timerType = TIMER_TYPE_RECORD_WEEKLY;
      break;

    case Myth::RT_WeeklyRecord:
      entry.timerType = TIMER_TYPE_RECORD_WEEKLY;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;

    case Myth::RT_ChannelRecord:
      if ((rule.Filter() & Myth::FM_ThisSeries))
        entry.timerType = TIMER_TYPE_RECORD_SERIES;
      else
        entry.timerType = TIMER_TYPE_RECORD_ALL;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;

    case Myth::RT_AllRecord:
      entry.timerType = TIMER_TYPE_RECORD_ALL;
      break;

    case Myth::RT_OverrideRecord:
      entry.timerType = TIMER_TYPE_OVERRIDE;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;

    case Myth::RT_DontRecord:
      entry.timerType = TIMER_TYPE_DONT_RECORD;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;

    default:
      entry.timerType = TIMER_TYPE_UNHANDLED;
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      break;
  }

  switch (rule.SearchType())
  {
    case Myth::ST_TitleSearch:
      entry.epgSearch = rule.Description();
      entry.timerType = TIMER_TYPE_SEARCH_TEXT;
      break;
    case Myth::ST_KeywordSearch:
      entry.epgSearch = rule.Description();
      entry.timerType = TIMER_TYPE_SEARCH_TEXT;
      entry.isFullTextEpgSearch = true;
      break;
    case Myth::ST_PeopleSearch:
      entry.epgSearch = rule.Description();
      entry.timerType = TIMER_TYPE_SEARCH_PEOPLE;
      break;
    case Myth::ST_PowerSearch:
      entry.epgSearch = rule.Description();
      entry.timerType = TIMER_TYPE_UNHANDLED;
      break;
    case Myth::ST_NoSearch:
      break;
    case Myth::ST_ManualSearch: // Manual
      entry.chanid = rule.ChannelID();
      entry.callsign = rule.Callsign();
      entry.startTime = rule.StartTime();
      entry.endTime = rule.EndTime();
      break;
    default:
      break;
  }

  switch (entry.timerType)
  {
    case TIMER_TYPE_RECORD_ONE:
    case TIMER_TYPE_RECORD_WEEKLY:
    case TIMER_TYPE_RECORD_DAILY:
    case TIMER_TYPE_RECORD_ALL:
    case TIMER_TYPE_RECORD_SERIES:
    case TIMER_TYPE_SEARCH_TEXT:
    case TIMER_TYPE_SEARCH_PEOPLE:
    case TIMER_TYPE_UNHANDLED:
      // For all rules without a time, set time based on next/last recording
      if (difftime(rule.NextRecording(), 0) > 0)
      {
        // fill timeslot starting at next recording
        entry.startTime = rule.NextRecording(); // it includes offset correction
        // WARNING: if next recording has been overriden then offset could be different
        timeadd(&entry.startTime, INTERVAL_MINUTE * rule.StartOffset()); // remove start offset
        if (!rule.Inactive())
          entry.recordingStatus = Myth::RS_WILL_RECORD;
      }
      else if (difftime(rule.LastRecorded(), 0) > 0)
      {
        // fill timeslot starting at last recorded
        entry.startTime = rule.LastRecorded(); // it includes offset correction
        // WARNING: if last recorded has been overriden then offset could be different
        timeadd(&entry.startTime, INTERVAL_MINUTE * rule.StartOffset()); // remove start offset
        if (!rule.Inactive())
          entry.recordingStatus = Myth::RS_RECORDED;
      }
      else
      {
        // Fall back to the timeslot used when the rule was created
        entry.startTime = rule.StartTime();
        entry.recordingStatus = Myth::RS_UNKNOWN;
      }

      if (entry.timerType == TIMER_TYPE_RECORD_WEEKLY || entry.timerType == TIMER_TYPE_RECORD_DAILY)
      {
        // Something which needs a sensible end time, add rule duration or 2 seconds if rule has no duration
        entry.endTime = entry.startTime;
        if (rule.EndTime() - rule.StartTime() > 0)
          timeadd(&entry.endTime, rule.EndTime() - rule.StartTime());
        else
          timeadd(&entry.endTime, 2);
      }
      else
        entry.endTime = 0; // any time

      // For all repeating set summary status
      if (node.HasConflict())
        entry.recordingStatus = Myth::RS_CONFLICT;
      else if (node.IsRecording())
        entry.recordingStatus = Myth::RS_RECORDING;
      //
      break;
    default:
      entry.startTime = rule.StartTime();
      entry.endTime = rule.EndTime();
  }

  // fill others
  entry.epgInfo = MythEPGInfo(rule.ChannelID(), rule.StartTime(), rule.EndTime());
  entry.title = rule.Title();
  entry.category = rule.Category();
  entry.startOffset = rule.StartOffset();
  entry.endOffset = rule.EndOffset();
  entry.dupMethod = rule.DuplicateControlMethod();
  entry.priority = rule.Priority();
  entry.expiration = GetRuleExpirationId(RuleExpiration(rule.AutoExpire(), rule.MaxEpisodes(), rule.NewExpiresOldRecord()));
  entry.isInactive = rule.Inactive();
  entry.firstShowing = (rule.Filter() & Myth::FM_FirstShowing ? true : false);
  entry.recordingGroup = GetRuleRecordingGroupId(rule.RecordingGroup());
  entry.entryIndex = MythScheduleManager::MakeIndex(rule); // rule index
  if (node.IsOverrideRule())
    entry.parentIndex = MythScheduleManager::MakeIndex(node.GetMainRule());
  else
    entry.parentIndex = 0;
  return true;
}

bool MythScheduleHelper75::FillTimerEntryWithUpcoming(MythTimerEntry& entry, const MythProgramInfo& recording) const
{
  //Only include timers which have an inactive status if the user has requested it (flag m_showNotRecording)
  switch (recording.Status())
  {
    //Upcoming recordings which are disabled due to being lower priority duplicates or already recorded
    case Myth::RS_EARLIER_RECORDING:  //will record earlier
    case Myth::RS_LATER_SHOWING:      //will record later
    case Myth::RS_CURRENT_RECORDING:  //Already in the current library
    case Myth::RS_PREVIOUS_RECORDING: //Previoulsy recorded but no longer in the library
      if (!m_manager->ShowNotRecording())
      {
        if (g_bExtraDebug)
          XBMC->Log(LOG_DEBUG, "75::%s: Skipping %s:%s on %s because status %d", __FUNCTION__,
                  recording.Title().c_str(), recording.Subtitle().c_str(), recording.ChannelName().c_str(),
                  recording.Status());
        return false;
      }
    default:
      break;
  }

  MythRecordingRuleNodePtr node = m_manager->FindRuleById(recording.RecordID());
  if (node)
  {
    MythRecordingRule rule = node->GetRule();
    // Relate the main rule as parent
    entry.parentIndex = MythScheduleManager::MakeIndex(node->GetMainRule());
    switch (rule.Type())
    {
      case Myth::RT_SingleRecord:
        return false; // Discard upcoming. We show only main rule.

      // API Dvr/1.9: Type OverrideRecord is use to qualify all modifers.
      // So I use the recording status to choose the right type
      case Myth::RT_DontRecord:
      case Myth::RT_OverrideRecord:
        switch (recording.Status())
        {
          case Myth::RS_DONT_RECORD:
          case Myth::RS_NEVER_RECORD:
            entry.recordingStatus = recording.Status();
            entry.timerType = TIMER_TYPE_DONT_RECORD;
            entry.isInactive = rule.Inactive();
            break;
          default:
            entry.recordingStatus = recording.Status();
            entry.timerType = TIMER_TYPE_OVERRIDE;
            entry.isInactive = rule.Inactive();
        }
        break;
      default:
        entry.recordingStatus = recording.Status();
        if (node->GetMainRule().SearchType() == Myth::ST_ManualSearch)
          entry.timerType = TIMER_TYPE_UPCOMING_MANUAL;
        else
          {
            switch (recording.Status())
            {
              case Myth::RS_EARLIER_RECORDING:  //will record earlier
              case Myth::RS_LATER_SHOWING:      //will record later
                entry.timerType = TIMER_TYPE_UPCOMING_ALTERNATE;
                break;
              case Myth::RS_CURRENT_RECORDING:  //Already in the current library
                entry.timerType = TIMER_TYPE_UPCOMING_RECORDED;
                break;
              case Myth::RS_PREVIOUS_RECORDING: //Previoulsy recorded but no longer in the library
                entry.timerType = TIMER_TYPE_UPCOMING_EXPIRED;
                break;
              case Myth::RS_INACTIVE:
                entry.timerType = TIMER_TYPE_RULE_INACTIVE;
                break;
              default:
                entry.timerType = TIMER_TYPE_UPCOMING;
                break;
            }
          }
    }
    entry.startOffset = rule.StartOffset();
    entry.endOffset = rule.EndOffset();
    entry.priority = rule.Priority();
    entry.expiration = GetRuleExpirationId(RuleExpiration(rule.AutoExpire(), 0, false));
  }
  else
    entry.timerType = TIMER_TYPE_ZOMBIE;

  switch (entry.timerType)
  {
    case TIMER_TYPE_UPCOMING:
    case TIMER_TYPE_RULE_INACTIVE:
    case TIMER_TYPE_UPCOMING_ALTERNATE:
    case TIMER_TYPE_UPCOMING_RECORDED:
    case TIMER_TYPE_UPCOMING_EXPIRED:
    case TIMER_TYPE_OVERRIDE:
    case TIMER_TYPE_UPCOMING_MANUAL:
      entry.epgCheck = true;
      break;
    default:
      entry.epgCheck = false;
  }

  entry.epgInfo = MythEPGInfo(recording.ChannelID(), recording.StartTime(), recording.EndTime());
  entry.description = "";
  entry.chanid = recording.ChannelID();
  entry.callsign = recording.Callsign();
  entry.startTime = recording.StartTime();
  entry.endTime = recording.EndTime();
  entry.title.assign(recording.Title());
  if (!recording.Subtitle().empty())
    entry.title.append(" (").append(recording.Subtitle()).append(")");
  if (recording.Season() && recording.Episode())
    entry.title.append(" - ").append(Myth::IntToString(recording.Season())).append(".").append(Myth::IntToString(recording.Episode()));
  else if (recording.Episode())
    entry.title.append(" - S").append(Myth::IntToString(recording.Episode()));
  entry.recordingGroup = GetRuleRecordingGroupId(recording.RecordingGroup());
  entry.entryIndex = MythScheduleManager::MakeIndex(recording); // upcoming index

  return true;
}

MythRecordingRule MythScheduleHelper75::NewRuleFromTemplate(const MythTimerEntry& entry)
{
  MythEPGInfo epgInfo = entry.epgInfo;
  // Load rule template from selected provider
  MythRecordingRule rule;
  switch (g_iRecTemplateType)
  {
  case 1: // Template provider is 'MythTV', then load the template from backend.
    if (!epgInfo.IsNull())
    {
      MythRecordingRuleList templates = m_manager->GetTemplateRules();
      MythRecordingRuleList::const_iterator tplIt = templates.end();
      for (MythRecordingRuleList::const_iterator it = templates.begin(); it != templates.end(); ++it)
      {
        if (it->Category() == epgInfo.Category())
        {
          XBMC->Log(LOG_INFO, "75::%s: Template found for category '%s'", __FUNCTION__, epgInfo.Category().c_str());
          tplIt = it;
          break;
        }
        if (it->Category() == epgInfo.CategoryType())
        {
          XBMC->Log(LOG_INFO, "75::%s: Template found for category type '%s'", __FUNCTION__, epgInfo.CategoryType().c_str());
          tplIt = it;
          continue;
        }
        if (it->Category() == "Default" && tplIt == templates.end())
          tplIt = it;
      }
      if (tplIt != templates.end())
      {
        XBMC->Log(LOG_INFO, "75::%s: Creating rule based on backend template %u '%s'", __FUNCTION__, (unsigned)tplIt->RecordID(), tplIt->Title().c_str());
        rule.SetPriority(tplIt->Priority());
        rule.SetStartOffset(tplIt->StartOffset());
        rule.SetEndOffset(tplIt->EndOffset());
        rule.SetSearchType(tplIt->SearchType());
        rule.SetDuplicateControlMethod(tplIt->DuplicateControlMethod());
        rule.SetCheckDuplicatesInType(tplIt->CheckDuplicatesInType());
        rule.SetRecordingGroup(tplIt->RecordingGroup());
        rule.SetRecordingProfile(tplIt->RecordingProfile());
        rule.SetStorageGroup(tplIt->StorageGroup());
        rule.SetPlaybackGroup(tplIt->PlaybackGroup());
        rule.SetUserJob(1, tplIt->UserJob(1));
        rule.SetUserJob(2, tplIt->UserJob(2));
        rule.SetUserJob(3, tplIt->UserJob(3));
        rule.SetUserJob(4, tplIt->UserJob(4));
        rule.SetAutoTranscode(tplIt->AutoTranscode());
        rule.SetAutoCommFlag(tplIt->AutoCommFlag());
        rule.SetAutoExpire(tplIt->AutoExpire());
        rule.SetAutoMetadata(tplIt->AutoMetadata());
        rule.SetMaxEpisodes(tplIt->MaxEpisodes());
        rule.SetNewExpiresOldRecord(tplIt->NewExpiresOldRecord());
        rule.SetFilter(tplIt->Filter());
        rule.SetCategory(tplIt->Category());
      }
      else
        XBMC->Log(LOG_INFO, "75::%s: No template found. Using blank rule.", __FUNCTION__);
    }
    break;
  case 0: // Template provider is 'Internal', then set rule with settings
    XBMC->Log(LOG_INFO, "75::%s: Using pvr.mythtv client internal template.", __FUNCTION__);
    rule.SetAutoCommFlag(g_bRecAutoCommFlag);
    rule.SetAutoMetadata(g_bRecAutoMetadata);
    rule.SetAutoTranscode(g_bRecAutoTranscode);
    rule.SetUserJob(1, g_bRecAutoRunJob1);
    rule.SetUserJob(2, g_bRecAutoRunJob2);
    rule.SetUserJob(3, g_bRecAutoRunJob3);
    rule.SetUserJob(4, g_bRecAutoRunJob4);
    rule.SetAutoExpire(g_bRecAutoExpire);
    rule.SetTranscoder(g_iRecTranscoder);
    // set defaults
    rule.SetPriority(GetRulePriorityDefaultId());
    rule.SetDuplicateControlMethod(static_cast<Myth::DM_t>(GetRuleDupMethodDefaultId()));
    rule.SetCheckDuplicatesInType(Myth::DI_InAll);
    rule.SetRecordingGroup(GetRuleRecordingGroupName(GetRuleRecordingGroupDefaultId()));
  }

  // Category override
  if (!epgInfo.IsNull())
  {
    Myth::SettingPtr overTimeCategory = m_control->GetSetting("OverTimeCategory", false);
    if (overTimeCategory && (overTimeCategory->value == epgInfo.Category() || overTimeCategory->value == epgInfo.CategoryType()))
    {
      Myth::SettingPtr categoryOverTime = m_control->GetSetting("CategoryOverTime", false);
      if (categoryOverTime && !categoryOverTime->value.empty())
      {
        int offset = atoi(categoryOverTime->value.c_str());
        XBMC->Log(LOG_DEBUG, "75::%s: Overriding end offset for category %s: +%d", __FUNCTION__, overTimeCategory->value.c_str(), offset);
        rule.SetEndOffset(offset);
      }
    }
  }

  //set sane defaults based on the MythTimerEntry entry where available
  //All rules must have a valid channelID and callsign or the backend will reject
  if (entry.chanid != 0 && !entry.callsign.empty())
  {
    rule.SetChannelID(entry.chanid);
    rule.SetCallsign(entry.callsign);
  }
  else if (!entry.epgInfo.IsNull())
  {
    rule.SetChannelID(entry.epgInfo.ChannelID());
    rule.SetCallsign(entry.epgInfo.Callsign());
  }
  else
  {
    rule.SetChannelID(100);
    rule.SetCallsign("Dummy");
  }
  //All rules must have an end time after the start time or the backend will reject
  if (entry.startTime > 0)
  {
    rule.SetStartTime(entry.startTime);
    rule.SetEndTime(entry.endTime);
  }
  else if (!entry.epgInfo.IsNull())
  {
    rule.SetStartTime(entry.epgInfo.StartTime());
    rule.SetEndTime(entry.epgInfo.EndTime());
  }
  else
  {
    rule.SetStartTime(time(0));
  }
  if (rule.EndTime() < rule.StartTime())
    rule.SetEndTime(rule.StartTime()+2);

  return rule;
}

MythRecordingRule MythScheduleHelper75::RuleFromMythTimer(const MythTimerEntry& entry)
{
  // Create a recording rule regarding timer attributes. The match SHOULD be opposite to
  // that which is applied in function 'FillTimerEntry'

  MythRecordingRule rule;

  if (entry.entryIndex == PVR_TIMER_NO_CLIENT_INDEX)
  {
    //This is the first time we've seen this entry - it is probably a new one
    XBMC->Log(LOG_DEBUG, "75::%s: Creating new template for entry %s", __FUNCTION__, entry.title.c_str());
    //Create an entry from the template
    rule = NewRuleFromTemplate(entry);
  }
  else
  {
    switch (entry.timerType)
    {
    case TIMER_TYPE_DONT_RECORD:
    case TIMER_TYPE_OVERRIDE:
    case TIMER_TYPE_UPCOMING:
    case TIMER_TYPE_UPCOMING_MANUAL:
    case TIMER_TYPE_ZOMBIE:
      //Not a 'rule' type
      XBMC->Log(LOG_DEBUG, "75::%s: Minimal template used for 'upcoming' entry %s", __FUNCTION__, entry.title.c_str());
      rule = NewRuleFromTemplate(entry);
      break;

    default:
      MythRecordingRuleNodePtr node = m_manager->FindRuleByIndex(entry.entryIndex);
      if (node)
      {
        rule = node->GetRule().DuplicateRecordingRule();
        XBMC->Log(LOG_DEBUG, "75::%s: Duplicated rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
        if (!FixRule(rule))
          XBMC->Log(LOG_DEBUG, "75::%s: This may cause problems later.", __FUNCTION__);
      }
      else
      {
        XBMC->Log(LOG_DEBUG, "75::%s: Unable to find existing rule with index %u (%s) for amendment",
                  __FUNCTION__, entry.entryIndex, entry.title.c_str());
        rule.SetType(Myth::RT_UNKNOWN);
        XBMC->Log(LOG_ERROR, "75::%s: Invalid timer %u: TYPE=%d CHANID=%u SIGN=%s ST=%u ET=%u", __FUNCTION__, entry.entryIndex,
                  entry.timerType, entry.chanid, entry.callsign.c_str(), (unsigned)entry.startTime, (unsigned)entry.endTime);
        return rule;
      }
      break;
    }
  }
  //Apply settings adjustable for all types
  rule.SetCategory(entry.category);
  rule.SetStartOffset(entry.startOffset);
  rule.SetEndOffset(entry.endOffset);
  rule.SetDuplicateControlMethod(entry.dupMethod);
  rule.SetPriority(entry.priority);
  RuleExpiration exr = GetRuleExpiration(entry.expiration);
  rule.SetAutoExpire(exr.autoExpire);
  rule.SetMaxEpisodes(exr.maxEpisodes);
  rule.SetNewExpiresOldRecord(exr.maxNewest);
  rule.SetRecordingGroup(GetRuleRecordingGroupName(entry.recordingGroup));
  rule.SetInactive(entry.isInactive);

  switch (entry.timerType)
  {
    case TIMER_TYPE_MANUAL_SEARCH:
    {
      if (entry.HasChannel() && entry.HasTimeSlot())
      {
        if (entry.entryIndex == PVR_TIMER_NO_CLIENT_INDEX)
        {
          rule.SetType(Myth::RT_SingleRecord);
          rule.SetSearchType(Myth::ST_ManualSearch);
        }
        rule.SetTitle(entry.title);
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
        rule.SetStartTime(entry.startTime);
        rule.SetEndTime(entry.endTime);
        XBMC->Log(LOG_DEBUG, "75::%s: Manual rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
        return rule;
      }
      break;
    }

    case TIMER_TYPE_THIS_SHOWING:
    {
      if (!entry.epgInfo.IsNull() && entry.entryIndex == PVR_TIMER_NO_CLIENT_INDEX)
      {
        rule.SetType(Myth::RT_SingleRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.epgInfo.Description());
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetCallsign(entry.epgInfo.Callsign());
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        XBMC->Log(LOG_DEBUG, "75::%s: This showing rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
        return rule;
      }
      else
      {
        XBMC->Log(LOG_DEBUG, "75::%s: This showing rule %u, ChanID %u, Filter %u, Type %u, Search %u edited",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
        return rule;
      }
      break;
    }

    case TIMER_TYPE_RECORD_ONE:
    {
      if (entry.entryIndex == PVR_TIMER_NO_CLIENT_INDEX)
      {
        rule.SetType(Myth::RT_OneRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetTitle(entry.title);
      }
      if (entry.HasChannel())
      {
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
      }
      XBMC->Log(LOG_DEBUG, "75::%s: Record one rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
      return rule;
      break;
    }

    case TIMER_TYPE_RECORD_WEEKLY:
    {
      rule.SetTitle(entry.title);
      if (entry.HasChannel())
      {
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
      }
      if (entry.HasTimeSlot())
      {
        rule.SetStartTime(entry.startTime);
        rule.SetEndTime(entry.endTime);
      }
      rule.SetType(Myth::RT_WeeklyRecord);
      rule.SetSearchType(Myth::ST_NoSearch);
      XBMC->Log(LOG_DEBUG, "75::%s: Record weekly rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
      return rule;
      break;
    }

    case TIMER_TYPE_RECORD_DAILY:
    {
      rule.SetTitle(entry.title);
      if (entry.HasChannel())
      {
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
      }
      if (entry.HasTimeSlot())
      {
        rule.SetStartTime(entry.startTime);
        rule.SetEndTime(entry.endTime);
      }
      rule.SetType(Myth::RT_DailyRecord);
      rule.SetSearchType(Myth::ST_NoSearch);
      XBMC->Log(LOG_DEBUG, "75::%s: Record daily rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
      return rule;
      break;
    }

    case TIMER_TYPE_RECORD_ALL:
    {
      rule.SetTitle(entry.title);
      rule.SetSearchType(Myth::ST_NoSearch);
      if (entry.HasChannel())
      {
        rule.SetType(Myth::RT_ChannelRecord);
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
      }
      else
        rule.SetType(Myth::RT_AllRecord);

      XBMC->Log(LOG_DEBUG, "75::%s: Record all rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
      return rule;
      break;
    }

    case TIMER_TYPE_RECORD_SERIES:
    {
      if (entry.HasChannel())
      {
        rule.SetType(Myth::RT_ChannelRecord);
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
      }
      else
        rule.SetType(Myth::RT_AllRecord);

      if (entry.entryIndex == PVR_TIMER_NO_CLIENT_INDEX)
      {
        if (!entry.epgInfo.IsNull())
        {
          rule.SetSearchType(Myth::ST_NoSearch);
          rule.SetTitle(entry.epgInfo.Title());
          rule.SetChannelID(entry.epgInfo.ChannelID());
          rule.SetCallsign(entry.epgInfo.Callsign());
          rule.SetStartTime(entry.epgInfo.StartTime());
          rule.SetEndTime(entry.epgInfo.EndTime());
          rule.SetCategory(entry.epgInfo.Category());
          rule.SetProgramID(entry.epgInfo.ProgramID());
          rule.SetSeriesID(entry.epgInfo.SeriesID());
          XBMC->Log(LOG_DEBUG, "75::%s: Record series rule %u, ChanID %u, Filter %u, Type %u, Search %u",
                    __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
          return rule;
        }
      }
      else
      {
        //All other settings are common
        XBMC->Log(LOG_DEBUG, "75::%s: Record series rule %u, ChanID %u, Filter %u, Type %u, Search %u edited",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
        return rule;
      }
      break;
    }

    case TIMER_TYPE_SEARCH_TEXT:
    {
      if (!entry.epgSearch.empty())
      {
        if (entry.HasChannel())
        {
          rule.SetType(Myth::RT_ChannelRecord);
          rule.SetChannelID(entry.chanid);
          rule.SetCallsign(entry.callsign);
        }
        else
          rule.SetType(Myth::RT_AllRecord);
        if (entry.isFullTextEpgSearch)
          rule.SetSearchType(Myth::ST_KeywordSearch); // Search keyword
        else
          rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        rule.SetTitle(entry.title);
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle("");
        rule.SetDescription(entry.epgSearch);
        XBMC->Log(LOG_DEBUG, "75::%s: Text search rule %u, ChanID %u, Filter %u, Type %u, Search %u edited",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());

        return rule;
      }
      break;
    }

    case TIMER_TYPE_SEARCH_PEOPLE:
    {
      if (!entry.epgSearch.empty())
      {
        if (entry.HasChannel())
        {
          rule.SetType(Myth::RT_ChannelRecord);
          rule.SetChannelID(entry.chanid);
          rule.SetCallsign(entry.callsign);
        }
        else
          rule.SetType(Myth::RT_AllRecord);
        rule.SetSearchType(Myth::ST_PeopleSearch); // Search People
        rule.SetTitle(entry.title);
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle("");
        rule.SetDescription(entry.epgSearch);
        XBMC->Log(LOG_DEBUG, "75::%s: People search rule %u, ChanID %u, Filter %u, Type %u, Search %u edited",
                  __FUNCTION__, rule.RecordID(), rule.ChannelID(), rule.Filter(), rule.Type(), rule.SearchType());
        return rule;
      }
      break;
    }

    case TIMER_TYPE_UNHANDLED:
      XBMC->Log(LOG_DEBUG, "75::%s: Unhandled rule: %u passed", __FUNCTION__, rule.RecordID());
      return rule;

    case TIMER_TYPE_DONT_RECORD:
      rule.SetType(Myth::RT_DontRecord);
      rule.SetChannelID(entry.chanid);
      rule.SetCallsign(entry.callsign);
      rule.SetStartTime(entry.startTime);
      rule.SetEndTime(entry.endTime);
      rule.SetTitle(entry.title);
      rule.SetDescription(entry.description);
      return rule;
    case TIMER_TYPE_OVERRIDE:
      rule.SetType(Myth::RT_OverrideRecord);
      rule.SetChannelID(entry.chanid);
      rule.SetCallsign(entry.callsign);
      rule.SetStartTime(entry.startTime);
      rule.SetEndTime(entry.endTime);
      rule.SetTitle(entry.title);
      rule.SetDescription(entry.description);
      return rule;
    case TIMER_TYPE_UPCOMING:
    case TIMER_TYPE_RULE_INACTIVE:
    case TIMER_TYPE_UPCOMING_ALTERNATE:
    case TIMER_TYPE_UPCOMING_RECORDED:
    case TIMER_TYPE_UPCOMING_EXPIRED:
    case TIMER_TYPE_UPCOMING_MANUAL:
    case TIMER_TYPE_ZOMBIE:
      rule.SetType(Myth::RT_SingleRecord);
      rule.SetChannelID(entry.chanid);
      rule.SetCallsign(entry.callsign);
      rule.SetStartTime(entry.startTime);
      rule.SetEndTime(entry.endTime);
      rule.SetTitle(entry.title);
      rule.SetDescription(entry.description);
      return rule;

    default:
      break;
  }
  rule.SetType(Myth::RT_UNKNOWN);
  XBMC->Log(LOG_ERROR, "75::%s: Invalid timer %u: TYPE=%d CHANID=%u SIGN=%s ST=%u ET=%u", __FUNCTION__, entry.entryIndex,
          entry.timerType, entry.chanid, entry.callsign.c_str(), (unsigned)entry.startTime, (unsigned)entry.endTime);
  return rule;
}

MythRecordingRule MythScheduleHelper75::MakeDontRecord(const MythRecordingRule& rule, const MythProgramInfo& recording)
{
  MythRecordingRule modifier = rule.DuplicateRecordingRule();
  // Do the same as backend even we know the modifier will be rejected for manual rule:
  // Don't know if this behavior is a bug issue or desired: cf libmythtv/recordingrule.cpp
  if (modifier.SearchType() != Myth::ST_ManualSearch)
    modifier.SetSearchType(Myth::ST_NoSearch);

  modifier.SetType(Myth::RT_DontRecord);
  modifier.SetParentID(modifier.RecordID());
  modifier.SetRecordID(0);
  modifier.SetInactive(false);
  // Assign recording info
  modifier.SetTitle(recording.Title());
  modifier.SetSubtitle(recording.Subtitle());
  modifier.SetDescription(recording.Description());
  modifier.SetChannelID(recording.ChannelID());
  modifier.SetCallsign(recording.Callsign());
  modifier.SetStartTime(recording.StartTime());
  modifier.SetEndTime(recording.EndTime());
  modifier.SetSeriesID(recording.SerieID());
  modifier.SetProgramID(recording.ProgramID());
  modifier.SetCategory(recording.Category());
  if (rule.InetRef().empty())
  {
    modifier.SetInerRef(recording.Inetref());
    modifier.SetSeason(recording.Season());
    modifier.SetEpisode(recording.Episode());
  }
  return modifier;
}

MythRecordingRule MythScheduleHelper75::MakeOverride(const MythRecordingRule& rule, const MythProgramInfo& recording)
{
  MythRecordingRule modifier = rule.DuplicateRecordingRule();
  // Do the same as backend even we know the modifier will be rejected for manual rule:
  // Don't know if this behavior is a bug issue or desired: cf libmythtv/recordingrule.cpp
  if (modifier.SearchType() != Myth::ST_ManualSearch)
    modifier.SetSearchType(Myth::ST_NoSearch);

  modifier.SetType(Myth::RT_OverrideRecord);
  modifier.SetParentID(modifier.RecordID());
  modifier.SetRecordID(0);
  modifier.SetInactive(false);
  // Assign recording info
  modifier.SetTitle(recording.Title());
  modifier.SetSubtitle(recording.Subtitle());
  modifier.SetDescription(recording.Description());
  modifier.SetChannelID(recording.ChannelID());
  modifier.SetCallsign(recording.Callsign());
  modifier.SetStartTime(recording.StartTime());
  modifier.SetEndTime(recording.EndTime());
  modifier.SetSeriesID(recording.SerieID());
  modifier.SetProgramID(recording.ProgramID());
  modifier.SetCategory(recording.Category());
  if (rule.InetRef().empty())
  {
    modifier.SetInerRef(recording.Inetref());
    modifier.SetSeason(recording.Season());
    modifier.SetEpisode(recording.Episode());
  }
  return modifier;
}

const MythTimerType::AttributeList& MythScheduleHelper75::GetRulePriorityList() const
{
  if (!m_priorityListInit)
  {
    char buf[4];
    m_priorityListInit = true;
    m_priorityList.reserve(200);
    memset(buf, 0, sizeof(buf));
    for (int i = -99; i <= 99; ++i)
    {
      if (i)
      {
        snprintf(buf, sizeof(buf), "%+2d", i);
        m_priorityList.push_back(std::make_pair(i, buf));
      }
      else
        m_priorityList.push_back(std::make_pair(0, "0"));
    }
  }
  return m_priorityList;
}

const MythTimerType::AttributeList& MythScheduleHelper75::GetRuleDupMethodList() const
{
  if (!m_dupMethodListInit)
  {
    m_dupMethodListInit = true;
    m_dupMethodList.push_back(std::make_pair(static_cast<int>(Myth::DM_CheckNone), XBMC->GetLocalizedString(30501)));
    m_dupMethodList.push_back(std::make_pair(static_cast<int>(Myth::DM_CheckSubtitle), XBMC->GetLocalizedString(30502)));
    m_dupMethodList.push_back(std::make_pair(static_cast<int>(Myth::DM_CheckDescription), XBMC->GetLocalizedString(30503)));
    m_dupMethodList.push_back(std::make_pair(static_cast<int>(Myth::DM_CheckSubtitleAndDescription), XBMC->GetLocalizedString(30504)));
    m_dupMethodList.push_back(std::make_pair(static_cast<int>(Myth::DM_CheckSubtitleThenDescription), XBMC->GetLocalizedString(30505)));
  }
  return m_dupMethodList;
}

const MythScheduleHelperNoHelper::RuleExpirationMap& MythScheduleHelper75::GetRuleExpirationMap() const
{
  if (!m_expirationMapInit)
  {
    m_expirationMapInit = true;
    char buf[256];
    memset(buf, 0, sizeof(buf));

    // Insert cases 'keep newest'
    int index = (EXPIRATION_NEVER_EXPIRE_ID < EXPIRATION_ALLOW_EXPIRE_ID ? EXPIRATION_NEVER_EXPIRE_ID : EXPIRATION_ALLOW_EXPIRE_ID) - 100;
    for (int i = 100; i >= 1; --i)
    {
      snprintf(buf, sizeof(buf), XBMC->GetLocalizedString(30509), i); // Keep %d newest and expire old
      m_expirationMap.insert(std::make_pair(index++, std::make_pair(RuleExpiration(false,i,true), buf)));
    }

    // Insert cases 'auto expire'
    m_expirationMap.insert(std::make_pair(EXPIRATION_NEVER_EXPIRE_ID, std::make_pair(RuleExpiration(false, 0, false), XBMC->GetLocalizedString(30506)))); // Recordings never expire
    m_expirationMap.insert(std::make_pair(EXPIRATION_ALLOW_EXPIRE_ID, std::make_pair(RuleExpiration(true, 0, false), XBMC->GetLocalizedString(30507)))); // Allow recordings to expire

    // Insert cases 'keep up'
    index = (EXPIRATION_ALLOW_EXPIRE_ID > EXPIRATION_NEVER_EXPIRE_ID ? EXPIRATION_ALLOW_EXPIRE_ID : EXPIRATION_NEVER_EXPIRE_ID) + 1;
    for (int i = 2; i <= 100; ++i)
    {
      snprintf(buf, sizeof(buf), XBMC->GetLocalizedString(30508), i); // Keep up to %d recordings
      m_expirationMap.insert(std::make_pair(index++, std::make_pair(RuleExpiration(false,i,false), buf)));
    }
  }
  return m_expirationMap;
}

const MythTimerType::AttributeList& MythScheduleHelper75::GetRuleRecordingGroupList() const
{
  if (!m_recGroupListInit && m_control)
  {
    int count = 0, index = RECGROUP_DFLT_ID;
    m_recGroupListInit = true;
    Myth::StringListPtr strl = m_control->GetRecGroupList();
    // First add default group
    for (Myth::StringList::const_iterator it = strl->begin(); it != strl->end(); ++it)
    {
      if (*it == RECGROUP_DFLT_NAME)
      {
        m_recGroupList.push_back(std::make_pair(index++, RECGROUP_DFLT_NAME));
        ++count;
      }
    }
    // Then others
    for (Myth::StringList::const_iterator it = strl->begin(); it != strl->end(); ++it)
    {
      if (*it != RECGROUP_DFLT_NAME)
      {
        if (count == PVR_ADDON_TIMERTYPE_VALUES_ARRAY_SIZE)
        {
          XBMC->Log(LOG_NOTICE, "75::%s: List overflow (%d): %u remaining value(s) are not loaded", __FUNCTION__, count, (unsigned)(strl->size() - count));
          break;
        }
        m_recGroupList.push_back(std::make_pair(index++, *it));
        ++count;
      }
    }
  }
  return m_recGroupList;
}

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

const std::vector<MythScheduleManager::TimerType>& MythScheduleHelper75::GetTimerTypes() const
{
  static bool _init = false;
  static std::vector<MythScheduleManager::TimerType> typeList;
  if (!_init)
  {
    _init = true;

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_ONCE_MANUAL_SEARCH,
            PVR_TIMER_TYPE_IS_MANUAL |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30460),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            MythScheduleManager::RuleDupMethodList(), // empty list
            0, // Check none
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_THIS_SHOWING,
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30465),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            MythScheduleManager::RuleDupMethodList(), // empty list
            0, // Check none
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_ONE_SHOWING,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES,
            XBMC->GetLocalizedString(30461),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_ALL_SHOWINGS,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES,
            XBMC->GetLocalizedString(30464),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_ONE_SHOWING_WEEKLY,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_FIRST_DAY |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30462),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            MythScheduleManager::RuleDupMethodList(), // empty list
            0, // Check none
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_ONE_SHOWING_DAILY,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_FIRST_DAY |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30463),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            MythScheduleManager::RuleDupMethodList(), // empty list
            0, // Check none
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_TEXT_SEARCH,
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH |
            PVR_TIMER_TYPE_SUPPORTS_FULLTEXT_EPG_MATCH,
            XBMC->GetLocalizedString(30466),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    ///////////////////////////////////////////////////////////////////////////
    //// KEEP LAST
    ///////////////////////////////////////////////////////////////////////////
    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_UNHANDLED_RULE,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_IS_READONLY |
            PVR_TIMER_TYPE_IS_REPEATING |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH,
            XBMC->GetLocalizedString(30451),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_RECORD,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30452),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_OVERRIDE,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP,
            XBMC->GetLocalizedString(30453),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));

    typeList.push_back(MythScheduleManager::TimerType(TIMER_TYPE_DONT_RECORD,
            PVR_TIMER_TYPE_FORBIDS_NEW_INSTANCES |
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE,
            XBMC->GetLocalizedString(30454),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
            GetRuleExpirationList(),
            GetRuleExpirationDefault(),
            GetRuleRecordingGroupList(),
            GetRuleRecordingGroupDefault()));
  }
  return typeList;
}

const MythScheduleManager::RulePriorityList& MythScheduleHelper75::GetRulePriorityList() const
{
  static bool _init = false;
  static MythScheduleManager::RulePriorityList _list;
  if (!_init)
  {
    char buf[4];
    _init = true;
    _list.reserve(200);
    memset(buf, 0, sizeof(buf));
    for (int i = -99; i <= 99; ++i)
    {
      if (i)
      {
        snprintf(buf, sizeof(buf) - 1, "%+2d", i);
        _list.push_back(std::make_pair(i, buf));
      }
      else
        _list.push_back(std::make_pair(0, "0"));
    }
  }
  return _list;
}

const MythScheduleManager::RuleDupMethodList& MythScheduleHelper75::GetRuleDupMethodList() const
{
  static bool _init = false;
  static MythScheduleManager::RuleDupMethodList _list;
  if (!_init)
  {
    _init = true;
    _list.push_back(std::make_pair(Myth::DM_CheckNone, 30501));
    _list.push_back(std::make_pair(Myth::DM_CheckSubtitle, 30502));
    _list.push_back(std::make_pair(Myth::DM_CheckDescription, 30503));
    _list.push_back(std::make_pair(Myth::DM_CheckSubtitleAndDescription, 30504));
    _list.push_back(std::make_pair(Myth::DM_CheckSubtitleThenDescription, 30505));
  }
  return _list;
}

const MythScheduleManager::RuleExpirationList& MythScheduleHelper75::GetRuleExpirationList() const
{
  static bool _init = false;
  static MythScheduleManager::RuleExpirationList _list;
  if (!_init)
  {
    _init = true;
    _list.push_back(std::make_pair(0, 30506));
    _list.push_back(std::make_pair(1, 30507));
  }
  return _list;
}

const MythScheduleManager::RuleRecordingGroupList& MythScheduleHelper75::GetRuleRecordingGroupList() const
{
  static bool _init = false;
  static MythScheduleManager::RuleRecordingGroupList _list;
  if (!_init && m_control)
  {
    int index = RECGROUP_ID_NONE;
    _init = true;
    _list.push_back(std::make_pair(index++, "?"));
    Myth::StringListPtr strl = m_control->GetRecGroupList();
    for (Myth::StringList::const_iterator it = strl->begin(); it != strl->end(); ++it)
      _list.push_back(std::make_pair(index++, *it));
  }
  return _list;
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

bool MythScheduleHelper75::FillTimerEntry(MythTimerEntry& entry, const MythRecordingRuleNode& node) const
{
  MythRecordingRule rule = node.GetRule();
  XBMC->Log(LOG_DEBUG,"%s (MythScheduleHelper75): %u (%s:%s)", __FUNCTION__, rule.RecordID(), rule.Title().c_str(), rule.Subtitle().c_str());
  switch (rule.Type())
  {
    case Myth::RT_DailyRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING_DAILY;
      break;
    case Myth::RT_WeeklyRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING_WEEKLY;
      break;
    case Myth::RT_AllRecord:
      //NB: I seem to remember reading on the mythv forum that the reason for deprectating the RT_ChannelRecord
      //type was that it could also be achieved using AllRecord and 'This Channel' filter setting.
      if (!(rule.Filter() & Myth::FM_ThisChannel))
        entry.isAnyChannel = true; // Identify as AnyChannel rule, then fall through to standard case
    case Myth::RT_ChannelRecord:
      entry.timerType = TIMER_TYPE_ALL_SHOWINGS;
      if (rule.SearchType() == Myth::ST_NoSearch)
        entry.epgSearch = rule.Title(); // EPG based
      break;
    case Myth::RT_SingleRecord:
      {
        entry.timerType = TIMER_TYPE_THIS_SHOWING;
        // Fill recording status from its upcoming
        MythScheduleList recordings = m_manager->FindUpComingByRuleId(rule.RecordID());
        MythScheduleList::const_reverse_iterator it = recordings.rbegin();
        if (it != recordings.rend())
          entry.recordingStatus = it->second->Status();
        else
          return false; // Don't transfer single without upcoming
      }
      break;
    case Myth::RT_OneRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING;
      if (!(rule.Filter() & Myth::FM_ThisChannel))
        entry.isAnyChannel = true; // Identify as AnyChannel rule, then fall through to standard case
      break;
    case Myth::RT_DontRecord:
      entry.timerType = TIMER_TYPE_DONT_RECORD;
      break;
    case Myth::RT_OverrideRecord:
      entry.timerType = TIMER_TYPE_OVERRIDE;
      break;
    default:
      entry.timerType = TIMER_TYPE_UNHANDLED_RULE;
      break;
  }
  switch (rule.SearchType())
  {
    case Myth::ST_TitleSearch:
      entry.epgSearch = rule.Description();
      break;
    case Myth::ST_KeywordSearch:
    case Myth::ST_PeopleSearch:
    case Myth::ST_PowerSearch:
      entry.epgSearch = rule.Description();
      entry.timerType = TIMER_TYPE_UNHANDLED_RULE;
    default:
      break;
  }
  entry.title = rule.Title();
  entry.chanid = rule.ChannelID();
  entry.callsign = rule.Callsign();

  MythScheduleList recordings;
  switch (entry.timerType)
  {
    case TIMER_TYPE_ALL_SHOWINGS:
    case TIMER_TYPE_ONE_SHOWING:
    case TIMER_TYPE_ONE_SHOWING_DAILY:
    case TIMER_TYPE_ONE_SHOWING_WEEKLY:
    case TIMER_TYPE_UNHANDLED_RULE:
      if (difftime(rule.NextRecording(), 0) > 0)
      {
        // fill timeslot starting at next recording
        entry.startTime = entry.endTime = rule.NextRecording();
        timeadd(&entry.endTime, difftime(rule.EndTime(), rule.StartTime()));
      }
      else if (difftime(rule.LastRecorded(), 0) > 0)
      {
        // fill timeslot starting at last recorded
        entry.startTime = entry.endTime = rule.LastRecorded();
        timeadd(&entry.endTime, difftime(rule.EndTime(), rule.StartTime()));
      }
      else
      {
        entry.startTime = rule.StartTime();
        entry.endTime = rule.EndTime();
      }
      //Mythbackend doesn't appear to consider currently active recordings, so check for one
      recordings = m_manager->FindUpComingByRuleId(rule.RecordID());
      for (MythScheduleList::const_reverse_iterator it = recordings.rbegin(); it !=recordings.rend(); ++it)
      {
        if ( (it->second->Status() == Myth::RS_RECORDING) ||
             (it->second->Status() == Myth::RS_TUNING) )
        {

            entry.recordingStatus = it->second->Status();
            entry.startTime = it->second->StartTime();
            entry.endTime = it->second->EndTime();
            XBMC->Log(LOG_DEBUG,"%s: Found active recording for rule %u", __FUNCTION__, rule.RecordID());
            break;
        }
      }
      break;
    default:
      entry.startTime = rule.StartTime();
      entry.endTime = rule.EndTime();
  }

  entry.startOffset = rule.StartOffset();
  entry.endOffset = rule.EndOffset();
  entry.dupMethod = rule.DuplicateControlMethod();
  entry.priority = rule.Priority();
  entry.autoExpire = rule.AutoExpire();
  entry.isInactive = rule.Inactive();
  entry.recordingGroup = GetRuleRecordingGroupId(rule.RecordingGroup());
  return true;
}

MythRecordingRule MythScheduleHelper75::NewFromTemplate(const MythEPGInfo& epgInfo)
{
  MythRecordingRule rule;
  // Load rule template from selected provider
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
          tplIt = it;
          break;
        }
        if (it->Category() == epgInfo.CategoryType())
        {
          tplIt = it;
          continue;
        }
        if (it->Category() == "Default" && tplIt == templates.end())
          tplIt = it;
      }
      if (tplIt != templates.end())
      {
        XBMC->Log(LOG_INFO, "Overriding the rule with template %u '%s'", (unsigned)tplIt->RecordID(), tplIt->Title().c_str());
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
      }
      else
        XBMC->Log(LOG_INFO, "No template found for the category '%s'", epgInfo.Category().c_str());
    }
    break;
  case 0: // Template provider is 'Internal', then set rule with settings
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
    rule.SetPriority(GetRulePriorityDefault());
    rule.SetAutoExpire(GetRuleExpirationDefault());
    rule.SetDuplicateControlMethod(static_cast<Myth::DM_t>(GetRuleDupMethodDefault()));
    rule.SetCheckDuplicatesInType(Myth::DI_InAll);
    rule.SetRecordingGroup(GetRuleRecordingGroupName(GetRuleRecordingGroupDefault()));
  }

  // Category override
  if (!epgInfo.IsNull())
  {
    Myth::SettingPtr overTimeCategory = m_control->GetSetting("OverTimeCategory", false);
    if (overTimeCategory && (overTimeCategory->value == epgInfo.Category() || overTimeCategory->value == epgInfo.CategoryType()))
    {
      Myth::SettingPtr categoryOverTime = m_control->GetSetting("CategoryOverTime", false);
      if (categoryOverTime)
      {
        XBMC->Log(LOG_DEBUG, "Overriding end offset for category %s: +%s", overTimeCategory->value.c_str(), categoryOverTime->value.c_str());
        rule.SetEndOffset(atoi(categoryOverTime->value.c_str()));
      }
    }
  }
  return rule;
}

MythRecordingRule MythScheduleHelper75::NewFromTimer(const MythTimerEntry& entry, bool withTemplate)
{
  MythRecordingRule rule;
  if (withTemplate)
  {
    // Base on template
    rule = NewFromTemplate(entry.epgInfo);
    // Override template with timer settings
    rule.SetStartOffset(rule.StartOffset() + entry.startOffset);
    rule.SetEndOffset(rule.EndOffset() + entry.endOffset);
    if (entry.dupMethod != GetRuleDupMethodDefault())
    {
      rule.SetDuplicateControlMethod(entry.dupMethod);
      rule.SetCheckDuplicatesInType(Myth::DI_InAll);
    }
    if (entry.priority != GetRulePriorityDefault())
      rule.SetPriority(entry.priority);
    if (entry.autoExpire != GetRuleExpirationDefault())
      rule.SetAutoExpire(entry.autoExpire);
    if (entry.recordingGroup != RECGROUP_ID_NONE)
      rule.SetRecordingGroup(GetRuleRecordingGroupName(entry.recordingGroup));
  }
  else
  {
    rule.SetStartOffset(entry.startOffset);
    rule.SetEndOffset(entry.endOffset);
    rule.SetDuplicateControlMethod(entry.dupMethod);
    rule.SetPriority(entry.priority);
    rule.SetAutoExpire(entry.autoExpire);
    rule.SetRecordingGroup(GetRuleRecordingGroupName(entry.recordingGroup));
  }

  switch (entry.timerType)
  {
    case TIMER_TYPE_ONE_SHOWING_WEEKLY:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_WeeklyRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.description);
        rule.SetCallsign(entry.epgInfo.Callsign());
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (entry.HasChannel() && entry.HasTimeSlot())
      {
        rule.SetType(Myth::RT_WeeklyRecord);
        rule.SetSearchType(Myth::ST_ManualSearch); // Using timeslot
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
        rule.SetStartTime(entry.startTime);
        rule.SetEndTime(entry.endTime);
        rule.SetTitle(entry.title);
        rule.SetDescription(entry.description);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (!entry.epgSearch.empty())
      {
        rule.SetType(Myth::RT_WeeklyRecord);
        rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        if (entry.HasChannel())
        {
          rule.SetChannelID(entry.chanid);
          rule.SetCallsign(entry.callsign);
        }
        rule.SetTitle(entry.title);
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle("");
        rule.SetDescription(entry.epgSearch);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      break;
    }

    case TIMER_TYPE_ONE_SHOWING_DAILY:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_DailyRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.description);
        rule.SetCallsign(entry.epgInfo.Callsign());
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (entry.HasChannel() && entry.HasTimeSlot())
      {
        rule.SetType(Myth::RT_DailyRecord);
        rule.SetSearchType(Myth::ST_ManualSearch); // Using timeslot
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
        rule.SetStartTime(entry.startTime);
        rule.SetEndTime(entry.endTime);
        rule.SetTitle(entry.title);
        rule.SetDescription(entry.description);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (!entry.epgSearch.empty())
      {
        rule.SetType(Myth::RT_DailyRecord);
        rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        if (entry.HasChannel())
        {
          rule.SetChannelID(entry.chanid);
          rule.SetCallsign(entry.callsign);
        }
        rule.SetTitle(entry.title);
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle("");
        rule.SetDescription(entry.epgSearch);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      break;
    }

    case TIMER_TYPE_ONCE_MANUAL_SEARCH:
    case TIMER_TYPE_THIS_SHOWING:
    case TIMER_TYPE_ONE_SHOWING:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_SingleRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.description);
        rule.SetCallsign(entry.epgInfo.Callsign());
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (entry.HasChannel() && entry.HasTimeSlot())
      {
        rule.SetType(Myth::RT_SingleRecord);
        rule.SetSearchType(Myth::ST_ManualSearch); // Using timeslot
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
        rule.SetStartTime(entry.startTime);
        rule.SetEndTime(entry.endTime);
        rule.SetTitle(entry.title);
        rule.SetDescription(entry.description);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (!entry.epgSearch.empty())
      {
        rule.SetType(Myth::RT_OneRecord);
        rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        if (entry.HasChannel())
        {
          rule.SetChannelID(entry.chanid);
          rule.SetCallsign(entry.callsign);
        }
        rule.SetTitle(entry.title);
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle("");
        rule.SetDescription(entry.epgSearch);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      break;
    }

    case TIMER_TYPE_ALL_SHOWINGS:
    {
      if (!entry.epgInfo.IsNull())
      {
        if (entry.HasChannel() && !entry.isAnyChannel)
          rule.SetType(Myth::RT_ChannelRecord);
        else
          rule.SetType(Myth::RT_AllRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.description);
        rule.SetCallsign(entry.epgInfo.Callsign());
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      if (!entry.epgSearch.empty())
      {
        if (entry.HasChannel() && !entry.isAnyChannel)
          rule.SetType(Myth::RT_ChannelRecord);
        else
          rule.SetType(Myth::RT_AllRecord);
        rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        if (entry.HasChannel())
        {
          rule.SetChannelID(entry.chanid);
          rule.SetCallsign(entry.callsign);
        }
        rule.SetTitle(entry.title);
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle("");
        rule.SetDescription(entry.epgSearch);
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      break;
    }

    case TIMER_TYPE_DONT_RECORD:
    case TIMER_TYPE_OVERRIDE:
    case TIMER_TYPE_RECORD:
      // any type
      rule.SetType(Myth::RT_NotRecording);
      rule.SetChannelID(entry.chanid);
      rule.SetCallsign(entry.callsign);
      rule.SetStartTime(entry.startTime);
      rule.SetEndTime(entry.endTime);
      rule.SetTitle(entry.title);
      rule.SetDescription(entry.description);
      rule.SetInactive(entry.isInactive);
      return rule;

    default:
      break;
  }
  rule.SetType(Myth::RT_UNKNOWN);
  XBMC->Log(LOG_ERROR, "%s - Invalid timer %u: TYPE=%d CHANID=%u SIGN=%s ST=%u ET=%u", __FUNCTION__, entry.entryIndex,
          entry.timerType, entry.chanid, entry.callsign.c_str(), (unsigned)entry.startTime, (unsigned)entry.endTime);
  return rule;
}

MythScheduleManager::RuleSummaryInfo MythScheduleHelper75::GetSummaryInfo(const MythRecordingRule &rule) const
{
  MythScheduleManager::RuleSummaryInfo meta;
  time_t st = rule.StartTime();
  meta.isRepeating = false;
  meta.weekDays = 0;
  meta.marker = "";
  switch (rule.Type())
  {
    case Myth::RT_DailyRecord:
    case Myth::RT_FindDailyRecord:
      meta.isRepeating = true;
      meta.weekDays = 0x7F;
      meta.marker = "d";
      break;
    case Myth::RT_WeeklyRecord:
    case Myth::RT_FindWeeklyRecord:
      meta.isRepeating = true;
      meta.weekDays = 1 << ((weekday(&st) + 6) % 7);
      meta.marker = "w";
      break;
    case Myth::RT_ChannelRecord:
      meta.isRepeating = true;
      meta.weekDays = 0x7F;
      meta.marker = "C";
      break;
    case Myth::RT_AllRecord:
      meta.isRepeating = true;
      meta.weekDays = 0x7F;
      meta.marker = "A";
      break;
    case Myth::RT_OneRecord:
      meta.isRepeating = false;
      meta.weekDays = 0;
      meta.marker = "1";
      break;
    case Myth::RT_DontRecord:
      meta.isRepeating = false;
      meta.weekDays = 0;
      meta.marker = "x";
      break;
    case Myth::RT_OverrideRecord:
      meta.isRepeating = false;
      meta.weekDays = 0;
      meta.marker = "o";
      break;
    default:
      break;
  }
  return meta;
}

MythRecordingRule MythScheduleHelper75::NewSingleRecord(const MythEPGInfo& epgInfo)
{
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_SingleRecord);

  if (!epgInfo.IsNull())
  {
    rule.SetChannelID(epgInfo.ChannelID());
    rule.SetStartTime(epgInfo.StartTime());
    rule.SetEndTime(epgInfo.EndTime());
    rule.SetSearchType(Myth::ST_NoSearch);
    rule.SetTitle(epgInfo.Title());
    rule.SetSubtitle(epgInfo.Subtitle());
    rule.SetCategory(epgInfo.Category());
    rule.SetDescription(epgInfo.Description());
    rule.SetCallsign(epgInfo.Callsign());
    rule.SetProgramID(epgInfo.ProgramID());
    rule.SetSeriesID(epgInfo.SeriesID());
  }
  else
  {
    // Using timeslot
    rule.SetSearchType(Myth::ST_ManualSearch);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckNone);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper75::NewDailyRecord(const MythEPGInfo& epgInfo)
{
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_DailyRecord);

  if (!epgInfo.IsNull())
  {
    rule.SetSearchType(Myth::ST_NoSearch);
    rule.SetChannelID(epgInfo.ChannelID());
    rule.SetStartTime(epgInfo.StartTime());
    rule.SetEndTime(epgInfo.EndTime());
    rule.SetTitle(epgInfo.Title());
    rule.SetSubtitle(epgInfo.Subtitle());
    rule.SetCategory(epgInfo.Category());
    rule.SetDescription(epgInfo.Description());
    rule.SetCallsign(epgInfo.Callsign());
    rule.SetProgramID(epgInfo.ProgramID());
    rule.SetSeriesID(epgInfo.SeriesID());
  }
  else
  {
    // Using timeslot
    rule.SetSearchType(Myth::ST_ManualSearch);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper75::NewWeeklyRecord(const MythEPGInfo& epgInfo)
{
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_WeeklyRecord);

  if (!epgInfo.IsNull())
  {
    rule.SetSearchType(Myth::ST_NoSearch);
    rule.SetChannelID(epgInfo.ChannelID());
    rule.SetStartTime(epgInfo.StartTime());
    rule.SetEndTime(epgInfo.EndTime());
    rule.SetTitle(epgInfo.Title());
    rule.SetSubtitle(epgInfo.Subtitle());
    rule.SetCategory(epgInfo.Category());
    rule.SetDescription(epgInfo.Description());
    rule.SetCallsign(epgInfo.Callsign());
    rule.SetProgramID(epgInfo.ProgramID());
    rule.SetSeriesID(epgInfo.SeriesID());
  }
  else
  {
    // Using timeslot
    rule.SetSearchType(Myth::ST_ManualSearch);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper75::NewChannelRecord(const MythEPGInfo& epgInfo)
{
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_ChannelRecord);

  if (!epgInfo.IsNull())
  {
    rule.SetSearchType(Myth::ST_TitleSearch);
    rule.SetChannelID(epgInfo.ChannelID());
    rule.SetStartTime(epgInfo.StartTime());
    rule.SetEndTime(epgInfo.EndTime());
    rule.SetTitle(epgInfo.Title());
    // Backend use the description to find program by keywords or title
    rule.SetSubtitle("");
    rule.SetDescription(epgInfo.Title());
    rule.SetCategory(epgInfo.Category());
    rule.SetCallsign(epgInfo.Callsign());
    rule.SetProgramID(epgInfo.ProgramID());
    rule.SetSeriesID(epgInfo.SeriesID());
  }
  else
  {
    // Not feasible
    rule.SetType(Myth::RT_NotRecording);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper75::NewOneRecord(const MythEPGInfo& epgInfo)
{
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_OneRecord);

  if (!epgInfo.IsNull())
  {
    rule.SetSearchType(Myth::ST_TitleSearch);
    rule.SetChannelID(epgInfo.ChannelID());
    rule.SetStartTime(epgInfo.StartTime());
    rule.SetEndTime(epgInfo.EndTime());
    rule.SetTitle(epgInfo.Title());
    // Backend use the description to find program by keywords or title
    rule.SetSubtitle("");
    rule.SetDescription(epgInfo.Title());
    rule.SetCategory(epgInfo.Category());
    rule.SetCallsign(epgInfo.Callsign());
    rule.SetProgramID(epgInfo.ProgramID());
    rule.SetSeriesID(epgInfo.SeriesID());
  }
  else
  {
    // Not feasible
    rule.SetType(Myth::RT_NotRecording);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

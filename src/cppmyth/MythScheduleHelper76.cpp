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
//// Version helper for database up to 1309 (0.27)
////
//// Remove the Timeslot and Weekslot recording rule types. These rule
//// types are too rigid and don't work when a broadcaster shifts the
//// starting time of a program by a few minutes. Users should now use
//// Channel recording rules in place of Timeslot and Weekslot rules. To
//// approximate the old functionality, two new schedule filters have been
//// added. In addition, the new "This time" and "This day and time"
//// filters are less strict and match any program starting within 10
//// minutes of the recording rule time.
//// Restrict the use of the FindDaily? and FindWeekly? recording rule types
//// (now simply called Daily and Weekly) to search and manual recording
//// rules. These rule types are rarely needed and limiting their use to
//// the most powerful cases simplifies the user interface for the more
//// common cases. Users should now use Daily and Weekly, custom search
//// rules in place of FindDaily? and FindWeekly? rules.
//// Any existing recording rules using the no longer supported or allowed
//// types are automatically converted to the suggested alternatives.
////

#include "MythScheduleHelper76.h"
#include "../client.h"
#include "../tools.h"

#include <cstdio>
#include <cassert>

using namespace ADDON;

bool MythScheduleHelper76::FillTimerEntry(MythTimerEntry& entry, const MythRecordingRuleNode& node) const
{
  MythRecordingRule rule = node.GetRule();
  XBMC->Log(LOG_DEBUG,"%s (MythScheduleHelper76): %u (%s:%s)", __FUNCTION__, rule.RecordID(), rule.Title().c_str(), rule.Subtitle().c_str());
  switch (rule.Type())
  {
    case Myth::RT_DailyRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING_DAILY;
      break;
    case Myth::RT_WeeklyRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING_WEEKLY;
      break;
    case Myth::RT_AllRecord:
      if ((rule.Filter() & Myth::FM_ThisDayAndTime))
        entry.timerType = TIMER_TYPE_ONE_SHOWING_WEEKLY;
      else if ((rule.Filter() & Myth::FM_ThisTime))
        entry.timerType = TIMER_TYPE_ONE_SHOWING_DAILY;
      else
      {
        if (rule.SearchType() == Myth::ST_NoSearch) 
        {
          entry.timerType = TIMER_TYPE_ALL_SHOWINGS;
        }
        else if (rule.SearchType() == Myth::ST_TitleSearch)
        {
          entry.timerType = TIMER_TYPE_TEXT_SEARCH;
          entry.epgSearch = rule.Description();
          entry.isFullTextSearch = false;
        }
        else if (rule.SearchType() == Myth::ST_KeywordSearch)
        {
          entry.timerType = TIMER_TYPE_TEXT_SEARCH;
          entry.epgSearch = rule.Description();
          entry.isFullTextSearch = true;
        }
      }
      if (!(rule.Filter() & Myth::FM_ThisChannel))
        entry.isAnyChannel = true; // Identify this as an AnyChannel rule
      break;
    case Myth::RT_SingleRecord:
      {
        if (rule.SearchType()  == Myth::ST_ManualSearch)
          entry.timerType = TIMER_TYPE_ONCE_MANUAL_SEARCH;
        else
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
        entry.isAnyChannel = true; // Identify this as an AnyChannel rule
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
    case Myth::ST_PeopleSearch:
    case Myth::ST_PowerSearch:
      entry.epgSearch = rule.Description();
      entry.timerType = TIMER_TYPE_UNHANDLED_RULE;
      break;
    case Myth::ST_TitleSearch:
    case Myth::ST_KeywordSearch:
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
    case TIMER_TYPE_TEXT_SEARCH:
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
      //Mythbackend (0.27) doesn't appear to consider currently active recordings, so check for one
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
  entry.firstShowing = (rule.Filter() & Myth::FM_FirstShowing ? true : false);
  entry.recordingGroup = GetRuleRecordingGroupId(rule.RecordingGroup());
  return true;
}

MythRecordingRule MythScheduleHelper76::NewFromTimer(const MythTimerEntry& entry, bool withTemplate)
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
    case TIMER_TYPE_ONCE_MANUAL_SEARCH:
    case TIMER_TYPE_THIS_SHOWING:
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
      if (entry.HasChannel() && entry.HasTimeSlot() && !entry.isAnyChannel)
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
      break;
    }

    case TIMER_TYPE_ONE_SHOWING_WEEKLY:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_AllRecord);
        rule.SetFilter(Myth::FM_ThisChannel + Myth::FM_ThisDayAndTime);
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
      break;
    }

    case TIMER_TYPE_ONE_SHOWING_DAILY:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_AllRecord);
        rule.SetFilter(Myth::FM_ThisChannel + Myth::FM_ThisTime);
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
      if (entry.HasChannel() && entry.HasTimeSlot() && !entry.isAnyChannel)
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
      break;
    }

    case TIMER_TYPE_ONE_SHOWING:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_OneRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetCallsign(entry.epgInfo.Callsign());
        if (!entry.isAnyChannel) rule.SetFilter(Myth::FM_ThisChannel);
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.description);
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      break;
    }

    case TIMER_TYPE_ALL_SHOWINGS:
    {
      if (!entry.epgInfo.IsNull())
      {
        rule.SetType(Myth::RT_AllRecord);
        rule.SetSearchType(Myth::ST_NoSearch);
        rule.SetChannelID(entry.epgInfo.ChannelID());
        rule.SetCallsign(entry.epgInfo.Callsign());
        if (!entry.isAnyChannel) rule.SetFilter(Myth::FM_ThisChannel);
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetTitle(entry.epgInfo.Title());
        rule.SetSubtitle(entry.epgInfo.Subtitle());
        rule.SetDescription(entry.description);
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetProgramID(entry.epgInfo.ProgramID());
        rule.SetSeriesID(entry.epgInfo.SeriesID());
        rule.SetInactive(entry.isInactive);
        return rule;
      }
      break;
    }

    case TIMER_TYPE_TEXT_SEARCH:
    {
      rule.SetType(Myth::RT_AllRecord);
      rule.SetInactive(entry.isInactive);
      if (!entry.epgInfo.IsNull())
      {
        if (entry.isAnyChannel) 
        { 
          rule.SetChannelID(entry.epgInfo.ChannelID());
          rule.SetCallsign(entry.epgInfo.Callsign());
        }
        rule.SetStartTime(entry.epgInfo.StartTime());
        rule.SetEndTime(entry.epgInfo.EndTime());
        rule.SetCategory(entry.epgInfo.Category());
        rule.SetInactive(entry.isInactive);
      }
      if (!entry.isAnyChannel)
      {
        rule.SetChannelID(entry.chanid);
        rule.SetCallsign(entry.callsign);
        rule.SetFilter(Myth::FM_ThisChannel);
      }
      if (!entry.epgSearch.empty())
      {
        if (entry.isFullTextSearch)
        {
          rule.SetTitle(entry.title); //(Keyword Search)
          rule.SetSearchType(Myth::ST_KeywordSearch);
        }
        else
        {
          rule.SetTitle(entry.title); //(Title Search)
          rule.SetSearchType(Myth::ST_TitleSearch);
        }
        // Backend use the subtitle/description to find program by keywords or title
        rule.SetSubtitle(""); // Backend uses Subtitle for table join SQL for power searches (not needed for keyword or title)
        rule.SetDescription(entry.epgSearch); // Backend uses description to find program by keywords or title and SQL for power searches
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

MythScheduleManager::RuleSummaryInfo MythScheduleHelper76::GetSummaryInfo(const MythRecordingRule &rule) const
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
      if ((rule.Filter() & Myth::FM_ThisDayAndTime))
      {
        meta.weekDays = 1 << ((weekday(&st) + 6) % 7);
        meta.marker = "w";
      }
      else if ((rule.Filter() & Myth::FM_ThisTime))
      {
        meta.weekDays = 0x7F;
        meta.marker = "d";
      }
      else
      {
        meta.weekDays = 0x7F;
        meta.marker = "A";
      }
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

MythRecordingRule MythScheduleHelper76::NewDailyRecord(const MythEPGInfo& epgInfo)
{
  unsigned int filter;
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_AllRecord);
  filter = Myth::FM_ThisChannel + Myth::FM_ThisTime;
  rule.SetFilter(filter);

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
    // No EPG! Create custom daily for this channel
    rule.SetType(Myth::RT_DailyRecord);
    rule.SetFilter(Myth::FM_ThisChannel);
    // Using timeslot
    rule.SetSearchType(Myth::ST_ManualSearch);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper76::NewWeeklyRecord(const MythEPGInfo& epgInfo)
{
  unsigned int filter;
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_AllRecord);
  filter = Myth::FM_ThisChannel + Myth::FM_ThisDayAndTime;
  rule.SetFilter(filter);

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
    // No EPG! Create custom weekly for this channel
    rule.SetType(Myth::RT_WeeklyRecord);
    rule.SetFilter(Myth::FM_ThisChannel);
    // Using timeslot
    rule.SetSearchType(Myth::ST_ManualSearch);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper76::NewChannelRecord(const MythEPGInfo& epgInfo)
{
  unsigned int filter;
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_AllRecord);
  filter = Myth::FM_ThisChannel;
  rule.SetFilter(filter);

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
    // Not feasible
    rule.SetType(Myth::RT_NotRecording);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

MythRecordingRule MythScheduleHelper76::NewOneRecord(const MythEPGInfo& epgInfo)
{
  unsigned int filter;
  MythRecordingRule rule = this->NewFromTemplate(epgInfo);

  rule.SetType(Myth::RT_OneRecord);
  filter = Myth::FM_ThisEpisode;
  rule.SetFilter(filter);

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
    // Not feasible
    rule.SetType(Myth::RT_NotRecording);
  }
  rule.SetDuplicateControlMethod(Myth::DM_CheckSubtitleAndDescription);
  rule.SetCheckDuplicatesInType(Myth::DI_InAll);
  rule.SetInactive(false);
  return rule;
}

/*
 *      Copyright (C) 2005-2014 Team XBMC
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

#include "MythScheduleManager.h"
#include "../client.h"
#include "../tools.h"

#include <cstdio>
#include <cassert>

#define RECGROUP_ID_NONE    0
#define RECGROUP_DFLT_NAME  "Default"

using namespace ADDON;
using namespace PLATFORM;

enum
{
  METHOD_UNKNOWN            = 0,
  METHOD_NOOP               = 1,
  METHOD_UPDATE_INACTIVE,
  METHOD_CREATE_OVERRIDE,
  METHOD_CREATE_DONTRECORD,
  METHOD_DELETE,
  METHOD_DISCREET_UPDATE,
  METHOD_FULL_UPDATE
};

static uint_fast32_t hashvalue(uint_fast32_t maxsize, const char *value)
{
  uint_fast32_t h = 0, g;

  while (*value)
  {
    h = (h << 4) + *value++;
    if ((g = h & 0xF0000000L))
    {
      h ^= g >> 24;
    }
    h &= ~g;
  }

  return h % maxsize;
}

///////////////////////////////////////////////////////////////////////////////
////
//// MythRecordingRuleNode
////

MythRecordingRuleNode::MythRecordingRuleNode(const MythRecordingRule &rule)
: m_rule(rule)
, m_mainRule()
, m_overrideRules()
{
}

bool MythRecordingRuleNode::IsOverrideRule() const
{
  return (m_rule.Type() == Myth::RT_DontRecord || m_rule.Type() == Myth::RT_OverrideRecord);
}

MythRecordingRule MythRecordingRuleNode::GetRule() const
{
  return m_rule;
}

MythRecordingRule MythRecordingRuleNode::GetMainRule() const
{
  if (this->IsOverrideRule())
    return m_mainRule;
  return m_rule;
}

bool MythRecordingRuleNode::HasOverrideRules() const
{
  return (!m_overrideRules.empty());
}

MythRecordingRuleList MythRecordingRuleNode::GetOverrideRules() const
{
  return m_overrideRules;
}

bool MythRecordingRuleNode::IsInactiveRule() const
{
  return m_rule.Inactive();
}


///////////////////////////////////////////////////////////////////////////////
////
//// MythScheduleManager
////

MythScheduleManager::MythScheduleManager(const std::string& server, unsigned protoPort, unsigned wsapiPort, const std::string& wsapiSecurityPin)
: m_lock()
, m_control(NULL)
, m_protoVersion(0)
, m_versionHelper(NULL)
, m_showNotRecording(false)
{
  m_control = new Myth::Control(server, protoPort, wsapiPort, wsapiSecurityPin);
  this->Update();
}

MythScheduleManager::~MythScheduleManager()
{
  SAFE_DELETE(m_versionHelper);
  SAFE_DELETE(m_control);
}

void MythScheduleManager::Setup()
{
  int old = m_protoVersion;
  m_protoVersion = m_control->CheckService();

  // On new connection the protocol version could change
  if (m_protoVersion != old)
  {
    SAFE_DELETE(m_versionHelper);
    if (m_protoVersion >= 76)
      m_versionHelper = new MythScheduleHelper76(this, m_control);
    else if (m_protoVersion >= 75)
      m_versionHelper = new MythScheduleHelper75(this, m_control);
    else
      m_versionHelper = new MythScheduleHelperNoHelper();
  }
}

uint32_t MythScheduleManager::MakeIndex(const MythProgramInfo& recording)
{
  // Recordings must keep same identifier even after refreshing cache (cf Update).
  // Numeric hash of UID is used to make the constant numeric identifier.
  // Since client share index range with 'node', I exclusively reserve the range
  // of values 0x80000000 - 0xFFFFFFFF for indexing of upcoming.
  uint32_t index = 0x80000000L | (recording.RecordID() << 16) | hashvalue(0xFFFF, recording.UID().c_str());
  return index;
}

uint32_t MythScheduleManager::MakeIndex(const MythRecordingRule& rule)
{
  // The range of values 0x0 - 0x7FFFFFFF is reserved for indexing of rule.
  return rule.RecordID() & 0x7FFFFFFFL;
}

MythRecordingRule MythScheduleManager::MakeDontRecord(const MythRecordingRule& rule, const MythScheduledPtr& recording)
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
  modifier.SetTitle(recording->Title());
  modifier.SetSubtitle(recording->Subtitle());
  modifier.SetDescription(recording->Description());
  modifier.SetChannelID(recording->ChannelID());
  modifier.SetCallsign(recording->Callsign());
  modifier.SetStartTime(recording->StartTime());
  modifier.SetEndTime(recording->EndTime());
  modifier.SetSeriesID(recording->SerieID());
  modifier.SetProgramID(recording->ProgramID());
  modifier.SetCategory(recording->Category());
  if (rule.InetRef().empty())
  {
    modifier.SetInerRef(recording->Inetref());
    modifier.SetSeason(recording->Season());
    modifier.SetEpisode(recording->Episode());
  }
  return modifier;
}

MythRecordingRule MythScheduleManager::MakeOverride(const MythRecordingRule& rule, const MythScheduledPtr& recording)
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
  modifier.SetTitle(recording->Title());
  modifier.SetSubtitle(recording->Subtitle());
  modifier.SetDescription(recording->Description());
  modifier.SetChannelID(recording->ChannelID());
  modifier.SetCallsign(recording->Callsign());
  modifier.SetStartTime(recording->StartTime());
  modifier.SetEndTime(recording->EndTime());
  modifier.SetSeriesID(recording->SerieID());
  modifier.SetProgramID(recording->ProgramID());
  modifier.SetCategory(recording->Category());
  if (rule.InetRef().empty())
  {
    modifier.SetInerRef(recording->Inetref());
    modifier.SetSeason(recording->Season());
    modifier.SetEpisode(recording->Episode());
  }
  return modifier;
}

unsigned MythScheduleManager::GetUpcomingCount() const
{
  CLockObject lock(m_lock);
  return (unsigned)m_recordings.size();
}

MythTimerEntryList MythScheduleManager::GetTimerEntries()
{
  CLockObject lock(m_lock);
  MythTimerEntryList entries;

  for (NodeList::iterator it = m_rules.begin(); it != m_rules.end(); ++it)
  {
    if ((*it)->IsOverrideRule())
      continue;
    MythTimerEntryPtr entry = MythTimerEntryPtr(new MythTimerEntry());
    if (m_versionHelper->FillTimerEntry(*entry, **it))
    {
      entry->entryIndex = MakeIndex((*it)->m_rule); // rule index
      entry->parentIndex = 0;
      entries.push_back(entry);
    }
  }

  for (RecordingList::iterator it = m_recordings.begin(); it != m_recordings.end(); ++it)
  {
    //Only include timers which have an inactive status if the user has requested it (flag m_showNotRecording)
    switch (it->second->Status())
    {
      //Upcoming recordings which are disabled due to being lower priority duplicates or already recorded
      case Myth::RS_EARLIER_RECORDING:  //will record earlier
      case Myth::RS_LATER_SHOWING:      //will record later
      case Myth::RS_CURRENT_RECORDING:  //Already in the current library
      case Myth::RS_PREVIOUS_RECORDING: //Previoulsy recorded but no longer in the library
        if (!m_showNotRecording)
        {
          XBMC->Log(LOG_DEBUG, "%s: Skipping %s:%s on %s because status %d and m_showNotRecording=%d", __FUNCTION__,
                    it->second->Title().c_str(), it->second->Subtitle().c_str(), it->second->ChannelName().c_str(), it->second->Status(), m_showNotRecording);
          continue;
        }
      default:
        break;
    }
    // Fill info from recording rule if possible
    MythTimerEntryPtr entry = MythTimerEntryPtr(new MythTimerEntry());
    MythRecordingRuleNodePtr node = FindRuleById(it->second->RecordID());
    if (node)
    {
      if (m_versionHelper->FillTimerEntry(*entry, *node))
      {
        switch (entry->timerType)
        {
          case TIMER_TYPE_UNHANDLED_RULE:
          case TIMER_TYPE_DONT_RECORD:
          case TIMER_TYPE_OVERRIDE:
            break;
          default:
            entry->timerType = TIMER_TYPE_RECORD;
        }
        entry->description = "";
        entry->chanid = it->second->ChannelID();
        entry->callsign = it->second->Callsign();
        entry->startTime = it->second->StartTime();
        entry->endTime = it->second->EndTime();
        entry->title = it->second->Title();
        entry->entryIndex = it->first; // upcoming index
        entry->parentIndex = MakeIndex(node->GetMainRule()); // parent rule index
        entry->recordingStatus = it->second->Status();
        entries.push_back(entry);
      }
    }
  }
  return entries;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::SubmitTimer(const MythTimerEntry& entry)
{
  switch (entry.timerType)
  {
    case TIMER_TYPE_RECORD:
    case TIMER_TYPE_DONT_RECORD:
    case TIMER_TYPE_OVERRIDE:
    case TIMER_TYPE_UNHANDLED_RULE:
      return MSM_ERROR_NOT_IMPLEMENTED;
    default:
      break;
  }
  MythRecordingRule rule = m_versionHelper->NewFromTimer(entry, true);
  MSM_ERROR ret = AddRecordingRule(rule);
  return ret;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::UpdateTimer(const MythTimerEntry& entry)
{
  switch (entry.timerType)
  {
    case TIMER_TYPE_UNHANDLED_RULE:
      break;
    case TIMER_TYPE_RECORD:
    case TIMER_TYPE_DONT_RECORD:
    case TIMER_TYPE_OVERRIDE:
    {
      MythRecordingRule newrule = m_versionHelper->NewFromTimer(entry, false);
      return UpdateRecording(entry.entryIndex, newrule);
    }
    default:
    {
      MythRecordingRule newrule = m_versionHelper->NewFromTimer(entry, false);
      return UpdateRecordingRule(entry.entryIndex, newrule);
    }
  }
  return MSM_ERROR_NOT_IMPLEMENTED;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::DeleteTimer(const MythTimerEntry& entry, bool force)
{
  switch (entry.timerType)
  {
    case TIMER_TYPE_UNHANDLED_RULE:
      break;
    case TIMER_TYPE_RECORD:
      return DisableRecording(entry.entryIndex);
    case TIMER_TYPE_DONT_RECORD:
    case TIMER_TYPE_OVERRIDE:
      return DeleteModifier(entry.entryIndex);
    case TIMER_TYPE_THIS_SHOWING:
    case TIMER_TYPE_ONCE_MANUAL_SEARCH:
      return DeleteRecordingRule(entry.entryIndex);
    default:
      if (force)
        return DeleteRecordingRule(entry.entryIndex);
      return MSM_ERROR_SUCCESS;
  }
  return MSM_ERROR_NOT_IMPLEMENTED;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::DeleteModifier(uint32_t index)
{
  CLockObject lock(m_lock);

  MythScheduledPtr recording = FindUpComingByIndex(index);
  if (!recording)
    return MSM_ERROR_FAILED;
  if (recording->Status() == Myth::RS_RECORDED)
    return MSM_ERROR_FAILED;
  MythRecordingRuleNodePtr node = this->FindRuleById(recording->RecordID());
  if (node && node->IsOverrideRule())
  {
    XBMC->Log(LOG_DEBUG, "%s - Deleting modifier rule %u relates recording %u", __FUNCTION__, node->m_rule.RecordID(), index);
    if (!m_control->RemoveRecordSchedule(node->m_rule.RecordID()))
      XBMC->Log(LOG_ERROR, "%s - Deleting recording rule failed", __FUNCTION__);
  }
  // Another client could delete the rule at the same time. Therefore always SUCCESS even if database delete fails.
  return MSM_ERROR_SUCCESS;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::DisableRecording(uint32_t index)
{
  CLockObject lock(m_lock);

  MythScheduledPtr recording = FindUpComingByIndex(index);
  if (!recording)
    return MSM_ERROR_FAILED;

  if (recording->Status() == Myth::RS_INACTIVE)
    return MSM_ERROR_SUCCESS;

  MythRecordingRuleNodePtr node = this->FindRuleById(recording->RecordID());
  if (node)
  {
    XBMC->Log(LOG_DEBUG, "%s - %u : %s:%s on channel %s program %s",
              __FUNCTION__, index, recording->Title().c_str(), recording->Subtitle().c_str(), recording->Callsign().c_str(), recording->UID().c_str());
    XBMC->Log(LOG_DEBUG, "%s - %u : Found rule %u type %d with recording status %d",
              __FUNCTION__, index, (unsigned)node->m_rule.RecordID(), (int)node->m_rule.Type(), recording->Status());
    int method = METHOD_UNKNOWN;
    MythRecordingRule handle = node->m_rule.DuplicateRecordingRule();

    // Method depends of its rule type
    switch (node->m_rule.Type())
    {
      case Myth::RT_SingleRecord:
        switch (recording->Status())
        {
          case Myth::RS_RECORDING:
          case Myth::RS_TUNING:
            method = METHOD_DELETE;
            break;
          case Myth::RS_PREVIOUS_RECORDING:
          case Myth::RS_EARLIER_RECORDING:
            method = METHOD_CREATE_DONTRECORD;
            break;
          default:
            method = METHOD_UPDATE_INACTIVE;
            break;
        }
        break;

      case Myth::RT_DontRecord:
      case Myth::RT_OverrideRecord:
        method = METHOD_DELETE;
        break;

      case Myth::RT_OneRecord:
      case Myth::RT_ChannelRecord:
      case Myth::RT_AllRecord:
      case Myth::RT_DailyRecord:
      case Myth::RT_WeeklyRecord:
      case Myth::RT_FindDailyRecord:
      case Myth::RT_FindWeeklyRecord:
        method = METHOD_CREATE_DONTRECORD;
        break;

      default:
        method = METHOD_UNKNOWN;
        break;
    }

    XBMC->Log(LOG_DEBUG, "%s - %u : Dealing with the problem using method %d", __FUNCTION__, index, method);
    switch (method)
    {
      case METHOD_UPDATE_INACTIVE:
        handle.SetInactive(true);
        if (!m_control->UpdateRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_rule = handle; // sync node
        return MSM_ERROR_SUCCESS;

      case METHOD_CREATE_DONTRECORD:
        handle = MakeDontRecord(handle, recording);
        XBMC->Log(LOG_DEBUG, "%s - %u : Creating Override for %u (%s: %s) on %u (%s)"
                  , __FUNCTION__, index, (unsigned)handle.ParentID(), handle.Title().c_str(),
                  handle.Subtitle().c_str(), handle.ChannelID(), handle.Callsign().c_str());

        if (!m_control->AddRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_overrideRules.push_back(handle); // sync node
        return MSM_ERROR_SUCCESS;

      case METHOD_DELETE:
        return this->DeleteRecordingRule(handle.RecordID());

      default:
        return MSM_ERROR_NOT_IMPLEMENTED;
    }
  }
  return MSM_ERROR_FAILED;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::EnableRecording(uint32_t index)
{
  CLockObject lock(m_lock);

  MythScheduledPtr recording = this->FindUpComingByIndex(index);
  if (!recording)
    return MSM_ERROR_FAILED;

  MythRecordingRuleNodePtr node = this->FindRuleById(recording->RecordID());
  if (node)
  {
    XBMC->Log(LOG_DEBUG, "%s - %u : %s:%s on channel %s program %s",
              __FUNCTION__, index, recording->Title().c_str(), recording->Subtitle().c_str(), recording->Callsign().c_str(), recording->UID().c_str());
    XBMC->Log(LOG_DEBUG, "%s - %u : Found rule %u type %d disabled by status %d",
              __FUNCTION__, index, (unsigned)node->m_rule.RecordID(), (int)node->m_rule.Type(), recording->Status());
    int method = METHOD_UNKNOWN;
    MythRecordingRule handle = node->m_rule.DuplicateRecordingRule();

    switch (recording->Status())
    {
      case Myth::RS_NEVER_RECORD:
      case Myth::RS_PREVIOUS_RECORDING:
      case Myth::RS_EARLIER_RECORDING:
      case Myth::RS_CURRENT_RECORDING:
        // Add override to record anyway
        method = METHOD_CREATE_OVERRIDE;
        break;

      default:
        // Enable parent rule
        method = METHOD_UPDATE_INACTIVE;
        break;
    }

    XBMC->Log(LOG_DEBUG, "%s - %u : Dealing with the problem using method %d", __FUNCTION__, index, method);
    switch (method)
    {
      case METHOD_UPDATE_INACTIVE:
        handle.SetInactive(false);
        if (!m_control->UpdateRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_rule = handle; // sync node
        return MSM_ERROR_SUCCESS;

      case METHOD_CREATE_OVERRIDE:
        handle = MakeOverride(handle, recording);
        XBMC->Log(LOG_DEBUG, "%s - %u : Creating Override for %u (%s:%s) on %u (%s)"
                  , __FUNCTION__, index, (unsigned)handle.ParentID(), handle.Title().c_str(),
                  handle.Subtitle().c_str(), handle.ChannelID(), handle.Callsign().c_str());

        if (!m_control->AddRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_overrideRules.push_back(handle); // sync node
        return MSM_ERROR_SUCCESS;

      default:
        return MSM_ERROR_NOT_IMPLEMENTED;
    }
  }
  return MSM_ERROR_FAILED;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::UpdateRecording(uint32_t index, MythRecordingRule& newrule)
{
  CLockObject lock(m_lock);

  if (newrule.Type() == Myth::RT_UNKNOWN)
    return MSM_ERROR_FAILED;

  MythScheduledPtr recording = this->FindUpComingByIndex(index);
  if (!recording)
    return MSM_ERROR_FAILED;

  MythRecordingRuleNodePtr node = this->FindRuleById(recording->RecordID());
  if (node)
  {
    XBMC->Log(LOG_DEBUG, "%s - %u : Found rule %u type %d and recording status %d",
              __FUNCTION__, index, (unsigned)node->m_rule.RecordID(), (int)node->m_rule.Type(), recording->Status());

    // Prior handle inactive
    if (!node->m_rule.Inactive() && newrule.Inactive())
    {
      XBMC->Log(LOG_DEBUG, "%s - Disable recording", __FUNCTION__);
      return DisableRecording(index);
    }

    int method = METHOD_UNKNOWN;
    MythRecordingRule handle = node->m_rule.DuplicateRecordingRule();

    switch (node->m_rule.Type())
    {
      case Myth::RT_NotRecording:
      case Myth::RT_TemplateRecord:
        method = METHOD_UNKNOWN;
        break;

      case Myth::RT_DontRecord:
        method = METHOD_NOOP;
        break;

      case Myth::RT_OverrideRecord:
        method = METHOD_DISCREET_UPDATE;
        handle.SetInactive(newrule.Inactive());
        handle.SetPriority(newrule.Priority());
        handle.SetAutoExpire(newrule.AutoExpire());
        handle.SetStartOffset(newrule.StartOffset());
        handle.SetEndOffset(newrule.EndOffset());
        handle.SetRecordingGroup(newrule.RecordingGroup());
        break;

      case Myth::RT_SingleRecord:
        switch (recording->Status())
        {
          case Myth::RS_RECORDING:
          case Myth::RS_TUNING:
            method = METHOD_DISCREET_UPDATE;
            handle.SetEndTime(newrule.EndTime());
            handle.SetEndOffset(newrule.EndOffset());
            break;
          case Myth::RS_NEVER_RECORD:
          case Myth::RS_PREVIOUS_RECORDING:
          case Myth::RS_EARLIER_RECORDING:
          case Myth::RS_CURRENT_RECORDING:
            // Add override to record anyway
            method = METHOD_CREATE_OVERRIDE;
            handle.SetPriority(newrule.Priority());
            handle.SetAutoExpire(newrule.AutoExpire());
            handle.SetStartOffset(newrule.StartOffset());
            handle.SetEndOffset(newrule.EndOffset());
            handle.SetRecordingGroup(newrule.RecordingGroup());
            break;
          default:
            method = METHOD_FULL_UPDATE;
          break;
        }
        break;

      default:
        method = METHOD_CREATE_OVERRIDE;
        handle.SetPriority(newrule.Priority());
        handle.SetAutoExpire(newrule.AutoExpire());
        handle.SetStartOffset(newrule.StartOffset());
        handle.SetEndOffset(newrule.EndOffset());
        handle.SetRecordingGroup(newrule.RecordingGroup());
        break;
    }

    XBMC->Log(LOG_DEBUG, "%s - %u : Dealing with the problem using method %d", __FUNCTION__, index, method);
    switch (method)
    {
      case METHOD_NOOP:
        return MSM_ERROR_SUCCESS;

      case METHOD_DISCREET_UPDATE:
        if (!m_control->UpdateRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_rule = handle; // sync node
        return MSM_ERROR_SUCCESS;

      case METHOD_CREATE_OVERRIDE:
        handle = MakeOverride(handle, recording);
        XBMC->Log(LOG_DEBUG, "%s - %u : Creating Override for %u (%s: %s) on %u (%s)"
                  , __FUNCTION__, index, (unsigned)node->m_rule.RecordID(), node->m_rule.Title().c_str(),
                  node->m_rule.Subtitle().c_str(), recording->ChannelID(), recording->Callsign().c_str());

        if (!m_control->AddRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_overrideRules.push_back(handle); // sync node
        //if (!m_con.UpdateSchedules(handle.RecordID()))
        //  return MSM_ERROR_FAILED;
        return MSM_ERROR_SUCCESS;

      case METHOD_FULL_UPDATE:
        handle = newrule;
        handle.SetRecordID(node->m_rule.RecordID());
        if (!m_control->UpdateRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_rule = handle; // sync node
        return MSM_ERROR_SUCCESS;

      default:
        return MSM_ERROR_NOT_IMPLEMENTED;
    }
  }
  return MSM_ERROR_FAILED;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::AddRecordingRule(MythRecordingRule &rule)
{
  if (rule.Type() == Myth::RT_UNKNOWN || rule.Type() == Myth::RT_NotRecording)
    return MSM_ERROR_FAILED;
  if (!m_control->AddRecordSchedule(*(rule.GetPtr())))
    return MSM_ERROR_FAILED;
  return MSM_ERROR_SUCCESS;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::DeleteRecordingRule(uint32_t index)
{
  CLockObject lock(m_lock);

  MythRecordingRuleNodePtr node = FindRuleByIndex(index);
  if (node)
  {
    XBMC->Log(LOG_DEBUG, "%s - Found rule %u type %d", __FUNCTION__, (unsigned)node->m_rule.RecordID(), (int)node->m_rule.Type());

    // Delete overrides and their related recording
    if (node->HasOverrideRules())
    {
      for (MythRecordingRuleList::iterator ito = node->m_overrideRules.begin(); ito != node->m_overrideRules.end(); ++ito)
      {
        XBMC->Log(LOG_DEBUG, "%s - Found override rule %u type %d", __FUNCTION__, (unsigned)ito->RecordID(), (int)ito->Type());
        MythScheduleList rec = this->FindUpComingByRuleId(ito->RecordID());
        for (MythScheduleList::iterator itr = rec.begin(); itr != rec.end(); ++itr)
        {
          XBMC->Log(LOG_DEBUG, "%s - Found overriden recording %s status %d", __FUNCTION__, itr->second->UID().c_str(), itr->second->Status());
          if (itr->second->Status() == Myth::RS_RECORDING || itr->second->Status() == Myth::RS_TUNING)
          {
            XBMC->Log(LOG_DEBUG, "%s - Stop recording %s", __FUNCTION__, itr->second->UID().c_str());
            m_control->StopRecording(*(itr->second->GetPtr()));
          }
        }
        XBMC->Log(LOG_DEBUG, "%s - Deleting recording rule %u (modifier of rule %u)", __FUNCTION__, (unsigned)ito->RecordID(), (unsigned)node->m_rule.RecordID());
        if (!m_control->RemoveRecordSchedule(ito->RecordID()))
          XBMC->Log(LOG_ERROR, "%s - Deleting recording rule failed", __FUNCTION__);
      }
    }
    // Delete related recordings
    MythScheduleList rec = FindUpComingByRuleId(node->m_rule.RecordID());
    for (MythScheduleList::iterator itr = rec.begin(); itr != rec.end(); ++itr)
    {
      XBMC->Log(LOG_DEBUG, "%s - Found recording %s status %d", __FUNCTION__, itr->second->UID().c_str(), itr->second->Status());
      if (itr->second->Status() == Myth::RS_RECORDING || itr->second->Status() == Myth::RS_TUNING)
      {
        XBMC->Log(LOG_DEBUG, "%s - Stop recording %s", __FUNCTION__, itr->second->UID().c_str());
        m_control->StopRecording(*(itr->second->GetPtr()));
      }
    }
    // Delete rule
    XBMC->Log(LOG_DEBUG, "%s - Deleting recording rule %u", __FUNCTION__, node->m_rule.RecordID());
    if (!m_control->RemoveRecordSchedule(node->m_rule.RecordID()))
      XBMC->Log(LOG_ERROR, "%s - Deleting recording rule failed", __FUNCTION__);
  }
  // Another client could delete the rule at the same time. Therefore always SUCCESS even if database delete fails.
  return MSM_ERROR_SUCCESS;
}

MythScheduleManager::MSM_ERROR MythScheduleManager::UpdateRecordingRule(uint32_t index, MythRecordingRule& newrule)
{
  CLockObject lock(m_lock);

  if (newrule.Type() == Myth::RT_UNKNOWN)
    return MSM_ERROR_FAILED;

  MythRecordingRuleNodePtr node = this->FindRuleByIndex(index);
  if (node)
  {
    XBMC->Log(LOG_DEBUG, "%s - Found rule %u type %d",
              __FUNCTION__, (unsigned)node->m_rule.RecordID(), (int)node->m_rule.Type());
    int method = METHOD_UNKNOWN;
    MythRecordingRule handle = node->m_rule.DuplicateRecordingRule();

    switch (node->m_rule.Type())
    {
      case Myth::RT_NotRecording:
      case Myth::RT_TemplateRecord:
        method = METHOD_UNKNOWN;
        break;

      case Myth::RT_DontRecord:
        method = METHOD_NOOP;
        break;

      case Myth::RT_OverrideRecord:
        method = METHOD_DISCREET_UPDATE;
        handle.SetInactive(newrule.Inactive());
        handle.SetPriority(newrule.Priority());
        handle.SetAutoExpire(newrule.AutoExpire());
        handle.SetStartOffset(newrule.StartOffset());
        handle.SetEndOffset(newrule.EndOffset());
        handle.SetRecordingGroup(newrule.RecordingGroup());
        break;

      case Myth::RT_SingleRecord:
        {
          MythScheduleList recordings = FindUpComingByRuleId(handle.RecordID());
          MythScheduleList::const_reverse_iterator it = recordings.rbegin();
          if (it != recordings.rend())
            return UpdateRecording(MakeIndex(*(it->second)), newrule);
          method = METHOD_UNKNOWN;
        }
        break;

      default:
        // When inactive we can replace with the new rule
        if (handle.Inactive() || newrule.Inactive())
          method = METHOD_FULL_UPDATE;
        else
        {
          method = METHOD_DISCREET_UPDATE;
          handle.SetPriority(newrule.Priority());
          handle.SetAutoExpire(newrule.AutoExpire());
          handle.SetStartOffset(newrule.StartOffset());
          handle.SetEndOffset(newrule.EndOffset());
          handle.SetRecordingGroup(newrule.RecordingGroup());
          handle.SetCheckDuplicatesInType(newrule.CheckDuplicatesInType());
          handle.SetDuplicateControlMethod(newrule.DuplicateControlMethod());
        }
        break;
    }

    XBMC->Log(LOG_DEBUG, "%s - Dealing with the problem using method %d", __FUNCTION__, method);
    switch (method)
    {
      case METHOD_NOOP:
        return MSM_ERROR_SUCCESS;

      case METHOD_DISCREET_UPDATE:
        if (!m_control->UpdateRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_rule = handle; // sync node
        return MSM_ERROR_SUCCESS;

      case METHOD_FULL_UPDATE:
        handle = newrule;
        handle.SetRecordID(node->m_rule.RecordID());
        if (!m_control->UpdateRecordSchedule(*(handle.GetPtr())))
          return MSM_ERROR_FAILED;
        node->m_rule = handle; // sync node
        return MSM_ERROR_SUCCESS;

      default:
        return MSM_ERROR_NOT_IMPLEMENTED;
    }
  }
  return MSM_ERROR_FAILED;
}

MythRecordingRuleNodePtr MythScheduleManager::FindRuleById(uint32_t recordid) const
{
  CLockObject lock(m_lock);

  NodeById::const_iterator it = m_rulesById.find(recordid);
  if (it != m_rulesById.end())
    return it->second;
  return MythRecordingRuleNodePtr();
}

MythRecordingRuleNodePtr MythScheduleManager::FindRuleByIndex(uint32_t index) const
{
  CLockObject lock(m_lock);

  NodeByIndex::const_iterator it = m_rulesByIndex.find(index);
  if (it != m_rulesByIndex.end())
    return it->second;
  return MythRecordingRuleNodePtr();
}

MythScheduleList MythScheduleManager::FindUpComingByRuleId(uint32_t recordid) const
{
  CLockObject lock(m_lock);

  MythScheduleList found;
  std::pair<RecordingIndexByRuleId::const_iterator, RecordingIndexByRuleId::const_iterator> range = m_recordingIndexByRuleId.equal_range(recordid);
  if (range.first != m_recordingIndexByRuleId.end())
  {
    for (RecordingIndexByRuleId::const_iterator it = range.first; it != range.second; ++it)
    {
      RecordingList::const_iterator recordingIt = m_recordings.find(it->second);
      if (recordingIt != m_recordings.end())
        found.push_back(std::make_pair(it->second, recordingIt->second));
    }
  }
  return found;
}

MythScheduledPtr MythScheduleManager::FindUpComingByIndex(uint32_t index) const
{
  CLockObject lock(m_lock);

  RecordingList::const_iterator it = m_recordings.find(index);
  if (it != m_recordings.end())
    return it->second;
  return MythScheduledPtr();
}

bool MythScheduleManager::OpenControl()
{
  if (m_control)
    return m_control->Open();
  return false;
}

void MythScheduleManager::CloseControl()
{
  if (m_control)
    m_control->Close();
}

void MythScheduleManager::Update()
{
  CLockObject lock(m_lock);

  // Setup VersionHelper for the new set
  this->Setup();
  Myth::RecordScheduleListPtr records = m_control->GetRecordScheduleList();
  m_rules.clear();
  m_rulesById.clear();
  m_rulesByIndex.clear();
  m_templates.clear();
  for (Myth::RecordScheduleList::iterator it = records->begin(); it != records->end(); ++it)
  {
    MythRecordingRule rule(*it);
    if (rule.Type() == Myth::RT_TemplateRecord)
    {
      m_templates.push_back(rule);
    }
    else
    {
      MythRecordingRuleNodePtr node = MythRecordingRuleNodePtr(new MythRecordingRuleNode(rule));
      m_rules.push_back(node);
      m_rulesById.insert(NodeById::value_type(rule.RecordID(), node));
      m_rulesByIndex.insert(NodeByIndex::value_type(MakeIndex(rule), node));
    }
  }

  for (NodeList::iterator it = m_rules.begin(); it != m_rules.end(); ++it)
  {
    // Is override rule ? Then find main rule and link to it
    if ((*it)->IsOverrideRule())
    {
      // First check parentid. Then fallback searching the same timeslot
      NodeById::iterator itp = m_rulesById.find((*it)->m_rule.ParentID());
      if (itp != m_rulesById.end())
      {
        itp->second->m_overrideRules.push_back((*it)->m_rule);
        (*it)->m_mainRule = itp->second->m_rule;
      }
      else
      {
        for (NodeList::iterator itm = m_rules.begin(); itm != m_rules.end(); ++itm)
          if (!(*itm)->IsOverrideRule() && m_versionHelper->SameTimeslot((*it)->m_rule, (*itm)->m_rule))
          {
            (*itm)->m_overrideRules.push_back((*it)->m_rule);
            (*it)->m_mainRule = (*itm)->m_rule;
          }

      }
    }
  }

  m_recordings.clear();
  m_recordingIndexByRuleId.clear();
  // Add upcoming recordings
  Myth::ProgramListPtr recordings = m_control->GetUpcomingList();
  for (Myth::ProgramList::iterator it = recordings->begin(); it != recordings->end(); ++it)
  {
    MythScheduledPtr scheduled = MythScheduledPtr(new MythProgramInfo(*it));
    uint32_t index = MakeIndex(*scheduled);
    m_recordings.insert(RecordingList::value_type(index, scheduled));
    m_recordingIndexByRuleId.insert(RecordingIndexByRuleId::value_type(scheduled->RecordID(), index));
  }

  if (g_bExtraDebug)
  {
    for (NodeList::iterator it = m_rules.begin(); it != m_rules.end(); ++it)
      XBMC->Log(LOG_DEBUG, "%s - Rule node - recordid: %u, parentid: %u, type: %d, overriden: %s", __FUNCTION__,
              (unsigned)(*it)->m_rule.RecordID(), (unsigned)(*it)->m_rule.ParentID(),
              (int)(*it)->m_rule.Type(), ((*it)->HasOverrideRules() ? "Yes" : "No"));
    for (RecordingList::iterator it = m_recordings.begin(); it != m_recordings.end(); ++it)
      XBMC->Log(LOG_DEBUG, "%s - Recording - recordid: %u, index: %u, status: %d, title: %s", __FUNCTION__,
              (unsigned)it->second->RecordID(), (unsigned)it->first, it->second->Status(), it->second->Title().c_str());
  }
}

MythScheduleManager::TimerType::TimerType(TimerTypeId id, unsigned attributes, const std::string& description,
            const RulePriorityList& priorityList, int priorityDefault,
            const RuleDupMethodList& dupMethodList, int dupMethodDefault,
            const RuleExpirationList& expirationList, int expirationDefault,
            const RuleRecordingGroupList& recGroupList, int recGroupDefault)
{
  iId = id;
  iAttributes = attributes;
  PVR_STRCPY(strDescription, description.c_str());
  // Initialize priorities
  iPrioritiesSize = priorityList.size();
  assert(iPrioritiesSize <= PVR_ADDON_TIMERTYPE_VALUES_ARRAY_SIZE);
  unsigned index = 0;
  for (MythScheduleManager::RulePriorityList::const_iterator it = priorityList.begin(); it != priorityList.end(); ++it, ++index)
  {
    priorities[index].iValue = it->first;
    PVR_STRCPY(priorities[index].strDescription, it->second.c_str());
  }
  iPrioritiesDefault = priorityDefault;
  // Initialize duplicate methodes
  iPreventDuplicateEpisodesSize = dupMethodList.size();
  assert(iPreventDuplicateEpisodesSize <= PVR_ADDON_TIMERTYPE_VALUES_ARRAY_SIZE);
  index = 0;
  for (MythScheduleManager::RuleDupMethodList::const_iterator it = dupMethodList.begin(); it != dupMethodList.end(); ++it, ++index)
  {
    preventDuplicateEpisodes[index].iValue = it->first;
    PVR_STRCPY(preventDuplicateEpisodes[index].strDescription, XBMC->GetLocalizedString(it->second));
  }
  iPreventDuplicateEpisodesDefault = dupMethodDefault;
  // Initialize expirations
  iLifetimesSize = expirationList.size();
  assert(iLifetimesSize <= PVR_ADDON_TIMERTYPE_VALUES_ARRAY_SIZE);
  index = 0;
  for (MythScheduleManager::RuleExpirationList::const_iterator it = expirationList.begin(); it != expirationList.end(); ++it, ++index)
  {
    lifetimes[index].iValue = it->first;
    PVR_STRCPY(lifetimes[index].strDescription, XBMC->GetLocalizedString(it->second));
  }
  iLifetimesDefault = expirationDefault;
  // Initialize recording groups
  iRecordingGroupSize = recGroupList.size();
  assert(iRecordingGroupSize <= PVR_ADDON_TIMERTYPE_VALUES_ARRAY_SIZE);
  index = 0;
  for (MythScheduleManager::RuleRecordingGroupList::const_iterator it = recGroupList.begin(); it != recGroupList.end(); ++it, ++index)
  {
    recordingGroup[index].iValue = it->first;
    PVR_STRCPY(recordingGroup[index].strDescription, it->second.c_str());
  }
  iRecordingGroupDefault = recGroupDefault;
}

const std::vector<MythScheduleManager::TimerType>& MythScheduleManager::GetTimerTypes()
{
  return m_versionHelper->GetTimerTypes();
}

const MythScheduleManager::RulePriorityList& MythScheduleManager::GetRulePriorityList()
{
  return m_versionHelper->GetRulePriorityList();
}

int MythScheduleManager::GetRulePriorityDefault()
{
  return m_versionHelper->GetRulePriorityDefault();
}

const MythScheduleManager::RuleDupMethodList& MythScheduleManager::GetRuleDupMethodList()
{
  return m_versionHelper->GetRuleDupMethodList();
}

int MythScheduleManager::GetRuleDupMethodDefault()
{
  return m_versionHelper->GetRuleDupMethodDefault();
}

const MythScheduleManager::RuleExpirationList& MythScheduleManager::GetRuleExpirationList()
{
  return m_versionHelper->GetRuleExpirationList();
}

int MythScheduleManager::GetRuleExpirationDefault()
{
  return m_versionHelper->GetRuleExpirationDefault();
}

const MythScheduleManager::RuleRecordingGroupList& MythScheduleManager::GetRuleRecordingGroupList()
{
  return m_versionHelper->GetRuleRecordingGroupList();
}

int MythScheduleManager::GetRuleRecordingGroupDefault()
{
  return m_versionHelper->GetRuleRecordingGroupDefault();
}

MythRecordingRule MythScheduleManager::NewFromTimer(const MythTimerEntry& entry, bool withTemplate)
{
  return m_versionHelper->NewFromTimer(entry, withTemplate);
}

MythScheduleManager::RuleSummaryInfo MythScheduleManager::GetSummaryInfo(const MythRecordingRule &rule) const
{
  return m_versionHelper->GetSummaryInfo(rule);
}

MythRecordingRule MythScheduleManager::NewSingleRecord(const MythEPGInfo& epgInfo)
{
  return m_versionHelper->NewSingleRecord(epgInfo);
}

MythRecordingRule MythScheduleManager::NewDailyRecord(const MythEPGInfo& epgInfo)
{
  return m_versionHelper->NewDailyRecord(epgInfo);
}

MythRecordingRule MythScheduleManager::NewWeeklyRecord(const MythEPGInfo& epgInfo)
{
  return m_versionHelper->NewWeeklyRecord(epgInfo);
}

MythRecordingRule MythScheduleManager::NewChannelRecord(const MythEPGInfo& epgInfo)
{
  return m_versionHelper->NewChannelRecord(epgInfo);
}

MythRecordingRule MythScheduleManager::NewOneRecord(const MythEPGInfo& epgInfo)
{
  return m_versionHelper->NewOneRecord(epgInfo);
}

MythRecordingRuleList MythScheduleManager::GetTemplateRules() const
{
  return m_templates;
}

bool MythScheduleManager::ToggleShowNotRecording()
{
  m_showNotRecording ^= true;
  return m_showNotRecording;
}

///////////////////////////////////////////////////////////////////////////////
////
//// Version Helper for unknown version (no helper)
////

const std::vector<MythScheduleManager::TimerType>& MythScheduleHelperNoHelper::GetTimerTypes() const
{
  static std::vector<MythScheduleManager::TimerType> typeList;
  return typeList;
}

const MythScheduleManager::RulePriorityList& MythScheduleHelperNoHelper::GetRulePriorityList() const
{
  static bool _init = false;
  static MythScheduleManager::RulePriorityList _list;
  if (!_init)
  {
    _init = true;
    _list.push_back(std::make_pair(0, "0"));
  }
  return _list;
}

const MythScheduleManager::RuleDupMethodList& MythScheduleHelperNoHelper::GetRuleDupMethodList() const
{
  static bool _init = false;
  static MythScheduleManager::RuleDupMethodList _list;
  if (!_init)
  {
    _init = true;
    _list.push_back(std::make_pair(Myth::DM_CheckNone, 30501));
  }
  return _list;
}

const MythScheduleManager::RuleExpirationList& MythScheduleHelperNoHelper::GetRuleExpirationList() const
{
  static bool _init = false;
  static MythScheduleManager::RuleExpirationList _list;
  if (!_init)
  {
    _init = true;
    _list.push_back(std::make_pair(0, 30506));
  }
  return _list;
}

const MythScheduleManager::RuleRecordingGroupList& MythScheduleHelperNoHelper::GetRuleRecordingGroupList() const
{
  static bool _init = false;
  static MythScheduleManager::RuleRecordingGroupList _list;
  if (!_init)
  {
    _init = true;
    _list.push_back(std::make_pair(RECGROUP_ID_NONE, "?"));
  }
  return _list;
}

int MythScheduleHelperNoHelper::GetRuleRecordingGroupId(const std::string& name) const
{
  static bool _init = false;
  static std::map<std::string, int> _map;
  if (!_init)
  {
    _init = true;
    const MythScheduleManager::RuleRecordingGroupList& groupList = GetRuleRecordingGroupList();
    for (MythScheduleManager::RuleRecordingGroupList::const_iterator it = groupList.begin(); it != groupList.end(); ++it)
      _map.insert(std::make_pair(it->second, it->first));
  }
  std::map<std::string, int>::const_iterator it = _map.find(name);
  if (it != _map.end())
    return it->second;
  return RECGROUP_ID_NONE;
}

const std::string& MythScheduleHelperNoHelper::GetRuleRecordingGroupName(int id) const
{
  static bool _init = false;
  static std::map<int, std::string> _map;
  static std::string _empty = "";
  if (!_init)
  {
    _init = true;
    const MythScheduleManager::RuleRecordingGroupList& groupList = GetRuleRecordingGroupList();
    for (MythScheduleManager::RuleRecordingGroupList::const_iterator it = groupList.begin(); it != groupList.end(); ++it)
    {
      // Don't map id for none: Returned value must to be nil
      if (it->first != RECGROUP_ID_NONE)
        _map.insert(std::make_pair(it->first, it->second));
    }
  }
  std::map<int, std::string>::const_iterator it = _map.find(id);
  if (it != _map.end())
    return it->second;
  return _empty;
}

int MythScheduleHelperNoHelper::GetRuleRecordingGroupDefault() const
{
  static bool _init = false;
  static int _val = 0;
  if (!_init)
  {
    _init = true;
    _val = GetRuleRecordingGroupId(RECGROUP_DFLT_NAME);
  }
  return _val;
}

MythRecordingRule MythScheduleHelperNoHelper::NewFromTimer(const MythTimerEntry& entry, bool withTemplate)
{
  (void)entry;
  (void)withTemplate;
  return MythRecordingRule();
}

bool MythScheduleHelperNoHelper::SameTimeslot(const MythRecordingRule &first, const MythRecordingRule &second) const
{
  (void)first;
  (void)second;
  return false;
}

MythScheduleManager::RuleSummaryInfo MythScheduleHelperNoHelper::GetSummaryInfo(const MythRecordingRule &rule) const
{
  MythScheduleManager::RuleSummaryInfo meta;
  (void)rule;
  meta.isRepeating = false;
  meta.weekDays = 0;
  meta.marker = "";
  return meta;
}

bool MythScheduleHelperNoHelper::FillTimerEntry(MythTimerEntry& entry, const MythRecordingRuleNode& node) const
{
  (void)node;
  entry.timerType = TIMER_TYPE_UNHANDLED_RULE;
  return true;
}


MythRecordingRule MythScheduleHelperNoHelper::NewFromTemplate(const MythEPGInfo& epgInfo)
{
  (void)epgInfo;
  return MythRecordingRule();
}

MythRecordingRule MythScheduleHelperNoHelper::NewSingleRecord(const MythEPGInfo& epgInfo)
{
  (void)epgInfo;
  return MythRecordingRule();
}

MythRecordingRule MythScheduleHelperNoHelper::NewDailyRecord(const MythEPGInfo& epgInfo)
{
  (void)epgInfo;
  return MythRecordingRule();
}

MythRecordingRule MythScheduleHelperNoHelper::NewWeeklyRecord(const MythEPGInfo& epgInfo)
{
  (void)epgInfo;
  return MythRecordingRule();
}

MythRecordingRule MythScheduleHelperNoHelper::NewChannelRecord(const MythEPGInfo& epgInfo)
{
  (void)epgInfo;
  return MythRecordingRule();
}

MythRecordingRule MythScheduleHelperNoHelper::NewOneRecord(const MythEPGInfo& epgInfo)
{
  (void)epgInfo;
  return MythRecordingRule();
}

///////////////////////////////////////////////////////////////////////////////
////
//// Version helper for backend version 75 (0.26)
////

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
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
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
            PVR_TIMER_TYPE_SUPPORTS_ENABLE_DISABLE |
            PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
            PVR_TIMER_TYPE_SUPPORTS_START_END_TIME |
            PVR_TIMER_TYPE_SUPPORTS_START_END_MARGIN |
            PVR_TIMER_TYPE_SUPPORTS_PRIORITY |
            PVR_TIMER_TYPE_SUPPORTS_LIFETIME |
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH,
            XBMC->GetLocalizedString(30461),
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
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH,
            XBMC->GetLocalizedString(30462),
            GetRulePriorityList(),
            GetRulePriorityDefault(),
            GetRuleDupMethodList(),
            GetRuleDupMethodDefault(),
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
            PVR_TIMER_TYPE_SUPPORTS_RECORDING_GROUP |
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH,
            XBMC->GetLocalizedString(30463),
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
            PVR_TIMER_TYPE_SUPPORTS_RECORD_ONLY_NEW_EPISODES |
            PVR_TIMER_TYPE_SUPPORTS_TITLE_EPG_MATCH,
            XBMC->GetLocalizedString(30464),
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
    for (int i = -99; i <= 99; ++i)
    {
      if (i)
      {
        snprintf(buf, sizeof(buf), "%+2d", i);
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
  switch (rule.Type())
  {
    case Myth::RT_DailyRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING_DAILY;
      break;
    case Myth::RT_WeeklyRecord:
      entry.timerType = TIMER_TYPE_ONE_SHOWING_WEEKLY;
      break;
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
  entry.startTime = rule.StartTime();
  entry.endTime = rule.EndTime();
  entry.title = rule.Title();
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
        rule.SetType(Myth::RT_ChannelRecord);
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
        rule.SetType(Myth::RT_ChannelRecord);
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

bool MythScheduleHelper76::FillTimerEntry(MythTimerEntry& entry, const MythRecordingRuleNode& node) const
{
  MythRecordingRule rule = node.GetRule();
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
        entry.timerType = TIMER_TYPE_ALL_SHOWINGS;
        if (rule.SearchType() == Myth::ST_NoSearch)
          entry.epgSearch = rule.Title(); // EPG based
      }
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
  entry.startTime = rule.StartTime();
  entry.endTime = rule.EndTime();
  entry.title = rule.Title();
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
      if (!entry.epgSearch.empty())
      {
        rule.SetType(Myth::RT_WeeklyRecord);
        rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        if (entry.HasChannel())
        {
          rule.SetFilter(Myth::FM_ThisChannel);
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
          rule.SetFilter(Myth::FM_ThisChannel);
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
          rule.SetFilter(Myth::FM_ThisChannel);
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
        rule.SetType(Myth::RT_AllRecord);
        rule.SetFilter(Myth::FM_ThisChannel);
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
        rule.SetType(Myth::RT_AllRecord);
        rule.SetSearchType(Myth::ST_TitleSearch); // Search title
        if (entry.HasChannel())
        {
          rule.SetFilter(Myth::FM_ThisChannel);
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

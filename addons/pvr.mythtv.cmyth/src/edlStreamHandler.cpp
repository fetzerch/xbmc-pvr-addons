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

#include "edlStreamHandler.h"

#include "client.h"

using namespace ADDON;

void EdlStreamHandler::ResetCutList()
{
  m_CutListSize = 0;
  m_Cut = NULL;
  m_IgnoredCut = NULL;
}

bool EdlStreamHandler::AddCutEntry(unsigned long long start, unsigned long long end)
{
  if (m_CutListSize < FILE_CUT_LIST_SIZE)
  {
    m_CutList[m_CutListSize].start = start;
    m_CutList[m_CutListSize].end = end;
    m_CutList[m_CutListSize].isIgnored = false;
    PinCutAround(&m_CutList[m_CutListSize]);
    m_CutListSize++;
    return true;
  }
  else
    return false;
}

void EdlStreamHandler::LoadCutList(MythConnection &conn, MythDatabase &db, MythProgramInfo &recording)
{
  unsigned long long startOffset = 0;
  unsigned long long endOffset = 0;
  long long psoffset;
  long long nsoffset;
  int mask = 0;
  Edl::const_iterator edlIt;
  this->ResetCutList();

  Edl commbreakList = conn.GetCommbreakList(recording);
  XBMC->Log(LOG_DEBUG, "%s - Found %d commercial breaks for: %s", __FUNCTION__, commbreakList.size(), recording.Title().c_str());

  Edl cutList = conn.GetCutList(recording);
  XBMC->Log(LOG_DEBUG, "%s - Found %d cut list entries for: %s", __FUNCTION__, cutList.size(), recording.Title().c_str());

  commbreakList.insert(commbreakList.end(), cutList.begin(), cutList.end());
  for (edlIt = commbreakList.begin(); edlIt != commbreakList.end(); ++edlIt)
  {
    if ((mask = db.GetRecordingSeekOffset(recording, edlIt->start_mark, &psoffset, &nsoffset)) > 0)
    {
      switch (mask)
      {
        case 2:
          startOffset = (unsigned long long)nsoffset; // Next frame
          break;
        default:
          startOffset = (unsigned long long)psoffset; // Choose to start on previous frame
      }
      if ((mask = db.GetRecordingSeekOffset(recording, edlIt->end_mark, &psoffset, &nsoffset)) > 0)
      {
        switch (mask)
        {
          case 1:
            endOffset = (unsigned long long)psoffset; // Previous frame
            break;
          default:
            endOffset = (unsigned long long)nsoffset; // Choose to end on next frame
        }
        if (endOffset > startOffset)
        {
          if (!this->AddCutEntry(startOffset, endOffset))
          {
            XBMC->Log(LOG_ERROR, "%s - Maximum number of EDL entries reached for: %s", __FUNCTION__, recording.Title().c_str());
            break;
          }
          XBMC->Log(LOG_DEBUG, "%s - Add cut entry to file: %llu - %llu", __FUNCTION__, startOffset, endOffset);
        }
        else
          XBMC->Log(LOG_NOTICE, "%s - Reject cut entry: %llu - %llu", __FUNCTION__, startOffset, endOffset);
      }
    }
  }
}

int EdlStreamHandler::Read(void *buffer, unsigned int length)
{
  int bytesRead;
  if (m_CutListSize > 0)
  {
    while (m_Cut != NULL && this->Position() >= m_Cut->start)
    {
      XBMC->Log(LOG_DEBUG,"%s - Seek >>> cut: %llu", __FUNCTION__, m_Cut->end);
      if (this->SeekOverCut(m_Cut->end, WHENCE_SET) < 0)
        break;
    }
    if (m_Cut != NULL && (unsigned long long)(this->Position() + length) > m_Cut->start)
    {
      length = (unsigned int)(m_Cut->start - this->Position());
      XBMC->Log(LOG_DEBUG,"%s - Read (%u) --> cut: %llu", __FUNCTION__, length, m_Cut->start);
    }
  }
  bytesRead = cmyth_file_read(m_myth_file, static_cast< char * >(buffer), length);
  return bytesRead;
}

long long EdlStreamHandler::Seek(long long offset, int whence)
{
  long long retval = 0;
  if (m_CutListSize > 0)
  {
    unsigned long long ppos = this->Position();
    retval = this->SeekOverCut(offset, whence);
    while (retval >= 0 && m_Cut != NULL && this->Position() > m_Cut->start)
    {
      if (this->Position() < ppos) {
        // On skip back, seek over:
        //   if ((offset = (long long)(this->Position() + m_Cut->start - m_Cut->end)) < 0)
        //     offset = 0;
        //   XBMC->Log(LOG_DEBUG,"%s - Seek <<< cut: %lld", __FUNCTION__, offset);
        // On skip back, ignore cut:
        XBMC->Log(LOG_DEBUG,"%s - Seek back! Ignore cut: %llu - %llu", __FUNCTION__, m_Cut->start, m_Cut->end);
        m_Cut->isIgnored = true;
        m_IgnoredCut = m_Cut;
        this->FindPinCutAround();
        break;
      }
      else
      {
        if ((offset = (long long)(this->Position() + m_Cut->end - m_Cut->start)) > (long long)this->Length())
          offset = (long long)this->Length();
        XBMC->Log(LOG_DEBUG,"%s - Seek >>> cut: %lld", __FUNCTION__, offset);
      }
      retval = this->SeekOverCut(offset, WHENCE_SET);
    }
  }
  else
    retval = cmyth_file_seek(m_myth_file, offset, whence);
  return retval;
}

void EdlStreamHandler::PinCutAround(FILE_CUT_ENTRY *cutEntry)
{
  if (!cutEntry->isIgnored)
  {
    if (cutEntry->end > this->Position() && (m_Cut == NULL || cutEntry->start < m_Cut->start))
    {
      m_Cut = cutEntry;
    }
  }
}

void EdlStreamHandler::FindPinCutAround()
{
  m_Cut = NULL;
  for (int i = 0; i < m_CutListSize; i++)
    PinCutAround(&m_CutList[i]);
}

long long EdlStreamHandler::SeekOverCut(long long offset, int whence)
{
  long long retval = 0;
  unsigned long long pos;
  if ((retval = cmyth_file_seek(m_myth_file, offset, whence)) >= 0)
  {
    pos = (unsigned long long)retval;
    if (m_IgnoredCut != NULL)
    {
      if (pos < m_IgnoredCut->start || pos >= m_IgnoredCut->end)
        m_IgnoredCut->isIgnored = false;
      else
        m_IgnoredCut->isIgnored = true;
    }
    this->FindPinCutAround();
  }
  return retval;
}

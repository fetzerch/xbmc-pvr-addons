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

#include "cppmyth/MythFile.h"
#include "cppmyth/MythDatabase.h"
#include "cppmyth/MythProgramInfo.h"

#include <boost/shared_ptr.hpp>

#define FILE_CUT_LIST_SIZE    32

class EdlStreamHandler : public BasicStreamHandler
{
public:
  void ResetCutList();
  bool AddCutEntry(unsigned long long start, unsigned long long end);
  void LoadCutList(MythConnection &conn, MythDatabase &db, MythProgramInfo &recording);

protected:
  virtual int Read(void *buffer, unsigned int length);
  virtual long long Seek(long long offset, int whence);

private:
  typedef struct
  {
    unsigned long long start;
    unsigned long long end;
    bool isIgnored;
  } FILE_CUT_ENTRY;

  FILE_CUT_ENTRY m_CutList[FILE_CUT_LIST_SIZE];
  int m_CutListSize;
  FILE_CUT_ENTRY *m_Cut; // The handled cut for current position
  FILE_CUT_ENTRY *m_IgnoredCut; // The cut to ignore
  void PinCutAround(FILE_CUT_ENTRY *cutEntry);
  void FindPinCutAround();
  long long SeekOverCut(long long offset, int whence);
};

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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "client.h"
#include "cppmyth/MythProgramInfo.h"

class CGUIDialogSelectRecording
{

public:
  CGUIDialogSelectRecording();
  virtual ~CGUIDialogSelectRecording();

  bool Show();
  void Close();
  void DoModal();

  void Reset();
  void SetHeading(const CStdString &heading);
  void AddRecording(MythProgramInfo &recording, const CStdString &icon);
  MythProgramInfo GetSelectedRecording();

private:
  bool OnClick(int controlId);
  bool OnFocus(int controlId);
  bool OnInit();
  bool OnAction(int actionId);

  static bool OnClickCB(GUIHANDLE cbhdl, int controlId);
  static bool OnFocusCB(GUIHANDLE cbhdl, int controlId);
  static bool OnInitCB(GUIHANDLE cbhdl);
  static bool OnActionCB(GUIHANDLE cbhdl, int actionId);

  CAddonGUIWindow *m_window;
  int m_selection;
  std::vector< std::pair<MythProgramInfo, CStdString> > m_recordings;
};


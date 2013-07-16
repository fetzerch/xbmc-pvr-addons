/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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

#include "GUIDialogSelectRecording.h"
#include "cppmyth/MythTimestamp.h"

#define BUTTON_CANCEL                     1
#define CONTROL_LIST                      10
#define WINDOW_DEFAULT_SKIN               "skin.confluence"
#define WINDOW_XML_FILENAME               "DialogSelectRecording.xml"
#define WINDOW_PROPERTY_HEADING           "Heading"

bool SortTimeDesc(const std::pair<time_t, size_t> &a, const std::pair<time_t, size_t> &b) { return a.first > b.first; }
bool SortTimeAsc(const std::pair<time_t, size_t> &a, const std::pair<time_t, size_t> &b) { return a.first < b.first; }
bool SortStringAsc(const std::pair<CStdString, size_t> &a, const std::pair<CStdString, size_t> &b) { return a.first < b.first; }

CGUIDialogSelectRecording::CGUIDialogSelectRecording()
  : m_window(0)
  , m_selection(-1)
  , m_recordings()
{
  m_window = GUI->Window_create(WINDOW_XML_FILENAME, WINDOW_DEFAULT_SKIN, false, true);
  m_window->m_cbhdl = this;
  m_window->CBOnInit = OnInitCB;
  m_window->CBOnFocus = OnFocusCB;
  m_window->CBOnClick = OnClickCB;
  m_window->CBOnAction = OnActionCB;
}

CGUIDialogSelectRecording::~CGUIDialogSelectRecording()
{
  GUI->Window_destroy(m_window);
}

bool CGUIDialogSelectRecording::OnInitCB(GUIHANDLE cbhdl)
{
  CGUIDialogSelectRecording* dialog = static_cast<CGUIDialogSelectRecording*>(cbhdl);
  return dialog->OnInit();
}

bool CGUIDialogSelectRecording::OnClickCB(GUIHANDLE cbhdl, int controlId)
{
  CGUIDialogSelectRecording* dialog = static_cast<CGUIDialogSelectRecording*>(cbhdl);
  return dialog->OnClick(controlId);
}

bool CGUIDialogSelectRecording::OnFocusCB(GUIHANDLE cbhdl, int controlId)
{
  CGUIDialogSelectRecording* dialog = static_cast<CGUIDialogSelectRecording*>(cbhdl);
  return dialog->OnFocus(controlId);
}

bool CGUIDialogSelectRecording::OnActionCB(GUIHANDLE cbhdl, int actionId)
{
  CGUIDialogSelectRecording* dialog = static_cast<CGUIDialogSelectRecording*>(cbhdl);
  return dialog->OnAction(actionId);
}

bool CGUIDialogSelectRecording::Show()
{
  if (m_window)
    return m_window->Show();

  return false;
}

void CGUIDialogSelectRecording::Close()
{
  if (m_window)
    m_window->Close();
}

void CGUIDialogSelectRecording::DoModal()
{
  if (m_window)
    m_window->DoModal();
}

bool CGUIDialogSelectRecording::OnInit()
{
  int j = 0;
  m_window->ClearList();
  for (std::vector< std::pair<MythProgramInfo, CStdString> >::iterator it = m_recordings.begin(); it != m_recordings.end(); ++it)
  {
    MythTimestamp starttime = MythTimestamp(it->first.RecordingStartTime());
    CAddonListItem* item = GUI->ListItem_create(it->first.Title().c_str(), starttime.DisplayString(true).c_str(), it->second.c_str(), NULL, NULL);
    m_window->AddItem(item, j++);
    GUI->ListItem_destroy(item);
  }
  return true;
}

bool CGUIDialogSelectRecording::OnClick(int controlId)
{
  if (controlId == BUTTON_CANCEL)
  {
    m_window->Close();
  }
  else if (controlId == CONTROL_LIST)
  {
    m_selection = m_window->GetCurrentListPosition();
    m_window->Close();
  }
  return true;
}

bool CGUIDialogSelectRecording::OnFocus(int controlId)
{
  (void)controlId;
  return true;
}

bool CGUIDialogSelectRecording::OnAction(int actionId)
{
  if (actionId == ADDON_ACTION_CLOSE_DIALOG
      || actionId == ADDON_ACTION_PREVIOUS_MENU)
    return OnClick(BUTTON_CANCEL);
  return false;
}

void CGUIDialogSelectRecording::Reset()
{
  m_recordings.clear();
  m_selection = -1;
}

void CGUIDialogSelectRecording::SetHeading(const CStdString &heading)
{
  if (m_window)
    m_window->SetProperty(WINDOW_PROPERTY_HEADING, heading.c_str());
}

void CGUIDialogSelectRecording::AddRecording(MythProgramInfo& recording, const CStdString& icon)
{
  m_recordings.push_back(std::make_pair(recording, icon));
}

MythProgramInfo CGUIDialogSelectRecording::GetSelectedRecording()
{
  if (m_selection >= 0)
      return m_recordings.at(m_selection).first;
  return MythProgramInfo();
}

void CGUIDialogSelectRecording::SortListByRecStartTime(bool reverse)
{
  std::vector< std::pair<MythProgramInfo, CStdString> > vtmp;
  std::vector< std::pair<time_t, size_t> > vsort;
  for (std::vector< std::pair<MythProgramInfo, CStdString> >::iterator it = m_recordings.begin(); it != m_recordings.end(); ++it)
  {
    vsort.push_back(std::make_pair(it->first.RecordingStartTime(), std::distance(m_recordings.begin(), it)));
  }
  if (reverse)
    std::sort(vsort.begin(), vsort.end(), SortTimeDesc);
  else
    std::sort(vsort.begin(), vsort.end(), SortTimeAsc);
  for (std::vector< std::pair<time_t, size_t> >::iterator it = vsort.begin(); it != vsort.end(); ++it)
  {
    vtmp.push_back(m_recordings.at(it->second));
  }
  Reset();
  m_recordings = vtmp;
}

void CGUIDialogSelectRecording::SortListByTitle()
{
  std::vector< std::pair<MythProgramInfo, CStdString> > vtmp;
  std::vector< std::pair<CStdString, size_t> > vsort;
  for (std::vector< std::pair<MythProgramInfo, CStdString> >::iterator it = m_recordings.begin(); it != m_recordings.end(); ++it)
  {
    vsort.push_back(std::make_pair(it->first.Title(), std::distance(m_recordings.begin(), it)));
  }
  std::sort(vsort.begin(), vsort.end(), SortStringAsc);
  for (std::vector< std::pair<CStdString, size_t> >::iterator it = vsort.begin(); it != vsort.end(); ++it)
  {
    vtmp.push_back(m_recordings.at(it->second));
  }
  Reset();
  m_recordings = vtmp;
}

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

#include "MythFile.h"
#include "MythPointer.h"

MythFile::MythFile()
  : m_file_t(new MythPointer<cmyth_file_t>())
  , m_conn(MythConnection())
  , m_stream()
{
}

MythFile::MythFile(cmyth_file_t myth_file, MythConnection &conn)
  : m_file_t(new MythPointer<cmyth_file_t>())
  , m_conn(conn)
  , m_stream(new BasicStreamHandler())
{
  *m_file_t = myth_file;
  m_stream->Register(*this);
}

bool MythFile::IsNull() const
{
  if (m_file_t == NULL)
    return true;
  return *m_file_t == NULL;
}

unsigned long long MythFile::Length()
{
  return m_stream->Length();
}

void MythFile::UpdateLength(unsigned long long length)
{
  int retval = 0;
  retval = cmyth_file_update_length(*m_file_t, length);
  (void)retval;
}

int MythFile::Read(void *buffer, unsigned int length)
{
  return m_stream->Read(buffer, length);
}

long long MythFile::Seek(long long offset, int whence)
{
  return m_stream->Seek(offset, whence);
}

unsigned long long MythFile::Position()
{
  return m_stream->Position();
}

void MythFile::RegisterStreamHandler(StreamHandler* stream)
{
  stream->Register(*this);
  m_stream.reset(stream);
}

MythFile::StreamHandler *MythFile::GetStreamHandler()
{
  return m_stream.get();
}

///////////////////////////////////////////////////////////////////////////////
////
//// Abstract stream handler
////
///////////////////////////////////////////////////////////////////////////////
void MythFile::StreamHandler::Register(const MythFile &file)
{
  m_myth_file = *(file.m_file_t);
}

inline MythFile::StreamHandler::~StreamHandler() { }

///////////////////////////////////////////////////////////////////////////////
////
//// Basic stream handler
////
///////////////////////////////////////////////////////////////////////////////
unsigned long long BasicStreamHandler::Length()
{
  long long retval;
  retval = cmyth_file_length(m_myth_file);
  return retval > 0 ? retval : 0;
}

int BasicStreamHandler::Read(void *buffer, unsigned int length)
{
  int bytesRead;
  bytesRead = cmyth_file_read(m_myth_file, static_cast< char * >(buffer), length);
  return bytesRead;
}

long long BasicStreamHandler::Seek(long long offset, int whence)
{
  long long retval;
  retval = cmyth_file_seek(m_myth_file, offset, whence);
  return retval;
}

unsigned long long BasicStreamHandler::Position()
{
  unsigned long long retval = 0;
  retval = cmyth_file_position(m_myth_file);
  return retval;
}

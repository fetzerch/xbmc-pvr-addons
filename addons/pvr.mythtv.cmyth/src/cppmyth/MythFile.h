#pragma once
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

#include "MythConnection.h"

#include <boost/shared_ptr.hpp>

extern "C" {
#include <cmyth/cmyth.h>
};

class MythFile
{
public:
  class StreamHandler
  {
  public:
    friend class MythFile;
    StreamHandler() : m_myth_file(NULL) { }
    virtual ~StreamHandler();

  protected:
    void Register(const MythFile &file);
    virtual unsigned long long Length() = 0;
    virtual int Read(void *buffer, unsigned int length) = 0;
    virtual long long Seek(long long offset, int whence) = 0;
    virtual unsigned long long Position() = 0;

    cmyth_file_t m_myth_file;
  };

  MythFile();
  MythFile(cmyth_file_t myth_file, MythConnection &conn);

  bool IsNull() const;

  unsigned long long Length();
  void UpdateLength(unsigned long long length);

  int Read(void *buffer, unsigned int length);
  long long Seek(long long offset, int whence);
  unsigned long long Position();

  void RegisterStreamHandler(StreamHandler *stream);
  StreamHandler *GetStreamHandler();
  
private:
  boost::shared_ptr<MythPointer<cmyth_file_t> > m_file_t;
  MythConnection m_conn;

  boost::shared_ptr<StreamHandler> m_stream;

};

class BasicStreamHandler : public MythFile::StreamHandler
{
protected:
  virtual unsigned long long Length();
  virtual int Read(void *buffer, unsigned int length);
  virtual long long Seek(long long offset, int whence);
  virtual unsigned long long Position();
};

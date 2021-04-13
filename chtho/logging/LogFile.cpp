// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "LogFile.h"

#include <unistd.h> // gethostname 

namespace chtho
{
using namespace futil;

LogFile::LogFile(const std::string& name, off_t rollsz, bool threadsafe,
    int flushInter, int checkEveryN)
  : name_(name),
    rollsz_(rollsz),
    flushInter_(flushInter),
    checkEveryN_(checkEveryN),
    cnt_(0),
    mutex_(threadsafe?new MutexLock:nullptr),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0)
{
  assert(name.find('/') == std::string::npos); // will not find '/' in name
  roll();
}
LogFile::~LogFile() = default;
void LogFile::append(const char* line, int len)
{
  if(mutex_)
  {
    MutexLockGuard lock(*mutex_);
    append_unlocked(line, len);
  }
  else append_unlocked(line, len);
}
void LogFile::flush()
{
  if(mutex_)
  {
    MutexLockGuard lock(*mutex_);
    file_->flush();
  }
  else file_->flush();
}
void LogFile::append_unlocked(const char* line, int len)
{
  file_->append(line, len);
  if(file_->written() > rollsz_) roll();
  else  
  {
    ++cnt_;
    if(cnt_ >= checkEveryN_)
    {
      cnt_ = 0;
      time_t now = ::time(NULL);
      time_t curPeriod = now/rollPerSec_*rollPerSec_; 
      if(curPeriod != startOfPeriod_) roll();
      else if(now - lastFlush_ > flushInter_)
      {
        lastFlush_ = now;
        file_->flush(); 
      }
    }
  }
}
bool LogFile::roll()
{
  time_t now = 0;
  std::string name = filename(name_, &now);
  time_t start = now/rollPerSec_*rollPerSec_;
  if(now > lastRoll_)
  {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new AppendFile(name));
    return true;
  }
  return false;
}
std::string LogFile::filename(const std::string& name, time_t* now)
{
  std::string fname;
  fname.reserve(name.size()+64);
  fname = name;
  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
  fname += timebuf;
  char buf[256];
  int r = ::gethostname(buf, sizeof(buf));
  if(r == 0)
  {
    buf[sizeof(buf)-1] = '\0';
    fname += buf;
  } else fname += "unknownhost";
  snprintf(buf, sizeof(buf), ".%d", ::getpid());
  fname += buf;
  fname += ".log";
  return fname;
}
} // namespace chtho


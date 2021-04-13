// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_LOGFILE_H
#define CHTHO_LOGGING_LOGFILE_H
// Back end of logging system 

#include "base/noncopyable.h"
#include "threads/MutexLockGuard.h"
#include "FileUtil.h"

#include <string> 
#include <memory> 

namespace chtho
{
using namespace futil;
class LogFile : noncopyable
{
private:
  const std::string name_;
  const off_t rollsz_;
  const int flushInter_; // flush interval 
  const int checkEveryN_; 

  int cnt_; 

  std::unique_ptr<MutexLock> mutex_; 
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  std::unique_ptr<AppendFile> file_;

  static const int rollPerSec_ = 60*60*24; // 60 days

  static std::string filename(const std::string& name, time_t* now); 
  void append_unlocked(const char* line, int len);
public:
  LogFile(const std::string& name, off_t rollsz, bool threadsafe=true,
      int flushInter=3, int checkEveryN=1024);
  ~LogFile();
  void append(const char* line, int len);
  void flush();
  bool roll();
};
  
  
} // namespace chtho


#endif // !CHTHO_LOGGING_LOGFILE_H
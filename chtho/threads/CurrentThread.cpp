// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CurrentThread.h"

#include <cstdio>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#include <unistd.h>
#define gettid() ::syscall(SYS_gettid)
#endif

namespace chtho
{

__thread pid_t CurrentThread::tid_cached = 0;
__thread char CurrentThread::tid_str[32] = "";
__thread int  CurrentThread::tid_len = 8;
__thread const char* CurrentThread::thread_name = "unknown";
  
__thread pid_t CurrentThread::pid_cached = 0;
__thread char CurrentThread::pid_str[32] = "";
__thread int  CurrentThread::pid_len = 8;

pid_t CurrentThread::tid()
{
  if(__glibc_unlikely(tid_cached == 0))
  {
    tid_cached = gettid();
    tid_len = snprintf(tid_str, sizeof(tid_str), "%5d ", tid_cached);
  }
  return tid_cached;
}

pid_t CurrentThread::pid()
{
  if(__glibc_unlikely(pid_cached == 0))
  {
    pid_cached = ::getpid();
    pid_len = snprintf(pid_str, sizeof(pid_str), "%5d ", pid_cached);
  }
  return pid_cached;
}
} // namespace chtho

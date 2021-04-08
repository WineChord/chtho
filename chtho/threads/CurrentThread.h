// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <pthread.h> 

#ifndef CHTHO_THREADS_CURRENTTHREAD_H
#define CHTHO_THREADS_CURRENTTHREAD_H

namespace chtho
{
class CurrentThread
{
public:
  static __thread pid_t tid_cached;
  static __thread char tid_str[32];
  static __thread int tid_len;
  static __thread const char* thread_name;
  static pid_t tid();
  static const char* tidStr() { return tid_str; }
  static int tidLen() { return tid_len; }
};
} // namespace chtho

#endif // !CHTHO_THREADS_CURRENTTHREAD_H

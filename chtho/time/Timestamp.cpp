// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Timestamp.h"

#include <time.h> // struct timeval
#include <sys/time.h> // gettimeofday

namespace chtho
{


Timestamp Timestamp::now()
{
  // contains seconds and micro seconds 
  struct timeval tv;
  // Get the current time of day and timezone information,
  // putting it into *TV and *TZ. If TZ is NULL, *TZ is not filled.
  // Returns 0 on success, -1 on errors.
  gettimeofday(&tv, NULL);
  int64_t secs = tv.tv_sec;
  return Timestamp(secs * usPerSec + tv.tv_usec);
}
  
} // namespace chtho

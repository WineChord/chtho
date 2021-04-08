// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TimeZone.h"

#include <cstring> // memset
#include <cassert> // assert

namespace chtho
{
  
struct tm TimeZone::toLocal(time_t secsSinceE) const
{
  struct tm local;
  memset(&local, sizeof(local), 0);
  time_t localSecs = secsSinceE + gmtOff_;
  ::gmtime_r(&localSecs, &local);
  // local.tm_isdst = xxx; // dst means day light saving time, I don't care.
  local.tm_gmtoff = gmtOff_;
  local.tm_zone = abbr_;
  return local; // named return value optimization hohoho
}

} // namespace chtho

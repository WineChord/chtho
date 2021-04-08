// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_TIME_TIMEZONE_H
#define CHTHO_TIME_TIMEZONE_H

#include <memory> // std::shared_ptr
#include <time.h> // struct tm;

namespace chtho
{
class TimeZone
{
private:
  bool set_ = false;
  // gmt means Greenwich Mean Time
  // utc means Coordinated Universal Time (no idea why not abbr it CUT)
  time_t gmtOff_; // offset from gmt (or utc, they can be regarded as the same)
  const char* abbr_; // abbreviation for time zone
public:
  TimeZone() = default; // an invalid time zone
  TimeZone(int gmtOff, const char* name)
    : set_(true), gmtOff_(gmtOff), abbr_(name) {}
  bool valid() const { return set_; }
  // given seconds since epoch
  // give out local time such as 2021.4.8 20:25:33 
  struct tm toLocal(time_t secsSinceE) const;
};

} // namespace chtho

#endif // !CHTHO_TIME_TIMEZONE_H
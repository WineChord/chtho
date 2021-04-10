// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_TIME_TIMESTAMP_H
#define CHTHO_TIME_TIMESTAMP_H

#include <stdint.h> // int64_t 
#include <time.h> // time_t 

#include <string> 

namespace chtho
{

class Timestamp
{
private:
  int64_t usSinceE_;  // us since epoch 
public:
  Timestamp() : usSinceE_(0) {}
  explicit Timestamp(int64_t usSinceE)
    : usSinceE_(usSinceE) {}
  static Timestamp now(); // get current time 

  int64_t usSinceE() const { return usSinceE_; }
  time_t secsSinceE() const 
  { return static_cast<time_t>(usSinceE_/usPerSec); }

  int us() const { return static_cast<int>(usSinceE_%usPerSec); }
  bool valid() const { return usSinceE_ > 0; }
  std::string toString() const;

  static double diffInSec(const Timestamp& lhs, const Timestamp& rhs)
  {
    return static_cast<double>(lhs.usSinceE() - rhs.usSinceE())/usPerSec;
  }

  static struct timespec toSpec(int64_t uss) 
  {
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(uss/usPerSec);
    ts.tv_nsec = static_cast<long>(uss%usPerSec * 1000);
    return ts;
  }

  // add seconds 
  Timestamp operator+(const double secs)
  {
    int64_t delta = static_cast<int64_t>(secs * usPerSec);
    return Timestamp(usSinceE_ + delta);
  }
  struct timespec operator-(const Timestamp& rhs) const 
  {
    int64_t us = usSinceE_ - rhs.usSinceE();
    if(us < 100) us = 100;
    return toSpec(us);
  }
  bool operator<(const Timestamp& rhs) const 
  {
    return usSinceE_ < rhs.usSinceE();
  }

  static const int usPerSec = 1000 * 1000; 
};



} // namespace chtho


#endif // !CHTHO_TIME_TIMESTAMP_H
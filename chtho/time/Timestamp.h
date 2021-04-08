// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_TIME_TIMESTAMP_H
#define CHTHO_TIME_TIMESTAMP_H

#include <stdint.h> // int64_t 
#include <time.h> // time_t 

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

  static const int usPerSec = 1000 * 1000; 
};



} // namespace chtho


#endif // !CHTHO_TIME_TIMESTAMP_H
// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_TIMER_H
#define CHTHO_NET_TIMER_H

#include "Callbacks.h" // TimerCB 
#include "time/Timestamp.h"

#include <atomic> // std::atomic_int64_t 

namespace chtho
{
namespace net
{
class Timer
{
private:
  const TimerCB cb_;
  Timestamp expir_; // expiration time 
  const double interval_; // > 0 if repeatable 
  const bool repeat_; // true if interavl_ > 0 
  const int64_t seq_; // sequence number 

  static std::atomic_int64_t numCreated_; 
public:
  Timer(TimerCB cb, Timestamp t, double interval)
    : cb_(std::move(cb)),
      expir_(t),
      interval_(interval),
      repeat_(interval_ > 0.0),
      seq_(++numCreated_)
  {}

  void run() const { cb_(); }
  Timestamp expir() const { return expir_; }
  int64_t seq() const { return seq_; }
  bool repeat() const { return repeat_; }
  void restart(Timestamp now)
  {
    if(repeat_) expir_ = now + interval_;
    else expir_ = Timestamp(); // invalid timestamp 
  }
};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_TIMER_H
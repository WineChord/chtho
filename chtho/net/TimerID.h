// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_TIMERID_H
#define CHTHO_NET_TIMERID_H

#include <stdint.h> // int64_t 

namespace chtho
{
namespace net
{
class Timer; // forward declaration
class TimerID
{
  friend class TimerQueue;
private:
  Timer* timer_;
  int64_t seq_;
public:
  TimerID() : timer_(nullptr), seq_(0) {}
  TimerID(Timer* timer, int64_t seq) : timer_(timer), seq_(seq) {}
};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_TIMERID_H
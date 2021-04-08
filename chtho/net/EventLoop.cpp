// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EventLoop.h"

namespace chtho
{
EventLoop::EventLoop():looping(false),threadID(CurrentThread::tid())
{

}
void EventLoop::abortNotInLoopThread()
{

}
} // namespace chtho

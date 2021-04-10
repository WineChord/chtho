// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_EVENTLOOPTHREADPOOL_H
#define CHTHO_NET_EVENTLOOPTHREADPOOL_H

#include "chtho/base/noncopyable.h"
#include "Callbacks.h"

#include <string>
#include <vector>
#include <memory> 

namespace chtho
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
private:
  EventLoop* base_;
  std::string name_;
  bool started_;
  int numThreads_;
  int nxt_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
public:
  EventLoopThreadPool(EventLoop* base, const std::string name);
  ~EventLoopThreadPool();
  void setNumThread(int num) { numThreads_ = num; }
  void start(const ThreadInitCB& cb = ThreadInitCB());

  EventLoop* getNextLoop();
  EventLoop* getLoopForHash(size_t hashCode);
  std::vector<EventLoop*> getAllLoops();

  bool started() const { return started_; }
  const std::string& name() const { return name_; }
};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_EVENTLOOPTHREADPOOL_H
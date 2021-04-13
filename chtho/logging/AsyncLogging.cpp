// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AsyncLogging.h"

#include <memory>

namespace chtho
{
AsyncLogging::AsyncLogging(const std::string& name, off_t rollsz,
    int flushInter)  
  : flushInter_(flushInter),
    running_(false),
    name_(name),
    rollsz_(rollsz),
    thread_([this](){this->threadFunc();}, "Logging"),
    latch_(1),
    mutex_(),
    cond_(mutex_),
    curBuf_(new Buf),
    nxtBuf_(new Buf),
    bufs_()
{
  curBuf_->zero();
  nxtBuf_->zero();
  bufs_.reserve(16);
}
void AsyncLogging::append(const char* line, int len)
{
  MutexLockGuard lock(mutex_);
  if(curBuf_->avail() > len)
  {
    curBuf_->append(line, len);
  }
  else  
  {
    // if current buffer cannot hold so much data
    // use next buffer, if next buffer is nullptr
    // the create a new one 
    bufs_.push_back(std::move(curBuf_));
    if(nxtBuf_) curBuf_ = std::move(nxtBuf_);
    else curBuf_.reset(new Buf); // rare case 
    curBuf_->append(line, len);
    cond_.notify();
  }
}
void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  // void start() is waiting on this latch_
  // this guarantees the thread function successfully
  // starts 
  latch_.countDown(); 
  LogFile output(name_, rollsz_, false); // false for threadsafe
  BufPtr buf1(new Buf);
  BufPtr buf2(new Buf);
  buf1->zero();
  buf2->zero();
  BufVec bufs;
  bufs.reserve(16);
  while(running_)
  {
    assert(buf1 && buf1->length() == 0);
    assert(buf2 && buf2->length() == 0);
    assert(bufs.empty());

    {
      MutexLockGuard lock(mutex_);
      if(bufs_.empty()) cond_.waitForSecs(flushInter_);
      bufs_.push_back(std::move(curBuf_));
      curBuf_ = std::move(buf1);
      bufs.swap(bufs_);
      if(!nxtBuf_) nxtBuf_ = std::move(buf2);
    }
    assert(!bufs.empty());
    if(bufs.size() > 25)
    {
      char buf[256];
      snprintf(buf, sizeof(buf), "drop log at %s, %zd larger bufs\n",
        Timestamp::now().toString().c_str(), bufs.size()-2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      bufs.erase(bufs.begin()+2, bufs.end());
    }

    for(const auto& buf : bufs)
      output.append(buf->data(), buf->length());
    
    // drop buffers 
    if(bufs.size() > 2)
      bufs.resize(2);
    
    if(!buf1)
    {
      assert(!bufs.empty());
      buf1 = std::move(bufs.back());
      bufs.pop_back();
      buf1->reset();
    }

    if(!buf2)
    {
      assert(!bufs.empty());
      buf2 = std::move(bufs.back());
      bufs.pop_back();
      buf2->reset();
    }

    bufs.clear();
    output.flush();
  }
  output.flush();
}
} // namespace chtho

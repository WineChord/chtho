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
  BufPtr buf1(new Buf); // two empty buffers 
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

    // buffers_ stores the pointers to buffers that need to be written to disk
    // buffers is used to swap with buffers_ so the writing process can be 
    // continued on buffers instead of buffers_. this can narrow down the size 
    // of critical section, since the front end need to call append to write new 
    // messages to the current buffer and the current buffer might be full so as 
    // to be pushed back into buffers_. 
    { // starting critical section 
      MutexLockGuard lock(mutex_);
      // waiting conditions: 1. timeout, 2. one or more buffers are filled by front end 
      if(bufs_.empty()) cond_.waitForSecs(flushInter_); // the default flush interval is 3s
      bufs_.push_back(std::move(curBuf_));
      curBuf_ = std::move(buf1);
      // swap here is used to reduce critical section size
      // furing writing process can be perform on bufs rather than bufs_ 
      bufs.swap(bufs_); // bufs is actually buffers to write 
      // this happens when the current buffer's space is insufficient and is pushed
      // back to the buffers_. then the next buffer pointer is moved to current buffer
      // pointer. this will result in the next buffer pointer being nullptr 
      // in this case, the next buffer pointer is assigned a new buffer pointer 
      if(!nxtBuf_) nxtBuf_ = std::move(buf2); 
    }
    // out of critical section, begin to write bufs to file 
    assert(!bufs.empty());
    // drop logs if too many 
    if(bufs.size() > 25)
    {
      char buf[256];
      snprintf(buf, sizeof(buf), "drop log at %s, %zd larger bufs\n",
        Timestamp::now().toString().c_str(), bufs.size()-2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      bufs.erase(bufs.begin()+2, bufs.end());
    }
    // call LogFile::append
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

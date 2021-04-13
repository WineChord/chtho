// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_FIXEDBUFFER_H
#define CHTHO_LOGGING_FIXEDBUFFER_H
#include "base/noncopyable.h"

#include <cstddef> // size_t 
#include <cstring> // memcpy 

namespace chtho
{
const int kSmall = 4000; // 4KB
const int kLarge = 4000 * 1000; // 4MB

template<int LEN>
class FixedBuffer : noncopyable
{
private:
  char data_[LEN];
  char* cur_; // current pointer position 
  const char* end_; // point to end of the char array
public:
  FixedBuffer() : cur_(data_), end_(data_+LEN) {}
  ~FixedBuffer() {} 
  const char* data() const { return data_; }
  char* cur() { return cur_; }
  // current string length inside the buffer
  size_t length() const { return static_cast<size_t>(cur_ - data_); }
  // space available for current buffer 
  int avail() const { return static_cast<int>(end_ - cur_); }
  // move cur_ pointer forward len 
  void add(size_t len) { cur_ += len; }
  void reset() { cur_ = data_; }
  void zero() { memset(data_, 0, sizeof(data_)); }
  void append(const char* buf, size_t len)
  {
    if(static_cast<size_t>(avail()) > len) // space sufficient
    {
      memcpy(cur_, buf, len);
      cur_ += len; 
    }
  }
};

} // namespace chtho


#endif // !CHTHO_LOGGING_FIXEDBUFFER_H
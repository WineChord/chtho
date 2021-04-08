// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_LOGSTREAM_H
#define CHTHO_LOGGING_LOGSTREAM_H

#include "base/noncopyable.h"
#include "FixedBuffer.h"

#include <cstddef> // size_t 
#include <cstdio> // snprintf
#include <cstring> // strlen 
#include <string>
#include <type_traits> // enable_if, is_integral, is_floating_point
#include <algorithm> // reverse

namespace chtho
{
static const int kMaxNumSize = 32; 
template<typename T>
size_t convert(char buf[], T value);
// rewrite output stream for log, like iostream
class LogStream : noncopyable
{
private:
  using self = LogStream;
public:
  using Buffer = FixedBuffer<kSmall>; // 4KB buffer 
  // output bool value
  self& operator<<(bool v)
  {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }
  // output char 
  self& operator<<(char v)
  {
    buffer_.append(&v, 1);
    return *this;
  }
  // output const char*
  self& operator<<(const char* str)
  {
    if(str)
      buffer_.append(str, strlen(str));
    else
      buffer_.append("(null)", 6); 
    return *this;
  }
  self& operator<<(char* str)
  {
    return operator<<(static_cast<const char*>(str));
  }
  self& operator<<(const unsigned char* str)
  {
    return operator<<(reinterpret_cast<const char*>(str));
  }
  // output string 
  self& operator<<(const std::string& v)
  {
    buffer_.append(v.data(), v.size());
    return *this;
  }
  // output integral value
  template<typename Integer, 
    typename std::enable_if<std::is_integral<Integer>::value, bool>::type = true>
  self& operator<<(Integer v)
  {
    if(buffer_.avail() >= static_cast<int>(sizeof(v)))
    {
      size_t len = convert(buffer_.cur(), v);
      buffer_.add(len);
    }
    return *this;
  }
  // output floating point 
  template<typename Float,
    typename std::enable_if<std::is_floating_point<Float>::value, bool>::type = true>
  self& operator<<(Float v)
  {
    if(buffer_.avail() >= kMaxNumSize)
    {
      size_t len = snprintf(buffer_.cur(), kMaxNumSize, "%.12g", v);
      buffer_.add(len);
    }
    return *this;
  }
  // output pointers
  template<typename Pointer,
    typename std::enable_if<std::is_pointer<Pointer>::value, bool>::type = true>
  self& operator<<(Pointer p)
  {
    if(buffer_.avail() >= kMaxNumSize)
    {
      int len = snprintf(buffer_.cur(), kMaxNumSize, "%p", p);
      buffer_.add(len);
    }
    return *this;
  }

  const Buffer& buffer() const { return buffer_; }
private:
  Buffer buffer_; 
};

extern const char digits[];
extern const char* pzo;

template<typename T>
size_t convert(char buf[], T value)
{
  T i = value;
  char* p = buf;
  do 
  {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = pzo[lsd];
  } while (i != 0);
  if(value < 0)
    *p++ = '-';
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

} // namespace chtho

#endif // !CHTHO_LOGGING_LOGSTREAM_H
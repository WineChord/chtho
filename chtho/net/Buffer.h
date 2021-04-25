// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_BUFFER_H
#define CHTHO_NET_BUFFER_H

#include "base/StringPiece.h"

#include <vector>
#include <algorithm>

#include <assert.h> 
#include <arpa/inet.h>

namespace chtho
{
namespace net
{
// | prependable bytes | readable bytes | writable bytes |
// V                   V                V                V
// 0       <=      readerIdx_  <=    writerIdx_   <=    size 
class Buffer
{
private:
  std::vector<char> buf_;
  size_t readerIdx_;
  size_t writerIdx_;

  static const char CRLF[];

  char* begin() { return &*buf_.begin(); }
  const char* begin() const { return &*buf_.begin(); }
  void expand(size_t len)
  {
    if(writableBytes() + prependableBytes() < len + kPre)
    {
      buf_.resize(writerIdx_+len);
    }
    else  
    {
      // move readable data to the front
      // make space inside buf
      assert(kPre < readerIdx_);
      size_t readable = readableBytes();
      std::copy(begin()+readerIdx_,begin()+writerIdx_,begin()+kPre);
      readerIdx_ = kPre;
      writerIdx_ = readerIdx_ + readable;
      assert(readable == readableBytes());
    }
  }
public:
  static const size_t kPre = 8;
  static const size_t kInit = 1024; 
  explicit Buffer(size_t initSz=kInit)
    : buf_(kPre + initSz),
      readerIdx_(kPre),
      writerIdx_(kPre)
  {
    assert(readableBytes() == 0);
    assert(writableBytes() == initSz);
    assert(prependableBytes() == kPre);
  }

  size_t readableBytes() const { return writerIdx_-readerIdx_; }
  size_t writableBytes() const { return buf_.size()-writerIdx_; }
  size_t prependableBytes() const { return readerIdx_; }
  const char* peek() const { return begin()+readerIdx_; }

  int8_t peekInt8() const 
  {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x; 
  }
  int16_t peekInt16() const 
  {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t x = 0;
    memcpy(&x, peek(), sizeof(x));
    return ntohs(x);
  }
  int32_t peekInt32() const 
  {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t x = 0;
    memcpy(&x, peek(), sizeof(x));
    return ntohl(x);
  }

  int8_t readInt8()
  {
    int8_t res = peekInt8();
    retrieve(sizeof(res));
    return res; 
  }
  int16_t readInt16()
  {
    int16_t res = peekInt16();
    retrieve(sizeof(res));
    return res; 
  }
  int32_t readInt32()
  {
    int32_t res = peekInt32();
    retrieve(sizeof(res));
    return res; 
  }

  void prepend(const void* s, size_t len)
  {
    assert(len <= prependableBytes());
    readerIdx_ -= len;
    const char* d = static_cast<const char*>(s);
    std::copy(d, d+len, begin()+readerIdx_);
  }

  void append(const StringPiece& s) { append(s.data(), s.size()); }
  void append(const void* s, size_t len)
  { append(static_cast<const char*>(s), len); }
  void append(const char* s, size_t len)
  {
    ensure(len);
    std::copy(s, s+len, writePtr());
    written(len);
  }
  void appendInt8(int8_t x) { append(&x, sizeof(x)); }
  void appendInt16(int16_t x)
  {
    x = htons(x);
    append(&x, sizeof(x));
  }
  void appendInt32(int32_t x)
  {
    x = htonl(x);
    append(&x, sizeof(x));
  }
  
  void ensure(size_t len)
  {
    if(writableBytes() < len) expand(len);
    assert(writableBytes() >= len);
  }

  char* writePtr() { return begin() + writerIdx_; }
  const char* writePtr() const { return begin() + writerIdx_; }

  void written(size_t len)
  {
    assert(len <= writableBytes());
    writerIdx_ += len;
  }

  void retrieveAll() { readerIdx_=writerIdx_=kPre; }
  void retrieveUntil(const char* end) { retrieve(end - peek()); }

  void retrieve(size_t len)
  {
    assert(len <= readableBytes());
    if(len < readableBytes()) readerIdx_ += len;
    else retrieveAll();
  }

  std::string retrieveAsString(size_t len)
  {
    assert(len <= readableBytes());
    std::string res(peek(), len);
    retrieve(len);
    return res;
  }

  std::string retrieveAllAsString()
  {
    return retrieveAsString(readableBytes());
  }

  StringPiece toStringPiece() const 
  { return StringPiece(peek(), static_cast<int>(readableBytes())); }

  void swap(Buffer& r)
  {
    buf_.swap(r.buf_);
    std::swap(readerIdx_, r.readerIdx_);
    std::swap(writerIdx_, r.writerIdx_);
  }

  void shrink(size_t reserve)
  {
    Buffer b;
    b.ensure(readableBytes()+reserve);
    b.append(toStringPiece());
    swap(b);
  }

  const char* findEOL() const 
  {
    return findEOL(peek());
  }
  const char* findEOL(const char* start) const 
  {
    assert(peek() <= start);
    assert(start <= writePtr());
    const void* eol = memchr(start, '\n', writePtr()-start);
    return static_cast<const char*>(eol);
  }

  const char* findCRLF() const 
  {
    const char* crlf = std::search(peek(), writePtr(), CRLF, CRLF+2);
    return crlf == writePtr() ? NULL : crlf;
  }

  ssize_t readFd(int fd, int* savedErrno);
};
} // namespace net
} // namespace chtho



#endif // !CHTHO_NET_BUFFER_H
// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Buffer.h"

namespace chtho
{
namespace net
{
  
// readFd will read data from the file descriptor
// to Buffer::buf_ 
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // use scatter/gatter IO
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + writerIdx_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  // when the writable is large enough, do not use extrabuf 
  const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
  // readv will read data from fd and put the result in the buffer
  // described by iovec, which is a vector of iovcnt struct iovec
  // the buffers are filled in the order specified 
  // this operates just like 'read' except that data are put in
  // iovec instead of a contiguous buffer. 
  const ssize_t n = ::readv(fd, vec, iovcnt);
  if(n < 0) *savedErrno = errno;
  else if(static_cast<const size_t>(n) <= writable) writerIdx_ += n;
  else
  {
    writerIdx_ = buf_.size();
    append(extrabuf, n-writable);
  }
  return n; 
}
} // namespace net
} // namespace chtho

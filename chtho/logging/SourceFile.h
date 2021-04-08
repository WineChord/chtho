// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <cstring>

namespace chtho
{
// this class is a helper, it will automatically
// convert a filepath to its file base name at compile time
// its aim is to reduce runtime overhead (reduce the need to
// call strlen). 
class SourceFile
{
public:
  const char* data_; 
  int size_; 
  // given filepath, it will calculate the basename of the file
  // at compile time 
  template<int N> 
  SourceFile(const char (&s)[N])
    : data_(s),
      size_(N-1)
  {
    // strtchr will find the character position
    // in reverse order 
    const char* begin = strrchr(data_, '/');
    if(begin) // has a slash, extract the basename 
    { // update the data pointer and size 
      data_ = begin+1; 
      size_ -= static_cast<int>(data_ - s); 
    }
  }
};
} // namespace chtho

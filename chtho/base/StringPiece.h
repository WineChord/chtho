// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_BASE_STRINGPIECE_H
#define CHTHO_BASE_STRINGPIECE_H

#include <string.h> 
#include <string> 
namespace chtho
{
  

// a wrapper class for all string like object
class StringPiece
{
private:
  const char* p_;
  int len_;
public:
  StringPiece() : p_(nullptr), len_(0) {}
  StringPiece(const char* s) : p_(s), len_(static_cast<int>(strlen(s))) {}
  StringPiece(const unsigned char* s)
    : p_(reinterpret_cast<const char*>(s)),
      len_(static_cast<int>(strlen(p_))) {}
  StringPiece(const std::string& s)
    : p_(s.data()), len_(static_cast<int>(s.length())) {}
  StringPiece(const char* off, int len)
    : p_(off), len_(len) {} 
  const char* data() const { return p_; }
  int size() const { return len_; }
  bool empty() const { return len_ == 0; }
  const char* begin() const { return p_; }
  const char* end() const { return p_ + len_; }

  void clear() { p_ = NULL; len_ = 0; }
  void set(const char* buf, int len) { p_ = buf; len_ = len; }
  void set(const char* s) { p_ = s; len_ = static_cast<int>(strlen(s)); }
  void set(const void* buf, int len)
  { p_ = reinterpret_cast<const char*>(buf); len_ = len; }

  char operator[](int i) const { return p_[i]; }

};

bool operator==(const StringPiece& x, const StringPiece& y)
{
  if(x.size() != y.size()) return false;
  if(memcmp(x.data(), y.data(), x.size()) == 0) return true;
  return false;
}

bool operator!=(const StringPiece& x, const StringPiece& y)
{
  return !(x==y);
}

} // namespace chtho
#endif // !CHTHO_BASE_STRINGPIECE_H
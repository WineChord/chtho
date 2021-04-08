// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_BASE_NONCOPYABLE_H
#define CHTHO_BASE_NONCOPYABLE_H

namespace chtho
{
class noncopyable
{
public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;
protected:
  noncopyable() = default;
  ~noncopyable() = default;
};
} // namespace chtho


#endif // !CHTHO_BASE_NONCOPYABLE_H
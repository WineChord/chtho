// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_REVOKEGUARD_H
#define CHTHO_THREADS_REVOKEGUARD_H

#include "MutexLock.h"

namespace chtho
{
class RevokeGuard
{
private:
  MutexLock& owner_;
public:
  RevokeGuard(MutexLock& owner) : owner_(owner)
  {
    owner_.revokeHolder();
  }
  ~RevokeGuard() { owner_.assignHolder(); }
};
} // namespace chtho

#endif // !CHTHO_THREADS_REVOKEGUARD_H
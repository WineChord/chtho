// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_CALLBACK_H
#define CHTHO_NET_CALLBACK_H

#include <functional> // std::function 

namespace chtho
{
namespace net
{

using TimerCB = std::function<void()>;
using EventCB = std::function<void()>; // event callback
using ReadEventCB = std::function<void()>; // read event callback 

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_CALLBACK_H
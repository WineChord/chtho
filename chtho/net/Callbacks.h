// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_CALLBACK_H
#define CHTHO_NET_CALLBACK_H

#include "time/Timestamp.h"
// #include "TcpConnection.h"

#include <functional> // std::function 
#include <memory> 

namespace chtho
{
namespace net
{
class EventLoop;
class TcpConnection;
class Buffer;

using TimerCB = std::function<void()>;
using EventCB = std::function<void()>; // event callback
using ReadEventCB = std::function<void(Timestamp)>; // read event callback 

using ThreadInitCB = std::function<void(EventLoop*)>;

using TcpConnPtr = std::shared_ptr<TcpConnection>;
using ConnCB = std::function<void(const TcpConnPtr&)>;
using CloseCB = std::function<void(const TcpConnPtr&)>;
using WriteCompleteCB = std::function<void(const TcpConnPtr&)>;
using HighWaterMarkCB = std::function<void(const TcpConnPtr&, size_t)>;

using MsgCB = std::function<void(const TcpConnPtr&,Buffer*,Timestamp)>;

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_CALLBACK_H
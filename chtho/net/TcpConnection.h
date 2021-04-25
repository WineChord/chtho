// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_TCPCONNECTION_H
#define CHTHO_NET_TCPCONNECTION_H

#include "base/noncopyable.h"
#include "logging/Logger.h"
#include "Callbacks.h"
#include "Buffer.h"

#include "InetAddr.h"

#include <memory> 

namespace chtho
{
namespace net
{
class EventLoop;
class Socket;
class Channel;

class TcpConnection : noncopyable,
  public std::enable_shared_from_this<TcpConnection>
{
private:
  enum class State { Disconnected, Connecting, Connected, Disconnecting };
  EventLoop* loop_;
  const std::string name_;
  State state_;
  bool reading_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddr localAddr_;
  const InetAddr peerAddr_;
  ConnCB connCB_;
  MsgCB msgCB_;
  WriteCompleteCB writeCompleteCB_;
  HighWaterMarkCB highWaterMarkCB_;
  CloseCB closeCB_;
  size_t highWaterMark_;
  Buffer inputBuf_;
  Buffer outputBuf_;

  void handleRead(Timestamp rcv);
  void handleWrite();
  void handleClose();
  void handleError();

  void setState(State s) { state_ = s; }

  void shutdownInLoop();

  const char* stateToStr() const; 
  
public:
  TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddr& localAddr,
                const InetAddr& peerAddr);
  ~TcpConnection();
  EventLoop* loop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddr& localAddr() const { return localAddr_; }
  const InetAddr& peerAddr() const { return peerAddr_; }
  bool connected() const { return state_ == State::Connected; }
  bool disconnected() const { return state_ == State::Disconnecting; }
  void shutdown();
  void forceClose();
  void forceCloseInLoop();

  void send(Buffer* buf);
  void send(const StringPiece& msg);
  void sendInLoop(const StringPiece& msg);
  void sendInLoop(const void* data, size_t len);

  std::string getTcpInfoStr() const;

  void connEstablished();
  void connDestroyed(); 

  void setConnCB(const ConnCB& cb) { connCB_ = cb; }
  void setMsgCB(const MsgCB& cb) { msgCB_ = cb; }
  void setWriteCompleteCB(const WriteCompleteCB& cb) { writeCompleteCB_ = cb; }
  void setHighWaterMarkCB(const HighWaterMarkCB& cb) { highWaterMarkCB_ = cb; }
  void setCloseCB(const CloseCB& cb) { closeCB_ = cb; }
};

void defaultConnCB(const TcpConnPtr&);
void defaultMsgCB(const TcpConnPtr&, Buffer*, Timestamp);
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_TCPCONNECTION_H
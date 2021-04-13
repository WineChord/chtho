set(net_SRCS
  Acceptor.cpp
  Buffer.cpp
  Channel.cpp
  Connector.cpp 
  EventLoop.cpp
  EventLoopThread.cpp
  EventLoopThreadPool.cpp
  InetAddr.cpp  
  Socket.cpp
  TcpClient.cpp 
  TcpConnection.cpp  
  TcpServer.cpp 
  Timer.cpp
  TimerQueue.cpp
  poller/EPoll.cpp
  poller/Poll.cpp
  poller/Poller.cpp
)

add_library(chtho_net ${net_SRCS})
target_link_libraries(chtho_net chtho_logging chtho_threads)

add_subdirectory(tests)
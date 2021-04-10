set(net_SRCS
  Buffer.cpp
  Channel.cpp
  EventLoop.cpp
  EventLoopThread.cpp
  EventLoopThreadPool.cpp
  InetAddr.cpp  
  Socket.cpp
  Timer.cpp
  TimerQueue.cpp
  poller/EPoll.cpp
  poller/Poll.cpp
  poller/Poller.cpp
)

add_library(chtho_net ${net_SRCS})
target_link_libraries(chtho_net chtho_logging chtho_threads)

add_subdirectory(tests)
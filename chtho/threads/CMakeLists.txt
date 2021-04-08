set(threads_SRCS
  CountDownLatch.cpp
  CurrentThread.cpp
  Thread.cpp
  ThreadPool.cpp
)

add_library(chtho_threads ${threads_SRCS})
target_link_libraries(chtho_threads pthread)


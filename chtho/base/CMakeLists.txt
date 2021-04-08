set(base_SRCS
  # CurrentThread.cpp
  noncopyable.cpp
)

add_library(chtho_base ${base_SRCS})
target_link_libraries(chtho_base pthread)

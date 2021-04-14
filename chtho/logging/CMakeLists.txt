set(logging_SRCS
  AsyncLogging.cpp
  FileUtil.cpp 
  LogFile.cpp
  Logger.cpp
  LogStream.cpp  
)

add_library(chtho_logging ${logging_SRCS})
target_link_libraries(chtho_logging chtho_base chtho_time)

# install(TARGETS chtho_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/chtho/base)

add_subdirectory(tests)
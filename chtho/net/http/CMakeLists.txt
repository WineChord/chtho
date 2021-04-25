set(http_SRCS
  HTTPContext.cpp  
  HTTPResponse.cpp  
  HTTPServer.cpp  
)

add_library(chtho_http ${http_SRCS})
target_link_libraries(chtho_http chtho_net)

install(TARGETS chtho_http DESTINATION lib)

set(HEADERS  
  HTTPContext.h  
  HTTPRequest.h 
  HTTPResponse.h
  HTTPServer.h 
)

install(FILES ${HEADERS} DESTINATION include/chtho/net/http)

add_executable(http_server tests/HTTPServer_test.cpp)
target_link_libraries(http_server chtho_http)
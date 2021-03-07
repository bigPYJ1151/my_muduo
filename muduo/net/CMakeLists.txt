# include(CheckFunctionExists)

# check_function_exists(accept4 HAVE_ACCEPT4)
# if(NOT HAVE_ACCEPT4) 
#     set_source_files_properties(())
# endif()

set(net_SRCS
    Channel.cc 
    EventLoop.cc 
    Poller.cc 
    SocketsOps.cc 
    poller/DefaultPoller.cc 
    poller/EPollPoller.cc 
    poller/PollPoller.cc 
    Timer.cc 
    TimerQueue.cc 
    EventLoopThread.cc 
    EventLoopThreadPool.cc
    Acceptor.cc 
    Buffer.cc 
    InetAddress.cc 
    Socket.cc 
    TcpConnection.cc 
    TcpServer.cc 
    TcpClient.cc 
    Connector.cc 
)

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

install(TARGETS muduo_net DESTINATION lib)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(tests)
endif()

set(
    base_srcs

    Date.cc
    Timestamp.cc
    TimeZone.cc
    CurrentThread.cc
    Exception.cc
    Condition.cc
    CountDownLatch.cc
    Thread.cc
    ThreadPool.cc
    Logging.cc 
    LogStream.cc 
    FileUtil.cc
    ProcessInfo.cc
    LogFile.cc
    Logging.cc 
    LogStream.cc 
)

add_library(muduo_base ${base_srcs})
target_link_libraries(muduo_base pthread rt)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/base)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(tests)
endif()
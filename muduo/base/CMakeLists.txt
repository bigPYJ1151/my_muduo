
set(
    base_srcs

    Timestamp.cc
    CurrentThread.cc
    Exception.cc
    Condition.cc
    CountDownLatch.cc
    Thread.cc
)

add_library(muduo_base ${base_srcs})
target_link_libraries(muduo_base pthread rt)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/base)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(tests)
endif()
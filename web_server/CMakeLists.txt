
set(
    web_server_srcs

    ContextParse.cc 
)

add_executable(web_server ${web_server_srcs})
target_link_libraries(web_server muduo_base muduo_net)
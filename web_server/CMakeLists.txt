
set(
    web_server_srcs

    ContextParse.cc 
    Response.cc 
    Server.cc 
    TimeWheel.cc 
    Run.cc 
)

add_executable(web_server ${web_server_srcs})
target_link_libraries(web_server muduo_base muduo_net)
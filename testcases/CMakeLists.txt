#生成发布服务的可执行文件
add_executable(test_rpc_server test_rpc_server.cc order.pb.cc )

#生成客户端的可执行文件
add_executable(test_rpc_client test_rpc_client.cc order.pb.cc )

add_executable(test_client test_client.cc )
add_executable(test_tcp test_tcp.cc)
add_executable(test_eventloop test_eventloop.cc )
add_executable(test_log test_log.cc )

target_link_libraries(test_rpc_server tinyxml pthread protobuf netrpc fmt)
target_link_libraries(test_rpc_client tinyxml pthread protobuf netrpc fmt)
target_link_libraries(test_client tinyxml pthread protobuf netrpc fmt)
target_link_libraries(test_tcp tinyxml pthread protobuf netrpc fmt)
target_link_libraries(test_eventloop tinyxml pthread protobuf netrpc fmt)
target_link_libraries(test_log tinyxml pthread protobuf netrpc fmt)
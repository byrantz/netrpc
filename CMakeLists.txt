# 设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(netrpc_rpc)

# 生成debug版本，可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR})
# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(./netrpc/common COMMON_LIST)
aux_source_directory(./netrpc/common/enc ENC_LIST)
aux_source_directory(./netrpc/net NET_LIST)
aux_source_directory(./netrpc/net/tcp TCP_LIST)
aux_source_directory(./netrpc/net/coder CODER_LIST)
aux_source_directory(./netrpc/net/rpc RPC_LIST)
#帮我把test_rpc_server 打包成静态库
add_library(netrpc STATIC ${COMMON_LIST} ${ENC_LIST} ${NET_LIST}  ${TCP_LIST} ${CODER_LIST}  ${RPC_LIST} )
target_link_libraries(netrpc tinyxml pthread protobuf zookeeper_mt)

# 用户实现接口的目录
add_subdirectory(testcases)






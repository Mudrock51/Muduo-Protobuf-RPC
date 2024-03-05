cmake_minimum_required(VERSION 3.10)

set(SRC_LIST friendservice.cc ../friend.pb.cc)

add_executable(Provider ${SRC_LIST})
target_link_libraries(Provider mprpc protobuf)
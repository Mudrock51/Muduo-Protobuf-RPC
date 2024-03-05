cmake_minimum_required(VERSION 3.10)

set(SRC_LIST callfriendservice.cc ../friend.pb.cc)

add_executable(Consumer ${SRC_LIST})
target_link_libraries(Consumer mprpc protobuf)
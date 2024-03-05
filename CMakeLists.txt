cmake_minimum_required(VERSION 3.10)
project(mprpc)

set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g") # 提高调试优化级别: 仅做调试!


# 可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 项目库文件的输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径, g++: -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
# 设置编译库文件搜索路径, g++: -L
include_directories(${PROJECT_SOURCE_DIR}/lib)

# 在子目录下去寻找 CMakeLists.txt 来编译整个项目
add_subdirectory(src)
add_subdirectory(example) # 业务代码
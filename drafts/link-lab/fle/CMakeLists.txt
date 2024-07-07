cmake_minimum_required(VERSION 3.15)

project(FLETools)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug mode

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 查找包
find_package(nlohmann_json REQUIRED)

# 添加可执行文件
add_executable(FLETools main.cpp)

# 链接nlohmann_json库
target_link_libraries(FLETools PRIVATE nlohmann_json::nlohmann_json)

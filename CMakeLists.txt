cmake_minimum_required(VERSION 3.1)
project(hook)

# 1. so preload构建
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC)
add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE cjson)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fPIC -g -O3)

# 配置项
set(CONFIG_FILE_PATH "${CMAKE_SOURCE_DIR}/test/config.json" CACHE STRING "")
set(LOG_FILE_PATH "${CMAKE_SOURCE_DIR}/test/log.txt" CACHE STRING "")
set(ENABLE_ALLOW_LOG "false" CACHE STRING "")
set(ENABLE_DENY_LOG "true" CACHE STRING "")

configure_file(${CMAKE_SOURCE_DIR}/include/option.h.in  ${CMAKE_SOURCE_DIR}/include/option.h)
message("配置文件路径：${CONFIG_FILE_PATH}")
message("日志文件路径：${LOG_FILE_PATH}")
message("ALLOW日志开关: ${ENABLE_ALLOW_LOG}")
message("DENY 日志开关: ${ENABLE_DENY_LOG}")

# 生成测试用配置文件
configure_file(${CMAKE_SOURCE_DIR}/config.json.in ${CMAKE_SOURCE_DIR}/config.json)


# 2. 静态hook扩展构建
set(EXE static_hook)
aux_source_directory(${CMAKE_SOURCE_DIR}/extension_src EXT_SRC)
add_executable(${EXE} ${EXT_SRC})
target_include_directories(${EXE} PRIVATE ${CMAKE_SOURCE_DIR}/extension_include
                                            ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${EXE} PRIVATE cjson)

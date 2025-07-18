cmake_minimum_required(VERSION 3.16)

if(NOT MBED_OS_PATH)
  message(FATAL_ERROR
    "Path to Mbed OS repository must be specified (-DMBED_OS_PATH)")
endif()

include(${MBED_OS_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_OS_PATH} mbed-os)

project(Example C CXX ASM)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. mbed-os-mods)

include_directories(${MODULES_HDRS} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main main.cpp CounterModule.cpp ${MODULES_SRCS})
target_link_libraries(main mbed-os mbed-netsocket)
mbed_set_post_build(main)

mbed_finalize_build()

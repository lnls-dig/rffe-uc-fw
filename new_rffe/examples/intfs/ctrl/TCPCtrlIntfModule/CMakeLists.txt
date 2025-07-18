cmake_minimum_required(VERSION 3.16)

if(NOT MBED_OS_PATH)
  message(FATAL_ERROR
    "Path to Mbed OS repository must be specified (-DMBED_OS_PATH)")
endif()

include(${MBED_OS_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_OS_PATH} mbed-os)

project(Example C CXX ASM)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -g3 -O0 -fno-omit-frame-pointer -fno-inline")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -fno-omit-frame-pointer -fno-inline")
set(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -g3")


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../CtrlCoreModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../../TCPCtrlIntfModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../../SCPIIntModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers

  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../intfs/ctrl/TCPCtrlIntfModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../intfs/ctrl/CtrlIntfModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../Module
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/inc
  ${MODULES_HDRS})

add_executable(${PROJECT_NAME} main.cpp pll_cfg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../CtrlCoreModule/CtrlCoreModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../SCPIIntModule/SCPIIntModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../SCPIIntModule/GlobalRFFE.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../SCPIIntModule/I2CEeprom.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../SCPIIntModule/scpi_cmd.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../SCPIIntModule/scpi_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/DAT31R5SP.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/LM71.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../drivers/CDCE906.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../intfs/ctrl/TCPCtrlIntfModule/TCPCtrlIntfModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../intfs/ctrl/CtrlIntfModule/CtrlIntfModule.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/error.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/expression.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/fifo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/ieee488.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/lexer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/minimal.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/parser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/units.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../lib/libscpi/libscpi/src/utils.c

  ${MODULES_SRCS})


  target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:-g3>
  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
  $<$<CONFIG:Debug>:-fno-inline>
)


target_link_libraries(${PROJECT_NAME} mbed-os mbed-netsocket)
mbed_set_post_build(${PROJECT_NAME})


mbed_finalize_build()
#  cmake ..  -GNinja   -DMBED_TARGET=LPC1768   -DMBED_OS_PATH=../../../../mbed-os   -DCMAKE_BUILD_TYPE=Develop


cmake_minimum_required(VERSION 3.16)

if(NOT MBED_OS_PATH)
  message(FATAL_ERROR
    "Path to Mbed OS repository must be specified (-DMBED_OS_PATH)")
endif()

include(${MBED_OS_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_OS_PATH} mbed-os)

project(Example C CXX ASM)



include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/CtrlCoreModule
  ${CMAKE_CURRENT_SOURCE_DIR}/GlobalRFFE
  ${CMAKE_CURRENT_SOURCE_DIR}/PLL
  ${CMAKE_CURRENT_SOURCE_DIR}/SCPI_RFFE


  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/intfs/ctrl/TCPCtrlIntfModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/intfs/ctrl/SCPIIntfModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/intfs/ctrl/CtrlIntfModule
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/Module
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/inc

  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/CDCE906/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/LM71/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/DAT31R5SP/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/I2CEeprom/include

  ${MODULES_HDRS})


add_executable(${PROJECT_NAME} main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PLL/pll_cfg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CtrlCoreModule/CtrlCoreModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/GlobalRFFE/GlobalRFFE.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SCPI_RFFE/scpi_cmd.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SCPI_RFFE/scpi_tables.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/CDCE906/src/CDCE906.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/LM71/src/LM71.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/DAT31R5SP/src/DAT31R5SP.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-libs/I2CEeprom/src/I2CEeprom.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/intfs/ctrl/TCPCtrlIntfModule/TCPCtrlIntfModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/intfs/ctrl/CtrlIntfModule/CtrlIntfModule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/intfs/ctrl/SCPIIntfModule/SCPIIntfModule.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/error.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/expression.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/fifo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/ieee488.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/lexer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/minimal.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/parser.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/units.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../mbed-os-mods/libscpi/libscpi/src/utils.c

  ${MODULES_SRCS})


target_link_libraries(${PROJECT_NAME} mbed-os mbed-netsocket)
mbed_set_post_build(${PROJECT_NAME})


mbed_finalize_build()
#  cmake ..  -GNinja   -DMBED_TARGET=LPC1768   -DMBED_OS_PATH=../../../../mbed-os   -DCMAKE_BUILD_TYPE=Develop

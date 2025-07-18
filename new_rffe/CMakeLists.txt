cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19...3.26)

# set(MBED_APP_JSON_PATH mbed_app.json)

include(mbed-os/tools/cmake/app.cmake)
add_subdirectory(mbed-os)


project(RFFE
LANGUAGES C CXX ASM)

include_directories(.)

add_executable(RFFE
main.cpp
DAT31R5SP.cpp
)

set(MBED_CONF_TARGET_APPLICATION_PROFILE "full"
CACHE STRING "Mbed OS application profile")

if("MBED_CONF_TARGET_APPLICATION_PROFILE=bare-metal" IN_LIST MBED_CONFIG_DEFINITIONS)
    target_link_libraries(RFFE mbed-baremetal)
else()
    target_link_libraries(RFFE mbed-os)
endif()

#target_link_libraries(CDCE906 )

mbed_set_post_build(RFFE)
mbed_finalize_build()


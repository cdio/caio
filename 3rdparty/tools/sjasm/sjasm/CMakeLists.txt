cmake_minimum_required(VERSION 2.8)
project(sjasm)
file(GLOB sjasm_SRC
    "src/*.h"
    "src/*.cpp"
)
add_executable(sjasm ${sjasm_SRC})
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wno-switch-bool")
endif()
install_targets(/bin sjasm)

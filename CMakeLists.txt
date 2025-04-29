cmake_minimum_required(VERSION 3.12)

set(executable_name egt_energymon)

project(${executable_name}
        VERSION 1.0.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CONFIG_X86_64)
        message(STATUS "Building for x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCONFIG_X86_64=1")
endif()

set(CMAKE_INSTALL_PREFIX $ENV{exec_prefix})
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

find_package(PkgConfig REQUIRED)
find_package(Threads)
pkg_check_modules(EGT REQUIRED libegt)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(include ${CAIRO_INCLUDE_DIRS} ${EGT_INCLUDE_DIRS})

add_executable(${executable_name}
                src/main.cpp
                src/mainwin.cpp
                src/energymon.cpp
                src/pac193x.cpp
)

target_link_libraries(${executable_name} ${EGT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${executable_name} DESTINATION bin)


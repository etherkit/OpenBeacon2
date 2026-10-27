cmake_minimum_required(VERSION 3.16)

# Prefer g++ when the caller did not choose a compiler. On this image
# /usr/bin/c++ is clang and does not find libstdc++.
if(NOT DEFINED CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
  find_program(OB2_GXX NAMES g++)
  if(OB2_GXX)
    set(CMAKE_CXX_COMPILER "${OB2_GXX}" CACHE FILEPATH "Host C++ compiler" FORCE)
  endif()
endif()

project(ob2_host_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(ob2_host_tests
  test_pins.cpp
  test_hal.cpp
  ../OpenBeacon2/ob2_relays.cpp
  ../OpenBeacon2/ob2_power.cpp
  ../OpenBeacon2/ob2_softstart.cpp
  ../OpenBeacon2/ob2_dac.cpp
  ../OpenBeacon2/ob2_hold.cpp)
target_include_directories(ob2_host_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../OpenBeacon2)
target_include_directories(ob2_host_tests SYSTEM PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/doctest)
target_compile_options(ob2_host_tests PRIVATE -Wall -Wextra -Werror)

add_test(NAME host COMMAND ob2_host_tests)

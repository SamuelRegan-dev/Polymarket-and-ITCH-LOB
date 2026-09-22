cmake_minimum_required(VERSION 3.28)
project(untitled54)

set(CMAKE_CXX_STANDARD 20)          # bumped from 17: itchcpp requires C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_executable(untitled54 "My Code/main.cpp"
        "My Code/core/Matching_Engine.cpp"
        "My Code/transport/Fetch_Orders.h"
        "My Code/core/main.h"
        "My Code/core/Matching_Engine.h"
        "My Code/env.cpp"
        "My Code/env.h"
        "My Code/irrelevant stuff/Test_Fetch_Orders.cpp"
        "My Code/irrelevant stuff/beast_asio_practice.cpp"
        "My Code/run_feed.h"
        "My Code/config/event.h"
        "My Code/adapters/pm_adapter.h"
        "My Code/adapters/pm_adapter.cpp"
        "My Code/adapters/itch_adapter.h"
        "My Code/adapters/itch_adapter.cpp"
        "My Code/args.cpp"
        "My Code/config/config.cpp"
)

target_include_directories(untitled54 PRIVATE
        "${CMAKE_SOURCE_DIR}/My Code"
        "${CMAKE_SOURCE_DIR}/My Code/Config")

target_precompile_headers(untitled54 PRIVATE "My Code/transport/Fetch_Orders.h")

include(FetchContent)
FetchContent_Declare(
        simdjson
        GIT_REPOSITORY https://github.com/simdjson/simdjson.git
        GIT_TAG tags/v3.6.0
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(simdjson)

# itchcpp -- NASDAQ TotalView-ITCH 5.0 parser, MIT, (c) bbalouki
# https://github.com/bbalouki/itchcpp   (see THIRD_PARTY.md)
# Everything optional is off: this build only needs the parser.
set(ITCH_BUILD_TESTS         OFF CACHE BOOL "" FORCE)
set(ITCH_BUILD_BENCHMARKS    OFF CACHE BOOL "" FORCE)
set(ITCH_BUILD_EXAMPLES      OFF CACHE BOOL "" FORCE)
set(ITCH_BUILD_TOOLS         OFF CACHE BOOL "" FORCE)
set(ITCH_BUILD_FUZZERS       OFF CACHE BOOL "" FORCE)
set(ITCH_BUILD_PYTHON        OFF CACHE BOOL "" FORCE)
set(ITCH_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        itchcpp
        GIT_REPOSITORY https://github.com/bbalouki/itchcpp.git
        GIT_TAG main
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(itchcpp)

target_link_libraries(untitled54 PRIVATE simdjson)
target_link_libraries(untitled54 PRIVATE itch::itch)

target_link_libraries(untitled54 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
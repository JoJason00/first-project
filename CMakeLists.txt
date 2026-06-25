cmake_minimum_required(VERSION 3.16)

project(project1 LANGUAGES CXX)

message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "CXX compiler id: ${CMAKE_CXX_COMPILER_ID}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Enable code coverage" OFF)

function(enable_coverage target_name)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE
            -O0
            -g
            --coverage
        )

        get_target_property(target_type ${target_name} TYPE)

        if(NOT target_type STREQUAL "STATIC_LIBRARY")
            target_link_options(${target_name} PRIVATE
                --coverage
            )
        endif()
    else()
        message(WARNING "Coverage is currently configured only for GNU/GCC.")
    endif()
endfunction()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(add
    src/add.cpp
    src/add.h
)

target_include_directories(add PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/eigen-5.0.0
)



add_executable(app src/main.cpp)

target_link_libraries(app PRIVATE add)


include(CTest)

if(BUILD_TESTING)
    add_subdirectory(third_party/Catch2)

    add_executable(test_add
        tests/test_add.cpp
    )

    target_link_libraries(test_add PRIVATE
        add
        Catch2::Catch2WithMain
    )

    add_test(NAME test_add COMMAND test_add)
endif()

add_executable(profile_app
    src/main.cpp
)

target_link_libraries(profile_app PRIVATE
    add
)


if(MSVC)
    target_compile_options(app PRIVATE /W4 /permissive-)
    target_compile_options(add PRIVATE /W4 /permissive-)
    target_compile_options(profile_app PRIVATE /W4 /permissive-)

    if(BUILD_TESTING)
        target_compile_options(test_add PRIVATE /W4 /permissive-)
    endif()
else()
    target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(add PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(profile_app PRIVATE -Wall -Wextra -Wpedantic)

    if(BUILD_TESTING)
        target_compile_options(test_add PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()


if(ENABLE_COVERAGE)
    enable_coverage(add)
    enable_coverage(profile_app)
    if(BUILD_TESTING)
        enable_coverage(test_add)
    endif()
endif()


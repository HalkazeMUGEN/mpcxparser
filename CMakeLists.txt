cmake_minimum_required(VERSION 3.14)

set(MPCXPARSER_VERSION_MAJOR 1)
set(MPCXPARSER_VERSION_MINOR 1)
set(MPCXPARSER_VERSION_PATCH 0)

project(mpcxparser VERSION ${MPCXPARSER_VERSION_MAJOR}.${MPCXPARSER_VERSION_MINOR}.${MPCXPARSER_VERSION_PATCH} LANGUAGES CXX)

# ==== Config ====

option(MPCXPARSER_BUILD_ALL "Build all artifacts" OFF)

option(MPCXPARSER_BUILD_EXAMPLE "Build example" OFF)
option(MPCXPARSER_BUILD_EXAMPLE_HO "Build header only example" OFF)

option(MPCXPARSER_BUILD_TESTS "Build tests" OFF)
option(MPCXPARSER_BUILD_TESTS_HO "Build tests for header only version" OFF)

# ================

set(MPCXPARSER_SOURCES "include/mpcxparser/impl/mpcxparser.cpp" "include/mpcxparser/impl/mugenpcx.cpp")
set(MPCXPARSER_HEADERS "include/mpcxparser/mpcxparser.h" "include/mpcxparser/mugenpcx.hpp")

add_library(mpcxparser ${MPCXPARSER_SOURCES})
add_library(mpcxparser::mpcxparser ALIAS mpcxparser)

target_compile_features(mpcxparser PUBLIC cxx_std_20)

target_include_directories(mpcxparser PUBLIC include/)

target_compile_definitions(
  mpcxparser
  PUBLIC
  $<$<CONFIG:Debug>:
    _DEBUG
    DEBUG
  >
)

if(MSVC)
  set_property(TARGET mpcxparser PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET mpcxparser PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>")
  target_compile_options(
    mpcxparser
    PUBLIC
    /W4 /arch:AVX /fp:fast /GS /GT /MP /nologo /Oi /Qpar /utf-8
    $<$<CONFIG:Debug>:/JMC /LDd /Ob0 /Od /RTC1>
    $<$<CONFIG:Release>:/GA /GL /Gy /guard:cf- /LD /Os>
  )
  if(CMAKE_CXX_FLAGS MATCHES "/W3")
    STRING (REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  endif(CMAKE_CXX_FLAGS MATCHES "/W3")
  if(CMAKE_CXX_FLAGS MATCHES "/GR")
    STRING(REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else(CMAKE_CXX_FLAGS MATCHES "/GR")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
  endif(CMAKE_CXX_FLAGS MATCHES "/GR")
else(MSVC)
  target_compile_options(
    mpcxparser
    PUBLIC
    -Wall -Wextra -mavx -mfpmath=sse
    $<$<CONFIG:Debug>:-Og -g>
    $<$<CONFIG:Release>:-Ofast -ftree-vectorize -fomit-frame-pointer>
  )
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # for x64
    target_compile_options(mpcxparser PUBLIC -m64)
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # for x86
    target_compile_options(mpcxparser PUBLIC -m32)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
endif(MSVC)

# Define for header only version

add_library(mpcxparser_header_only INTERFACE)
add_library(mpcxparser::mpcxparser_header_only ALIAS mpcxparser_header_only)

target_compile_definitions(mpcxparser_header_only INTERFACE MPCXPARSER_HEADER_ONLY=1)
target_compile_features(mpcxparser_header_only INTERFACE cxx_std_20)

target_include_directories(mpcxparser_header_only INTERFACE include/)

if(MSVC)
  target_compile_options(
    mpcxparser_header_only
    INTERFACE
    /utf-8
  )
endif(MSVC)

# Define examples

if(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_EXAMPLE OR MPCXPARSER_BUILD_EXAMPLE_HO)

  if(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_EXAMPLE)
    add_subdirectory(example/mpcxparser)
  endif(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_EXAMPLE)

  if(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_EXAMPLE_HO)
    add_subdirectory(example/mpcxparser_header_only)
  endif(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_EXAMPLE_HO)

endif(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_EXAMPLE OR MPCXPARSER_BUILD_EXAMPLE_HO)

# Define tests

if(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_TESTS OR MPCXPARSER_BUILD_TESTS_HO)

  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  set(MPCXPARSER_TEST_SOURCES "test/parse.cpp" "test/write.cpp")

  if(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_TESTS)
    add_executable(${PROJECT_NAME}-googletest ${MPCXPARSER_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}-googletest mpcxparser)
    target_link_libraries(${PROJECT_NAME}-googletest gtest_main)
    if(MSVC)
      target_link_options(${PROJECT_NAME}-googletest PRIVATE /LTCG)
    endif(MSVC)
    add_test(NAME test COMMAND ${PROJECT_NAME}-googletest)

    # Copy assets to working dir
    add_custom_command(
      TARGET ${PROJECT_NAME}-googletest POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/test/assets $<TARGET_FILE_DIR:${PROJECT_NAME}-googletest>/assets
    )
  endif(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_TESTS)

  if(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_TESTS_HO)
    add_executable(${PROJECT_NAME}_header_only-googletest ${MPCXPARSER_TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_header_only-googletest mpcxparser_header_only)
    target_link_libraries(${PROJECT_NAME}_header_only-googletest gtest_main)
    add_test(NAME test_header_only COMMAND ${PROJECT_NAME}_header_only-googletest)

    # Copy assets to working dir
    add_custom_command(
      TARGET ${PROJECT_NAME}_header_only-googletest POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/test/assets $<TARGET_FILE_DIR:${PROJECT_NAME}_header_only-googletest>/assets
    )
  endif(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_TESTS_HO)

  enable_testing()

endif(MPCXPARSER_BUILD_ALL OR MPCXPARSER_BUILD_TESTS OR MPCXPARSER_BUILD_TESTS_HO)

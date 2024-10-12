cmake_minimum_required(VERSION 3.14)
project(mpcxparser_header_only-example VERSION 1.0.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)

# For stand-alone build
# (For example, if you use a git submodule)
if(NOT TARGET mpcxparser)
  find_package(mpcxparser REQUIRED)
endif()

## Recommended build style
# include(FetchContent)
# FetchContent_Declare(
#   mpcxparser
#   GIT_REPOSITORY https://github.com/HalkazeMUGEN/mpcxparser.git
#   GIT_TAG v1.0.0  # Choose any version
# )
# FetchContent_MakeAvailable(mpcxparser)

target_link_libraries(${PROJECT_NAME} mpcxparser::mpcxparser_header_only)

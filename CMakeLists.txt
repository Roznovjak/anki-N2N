cmake_minimum_required( VERSION 3.11 )
project( anki-N2N LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

find_package( CURL REQUIRED)
find_package( ICU 67 COMPONENTS uc REQUIRED )

add_executable( anki-N2N
    src/main.cpp
    src/helpers.cpp
    src/helpers.hpp
)

include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


target_include_directories( anki-N2N PRIVATE ${CURL_INCLUDE_DIR} ${json_INCLUDE_DIR} ${ICU_INCLUDE_DIR} )

target_link_libraries( anki-N2N PRIVATE CURL nlohmann_json::nlohmann_json ICU::uc )

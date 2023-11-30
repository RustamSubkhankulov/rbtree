set(SRC src)
set(INC inc)
set(CNF conf)

macro(TEST_TARGET TEST SOURCE)
  add_executable(${TEST} ${SRC}/${SOURCE}.cpp)
  set_target_properties(
        ${TEST} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
  target_link_libraries(${TEST} PRIVATE rbtree)
  target_include_directories(${TEST} PRIVATE ${INC})
endmacro()

macro(INTERACTIVE_TEST_TARGET TEST SOURCE)
  TEST_TARGET(${TEST} ${SOURCE})
  target_include_directories(${TEST} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

INTERACTIVE_TEST_TARGET(custom_query interactive)
INTERACTIVE_TEST_TARGET(stdset_query interactive)

TEST_TARGET(query query)
TEST_TARGET(unit unit)

option(STDDIST "Use std::distance instead of RBTREE::rbtree::distance() in interactive test with rbtree" OFF)
option(MEASURE_TIME "Measure time in interactive test" OFF)

configure_file(${CNF}/interactive_conf.hpp.in interactive_conf.hpp @ONLY)
target_compile_definitions(stdset_query PRIVATE STDSET)

find_package(GTest)
if (NOT GTest_FOUND)
  include(FetchContent)

  option(BUILD_GMOCK OFF)
  option(INSTALL_GTEST OFF)

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
  )
  FetchContent_MakeAvailable(googletest)

  add_library(GTest::GTest INTERFACE IMPORTED)
  target_link_libraries(GTest::GTest INTERFACE gtest_main)
endif()

target_link_libraries(query PRIVATE GTest::GTest)
target_link_libraries(unit PRIVATE GTest::GTest)

add_test(NAME query COMMAND query)
add_test(NAME unit COMMAND unit)

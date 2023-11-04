set(SRC src)
set(INC inc)
set(CNF conf)

macro(TEST_TARGET TEST)
  add_executable(${TEST} ${SRC}/${TEST}.cpp)
  set_target_properties(
        ${TEST} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
  target_link_libraries(${TEST} PRIVATE rbtree)
  target_include_directories(${TEST} PRIVATE ${INC})
endmacro()

TEST_TARGET(interactive)
TEST_TARGET(query)
TEST_TARGET(unit)

option(STDDIST "Use std::disnatce instead of RBTREE::rbtree::distance() in interactive test" OFF)
option(MEASURE_TIME "Measure time in interactive test" OFF)

configure_file(${CNF}/interactive_conf.hpp.in interactive_conf.hpp @ONLY)
target_include_directories(interactive PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

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

target_link_libraries(query PRIVATE GTest::GTest)
target_link_libraries(unit PRIVATE GTest::GTest)

add_test(NAME query COMMAND query)
add_test(NAME unit COMMAND unit)

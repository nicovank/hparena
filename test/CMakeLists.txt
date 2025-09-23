include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY git@github.com:google/googletest.git
  GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723 # v1.17.0
)

message(STATUS "Downloading and configuring googletest")
FetchContent_MakeAvailable(googletest)

add_executable(test-hparena-c test-hparena-c.cpp)
target_link_libraries(test-hparena-c PRIVATE hparena GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test-hparena-c)

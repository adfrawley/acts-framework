add_executable(
  ACTFWHelloWorldExample
  HelloWorld.cpp
  HelloLoggerAlgorithm.cpp
  HelloRandomAlgorithm.cpp
  HelloService.cpp
  HelloWhiteBoardAlgorithm.cpp)
target_link_libraries(
  ACTFWHelloWorldExample
  PRIVATE ACTFramework ACTFWExamplesCommon ActsCore Boost::program_options)

install(
  TARGETS ACTFWHelloWorldExample
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

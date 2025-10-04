cmake_minimum_required(VERSION 3.25)

qt_standard_project_setup()

qt_add_executable(model_tests model_tests.cpp)
add_test(NAME model_tests COMMAND model_tests)
target_compile_features(model_tests PUBLIC cxx_std_20)
target_link_libraries(model_tests PRIVATE Qt::Core Qt::Test)

cmake_minimum_required(VERSION 3.29)

project(Accountant LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets)

qt_standard_project_setup()

qt_add_executable(accountant main.cpp)
target_compile_features(accountant PUBLIC cxx_std_20)
target_compile_options(accountant PUBLIC "-pedantic-errors" "-Wall" "-Wextra")
target_link_libraries(accountant PUBLIC Qt6::Core Qt6::Sql Qt6::Widgets)

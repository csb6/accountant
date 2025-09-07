cmake_minimum_required(VERSION 3.29)

project(Accountant LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets)
find_package(PkgConfig)
pkg_check_modules(libofx "libofx" IMPORTED_TARGET)

set(LEAK_SUPPR_FILE ${CMAKE_SOURCE_DIR}/scripts/leak-suppressions.txt)

add_subdirectory(src)

cmake_minimum_required(VERSION 3.25)

project(Accountant LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets)
find_package(PkgConfig)
pkg_check_modules(libofx "libofx" IMPORTED_TARGET)

option(WITH_ASAN "Build with AddressSanitizer and LeakSanitizer" OFF)
if(WITH_ASAN)
    set(LEAK_SUPPR_FILE ${CMAKE_SOURCE_DIR}/scripts/leak-suppressions.txt)
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=address")
endif()

add_subdirectory(src)

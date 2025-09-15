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

option(SQL_QUERY_LOGGING "Log text of queries as they are sent to SQLite" OFF)
if(SQL_QUERY_LOGGING)
    message(NOTICE "NOTE: The installation of SQLite being used MUST be the same as the installation used by Qt's SQlite plugin")
    find_package(SQLite3 REQUIRED)
endif()

add_subdirectory(src)

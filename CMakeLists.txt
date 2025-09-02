cmake_minimum_required(VERSION 3.29)

project(Accountant LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(libofx "libofx" IMPORTED_TARGET REQUIRED)

add_executable(ofx_demo ofx_demo.cpp)
target_compile_features(ofx_demo PUBLIC cxx_std_20)
target_compile_options(ofx_demo PUBLIC "-pedantic-errors" "-Wall" "-Wextra")
target_link_libraries(ofx_demo PUBLIC PkgConfig::libofx)

qt_standard_project_setup()

qt_add_executable(accountant
    main.cpp
    views/mainwindow.ui views/MainWindow.cpp
    views/transactionsview.ui views/TransactionsView.cpp
    models/AccountTreeModel.cpp
    models/AccountModel.cpp
    sql_helpers.cpp)
target_include_directories(accountant PUBLIC .)
target_compile_features(accountant PUBLIC cxx_std_20)
target_compile_options(accountant PUBLIC "-pedantic-errors" "-Wall" "-Wextra")
target_link_libraries(accountant PUBLIC Qt::Core Qt::Sql Qt::Widgets)

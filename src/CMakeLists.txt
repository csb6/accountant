cmake_minimum_required(VERSION 3.25)

if(libofx_FOUND)
    add_executable(ofx_demo ofx_demo.cpp)
    target_compile_features(ofx_demo PUBLIC cxx_std_20)
    target_link_libraries(ofx_demo PUBLIC PkgConfig::libofx)
else()
    message("Unable to find libofx - OFX support skipped")
endif()

qt_standard_project_setup()

qt_add_executable(accountant
    main.cpp
    views/mainwindow.ui views/MainWindow.cpp
    views/transactionsview.ui views/TransactionsView.cpp
    models/AccountTree.cpp
    models/AccountTransactions.cpp
    models/DatabaseManager.cpp
    util/sql_helpers.cpp)
target_include_directories(accountant PUBLIC .)
target_compile_features(accountant PUBLIC cxx_std_20)
target_link_libraries(accountant PUBLIC Qt::Core Qt::Sql Qt::Widgets)

if(SQL_QUERY_LOGGING)
    target_compile_definitions(accountant PRIVATE SQL_QUERY_LOGGING)
    target_link_libraries(accountant PUBLIC SQLite::SQLite3)
endif()

cmake_minimum_required(VERSION 3.29)

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

if(WITH_ASAN)
    add_custom_target(run_with_asan
        "ASAN_OPTIONS=detect_leaks=1" "LSAN_OPTIONS=suppressions=${LEAK_SUPPR_FILE}"
        ${CMAKE_CURRENT_BINARY_DIR}/accountant accounts.db
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS accountant
    )
endif()

cmake_minimum_required(VERSION 3.25)

if(libofx_FOUND)
    add_executable(ofx_demo ofx_demo.cpp)
    target_compile_features(ofx_demo PUBLIC cxx_std_20)
    target_link_libraries(ofx_demo PUBLIC PkgConfig::libofx)
endif()

qt_standard_project_setup()

qt_add_executable(accountant
    MACOSX_BUNDLE
    main.cpp
    views/mainwindow.ui views/MainWindow.cpp
    views/accountsview.ui views/AccountsView.cpp
    views/transactionsview.ui views/TransactionsView.cpp
    views/AboutDialog.ui views/AboutDialog.cpp
    models/AccountTree.cpp
    models/AccountTransactions.cpp
    models/DatabaseManager.cpp
    delegates/EditDelegate.cpp
    util/sql_helpers.cpp)
target_include_directories(accountant PUBLIC .)
target_compile_features(accountant PUBLIC cxx_std_20)
target_link_libraries(accountant PUBLIC Qt::Core Qt::Sql Qt::Widgets)

if(SQL_QUERY_LOGGING)
    target_compile_definitions(accountant PRIVATE SQL_QUERY_LOGGING)
    target_link_libraries(accountant PUBLIC SQLite::SQLite3)
endif()

if(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}")
    # Fixup the .app bundle to include/use all of the shared libraries needed by Qt
    # Seem to need to run this twice for some reason (fails with error on first run)
    add_custom_target(deploy
        COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Accountant.app" "-always-overwrite"
        DEPENDS accountant)
    set_target_properties(accountant PROPERTIES
        ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/Accountant.app"
    )
endif()

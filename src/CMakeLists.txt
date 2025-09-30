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
    views/MainWindow.ui views/MainWindow.cpp
    views/AccountsView.ui views/AccountsView.cpp
    views/TransactionsView.ui views/TransactionsView.cpp
    views/AboutDialog.ui views/AboutDialog.cpp
    views/NewAccountDialog.ui views/NewAccountDialog.cpp
    models/AccountTree.cpp
    models/AccountTransactions.cpp
    models/DatabaseManager.cpp
    util/sql_helpers.cpp)

target_include_directories(accountant PUBLIC .)
target_compile_features(accountant PUBLIC cxx_std_20)
target_link_libraries(accountant PUBLIC Qt::Core Qt::Sql Qt::Widgets)

qt_add_executable(generate_about_text "${CMAKE_SOURCE_DIR}/tools/generate_about_text.cpp" "${CMAKE_SOURCE_DIR}/tools/spdx_parser.cpp")
target_include_directories(generate_about_text PUBLIC .)
target_compile_features(generate_about_text PUBLIC cxx_std_20)
target_link_libraries(generate_about_text PUBLIC Qt::Core)
set_property(SOURCE "about.md" PROPERTY QT_RESOURCE_ALIAS "about.md")
if("${Qt6_VERSION_MAJOR}" EQUAL 6 AND "${Qt6_VERSION_MINOR}" GREATER_EQUAL 8)
    target_compile_definitions(generate_about_text PRIVATE SPDX_PROVIDED)
    set(SBOM_PATH "${QT6_INSTALL_PREFIX}/share/qt/sbom/qtbase-${Qt6_VERSION}.spdx")
else()
    message(NOTICE "NOTE: SBOM generation disabled since your version of Qt (${Qt6_VERSION}) does not include SPDX files")
    set(SBOM_PATH ".")
endif()
add_custom_command(
    OUTPUT "about.md"
    COMMAND generate_about_text "${SBOM_PATH}" "about.md"
    DEPENDS generate_about_text
)
qt_add_resources(accountant "about" PREFIX "/accountant" FILES "${CMAKE_CURRENT_BINARY_DIR}/about.md")

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

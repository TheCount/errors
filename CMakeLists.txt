cmake_minimum_required( VERSION 3.0 )
project( errors )

# Version
set( ERRORS_VERSION_MAJOR 0 )
set( ERRORS_VERSION_MINOR 0 )
set( ERRORS_VERSION_PATCH 0 )
set( ERRORS_VERSION "${ERRORS_VERSION_MAJOR}.${ERRORS_VERSION_MINOR}.${ERRORS_VERSION_PATCH}" )

# Package information
set( PACKAGE_NAME ${PROJECT_NAME} )
set( PACKAGE_DESCRIPTION "Small C error string handling library" )
set( PACKAGE_VERSION "${ERRORS_VERSION}" )

# pkg-config
set( PKGCONFIG_NAME "pkgconfig" )
include( GNUInstallDirs )
set( prefix "${CMAKE_INSTALL_PREFIX}" )
set( includedir "${CMAKE_INSTALL_INCLUDEDIR}" )
set( PKGCONFIG_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/${PKGCONFIG_NAME}" )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/errors.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/errors.pc" @ONLY )

# Install files
install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/errors.h" DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR} )
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/errors.pc" DESTINATION ${PKGCONFIG_DIR} )

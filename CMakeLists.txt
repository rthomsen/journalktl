project(journalktl)
set(JOURNALKTL_VERSION_MAJOR "0")
set(JOURNALKTL_VERSION_MINOR "1")
set(JOURNALKTL_VERSION_PATCH "0")
set(JOURNALKTL_VERSION "${JOURNALKTL_VERSION_MAJOR}.${JOURNALKTL_VERSION_MINOR}.${JOURNALKTL_VERSION_PATCH}")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 5.0.0 CONFIG REQUIRED COMPONENTS
             Widgets
             Concurrent)

find_package(KF5 REQUIRED COMPONENTS
             CoreAddons
             XmlGui
             I18n)

message(STATUS "Searching for libsystemd...")
find_library(SYSTEMD_FOUND NAMES systemd)
if(SYSTEMD_FOUND)
    message(STATUS "Found at: " ${SYSTEMD_FOUND})
else(SYSTEMD_FOUND)
    message(FATAL_ERROR "Unable to find libsystemd. Configuration failed!")
endif(SYSTEMD_FOUND)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(CheckIncludeFiles)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -std=c++11")

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

add_definitions(-DTRANSLATION_DOMAIN=\"journalktl\")

add_subdirectory(src)

find_package(KF5I18n CONFIG REQUIRED)
ki18n_install(po)

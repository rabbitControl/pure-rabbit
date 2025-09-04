cmake_minimum_required(VERSION 3.30)

project(RabbitControl LANGUAGES C CXX VERSION 1.1.0)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_MACOSX_RPATH OFF)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")


# include and setup pd.cmake
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.4")
if(NOT EXISTS "${PDCMAKE_FILE}")
    file(DOWNLOAD https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/${PDCMAKE_VERSION}/pd.cmake ${PDCMAKE_FILE})
endif()
include(${PDCMAKE_FILE})


# add help files
pd_add_datafile(RabbitControl package/help/pd/1.RabbitControl-Overview.pd)
pd_add_datafile(RabbitControl package/help/pd/rabbit.client-help.pd)
pd_add_datafile(RabbitControl package/help/pd/rabbit.format-help.pd)
pd_add_datafile(RabbitControl package/help/pd/rabbit.parse-help.pd)
pd_add_datafile(RabbitControl package/help/pd/rabbit.server-help.pd)
pd_add_datafile(RabbitControl package/help/pd/sizeprefix-help.pd)
pd_add_datafile(RabbitControl package/help/pd/slipdecoder-help.pd)
pd_add_datafile(RabbitControl package/help/pd/slipencoder-help.pd)
pd_add_datafile(RabbitControl package/help/pd/sppp-help.pd)

# add rcp. -> rabbit. wrappers
pd_add_datafile(RabbitControl package/compat/rcp.server.pd)
pd_add_datafile(RabbitControl package/compat/rcp.client.pd)
pd_add_datafile(RabbitControl package/compat/rcp.format.pd)
pd_add_datafile(RabbitControl package/compat/rcp.parse.pd)

#
include_directories(dependencies/scaryws)
add_subdirectory(dependencies/scaryws)

include_directories(dependencies/rcp-c)
add_subdirectory(dependencies/rcp-c)


# include projects
include(cmake/rabbit.server.cmake)
include(cmake/rabbit.client.cmake)
include(cmake/rabbit.format.cmake)
include(cmake/rabbit.parse.cmake)
include(cmake/sizeprefix.cmake)
include(cmake/sppparser.cmake)
include(cmake/slipencode.cmake)
include(cmake/slipdecode.cmake)

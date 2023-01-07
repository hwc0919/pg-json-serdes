cmake_minimum_required(VERSION 3.10)
project(pg_func_serdes)

set(CMAKE_CXX_STANDARD 11)

#add_library(pg_func_serdes main.cpp)
add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/inc>)

set(public_headers
        lib/inc/pfs/Catalogue.h
        lib/inc/pfs/IResult.h
        lib/inc/pfs/PgFunc.h)

set(private_headers
        lib/src/CatalogueImpl.h)

target_sources(${PROJECT_NAME} PRIVATE
        ${public_headers}
        ${private_headers}
        lib/src/Catalogue.cpp
        lib/src/CatalogueImpl.cpp)

find_package(PostgreSQL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PostgreSQL::PostgreSQL)
include_directories(${PostgreSQL_INCLUDE_DIR})

execute_process(COMMAND cat sql/get_meta.sql
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE META_SQL
        OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/MetaSql.h.in"
        "${PROJECT_SOURCE_DIR}/lib/src/MetaSql.h" @ONLY)

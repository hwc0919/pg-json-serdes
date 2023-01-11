cmake_minimum_required(VERSION 3.10)
project(pg_func_serdes)

set(CMAKE_CXX_STANDARD 11)

add_library(pg_func_serdes lib/src/PgFunc.cpp lib/inc/pfs/utils/StringBuffer.h)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/inc>)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_SOURCE_DIR}/lib/inc)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/lib/src)
aux_source_directory(${CMAKE_SOURCE_DIR}/lib/src SRC_DIR)

set(public_headers
        lib/inc/pfs/PgField.h
        lib/inc/pfs/PgType.h
        lib/inc/pfs/PgFunc.h
        lib/inc/pfs/Catalogue.h
        lib/inc/pfs/IBuffer.h
        lib/inc/pfs/Cursor.h
        lib/inc/pfs/IParamSetter.h
        lib/inc/pfs/IResult.h
        lib/inc/pfs/IPgReader.h
        lib/inc/pfs/IPgWriter.h

        lib/inc/pfs/utils/GeneralParamSetter.h
        lib/inc/pfs/utils/PgResultWrapper.h
        lib/inc/pfs/utils/PgTextReader.h
        lib/inc/pfs/utils/PgTextWriter.h
        lib/inc/pfs/utils/RawCursor.h
        lib/inc/pfs/utils/StringBuffer.h
        )

#set(private_headers
#        lib/src/CatalogueImpl.h)
#
#target_sources(${PROJECT_NAME} PRIVATE
#        ${public_headers}
#        ${private_headers}
#        lib/src/Catalogue.cpp
#        lib/src/CatalogueImpl.cpp)

target_sources(${PROJECT_NAME}
        PRIVATE
        ${public_headers}
        ${SRC_DIR})

find_package(PostgreSQL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PostgreSQL::PostgreSQL)
include_directories(${PostgreSQL_INCLUDE_DIR})

find_package(nlohmann_json REQUIRED)
include_directories(${nlohmann_json_INCLUDE_DIR})

execute_process(COMMAND cat sql/get_meta.sql
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE META_SQL
        OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/MetaSql.h.in"
        "${PROJECT_SOURCE_DIR}/lib/src/MetaSql.h" @ONLY)

add_subdirectory(tests)

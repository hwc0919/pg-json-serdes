cmake_minimum_required(VERSION 3.10)
project(pg_json_serdes)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/inc>)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_SOURCE_DIR}/lib/inc)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/lib/src)
aux_source_directory(${CMAKE_SOURCE_DIR}/lib/src SRC_DIR)

set(public_headers
        lib/inc/pg_json/Buffer.h
        lib/inc/pg_json/Catalogue.h
        lib/inc/pg_json/Converter.h
        lib/inc/pg_json/Cursor.h
        lib/inc/pg_json/json.h
        lib/inc/pg_json/PgField.h
        lib/inc/pg_json/PgFormat.h
        lib/inc/pg_json/PgFunc.h
        lib/inc/pg_json/PgParamSetter.h
        lib/inc/pg_json/PgReader.h
        lib/inc/pg_json/PgResult.h
        lib/inc/pg_json/PgType.h
        lib/inc/pg_json/PgWriter.h
        lib/inc/pg_json/utils/GeneralParamSetter.h
        lib/inc/pg_json/utils/PgResultWrapper.h
        lib/inc/pg_json/utils/RawCursor.h
        lib/inc/pg_json/utils/StringBuffer.h)

set(private_headers
        lib/src/ByteOrder.h
        lib/src/CatalogueImpl.h
        lib/src/ConverterImpl.h
        lib/src/MetaSql.h
        lib/src/PgBinaryReader.h
        lib/src/PgBinaryWriter.h
        lib/src/PgFuncImpl.h
        lib/src/ConverterImpl.h
        lib/src/PgTextReader.h
        lib/src/PgTextWriter.h
        lib/src/Scope.h)

set(PJ_SRC
        lib/src/Catalogue.cpp
        lib/src/CatalogueImpl.cpp
        lib/src/Converter.cpp
        lib/src/ConverterImpl.cpp
        lib/src/PgBinaryReader.cpp
        lib/src/PgBinaryWriter.cpp
        lib/src/PgFuncImpl.cpp
        lib/src/PgReader.cpp
        lib/src/PgTextReader.cpp
        lib/src/PgTextWriter.cpp
        lib/src/PgWriter.cpp)

target_sources(${PROJECT_NAME} PRIVATE
        ${public_headers}
        ${private_headers}
        ${PJ_SRC})

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

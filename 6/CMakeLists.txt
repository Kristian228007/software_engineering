cmake_minimum_required(VERSION 3.10)
project(RecipeAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED Foundation Net Util JSON Data DataPostgreSQL JWT Redis)

include_directories(${POCO_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCES
    src/main.cpp
)

add_executable(server ${SOURCES})

target_link_libraries(server
    ${POCO_LIBRARIES}
    PocoFoundation
    PocoNet
    PocoUtil
    PocoJSON
    PocoData
    PocoDataPostgreSQL
    PocoJWT
    PocoRedis
    pq
)
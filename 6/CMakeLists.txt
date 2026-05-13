cmake_minimum_required(VERSION 3.10)
project(RecipeAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED Foundation Net Util JSON Data DataPostgreSQL JWT Redis)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RDKAFKA REQUIRED rdkafka)

include_directories(${POCO_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${RDKAFKA_INCLUDE_DIRS})

option(BUILD_API "Build API server" ON)
option(BUILD_CONSUMER "Build Kafka consumer" OFF)

if(BUILD_API)
    set(API_SOURCES
        src/main.cpp
        src/service/RecipeService.cpp
        src/kafka/RecipeProducer.cpp
    )

    add_executable(server ${API_SOURCES})

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
        ${RDKAFKA_LIBRARIES}
    )
endif()

if(BUILD_CONSUMER)
    set(CONSUMER_SOURCES
        src/consumer/main.cpp
        src/kafka/RecipeConsumer.cpp
    )

    add_executable(consumer ${CONSUMER_SOURCES})

    target_link_libraries(consumer
        ${POCO_LIBRARIES}
        PocoFoundation
        PocoJSON
        PocoData
        PocoDataPostgreSQL
        pq
        ${RDKAFKA_LIBRARIES}
    )
endif()

cmake_minimum_required(VERSION 3.16)
project(RecipeAPI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Poco components
find_package(Poco REQUIRED COMPONENTS 
    Net 
    Foundation 
    Util 
    JSON 
    MongoDB 
    Data 
    DataPostgreSQL
    JWT
)

# Add PostgreSQL client
find_package(PostgreSQL REQUIRED)

add_executable(server
    src/main.cpp
    src/db/MongoInitializer.hpp
)

target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(server
    PRIVATE
        Poco::Net
        Poco::Foundation
        Poco::Util
        Poco::JSON
        Poco::MongoDB
        Poco::Data
        Poco::DataPostgreSQL
        Poco::JWT
        PostgreSQL::PostgreSQL
)

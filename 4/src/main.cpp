#include "app/Application.hpp"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <iostream>

#include "db/database.h"
#include "db/MongoInitializer.hpp"

int main(int argc, char **argv)
{
    // Register PostgreSQL connector
    Poco::Data::PostgreSQL::Connector::registerConnector();

    // Initialize MongoDB
    try
    {
        db::Database::instance().initMongo("mongodb", 27017, "recipe_db");
        std::cout << "MongoDB connected successfully" << std::endl;

        // Initialize MongoDB with test data
        MongoInitializer::initialize();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Mongo init failed: " << e.what() << std::endl;
    }

    // Run application
    ServerApplication app;
    int result = app.run(argc, argv);

    Poco::Data::PostgreSQL::Connector::unregisterConnector();

    return result;
}
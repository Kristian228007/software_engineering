#include "app/Application.hpp"
#include <Poco/Data/PostgreSQL/Connector.h>

int main(int argc, char **argv)
{
    Poco::Data::PostgreSQL::Connector::registerConnector();

    ServerApplication app;
    int result = app.run(argc, argv);

    Poco::Data::PostgreSQL::Connector::unregisterConnector();

    return result;
}
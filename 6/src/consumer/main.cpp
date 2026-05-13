#include "../kafka/RecipeConsumer.hpp"
#include <Poco/Environment.h>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <iostream>
#include <csignal>

std::unique_ptr<RecipeConsumer> consumer;

void signalHandler(int)
{
    std::cout << "Shutting down consumer..." << std::endl;
    if (consumer)
        consumer->stop();
}

int main()
{
    std::string dbHost = Poco::Environment::get("DB_HOST", "postgres");
    std::string dbPort = Poco::Environment::get("DB_PORT", "5432");
    std::string dbUser = Poco::Environment::get("DB_USER", "recipe_user");
    std::string dbPassword = Poco::Environment::get("DB_PASSWORD", "recipe_pass");
    std::string dbName = Poco::Environment::get("DB_NAME", "recipe_db");
    std::string kafkaBrokers = Poco::Environment::get("KAFKA_BROKERS", "kafka:9092");

    std::string connectionString = "host=" + dbHost +
                                   " port=" + dbPort +
                                   " user=" + dbUser +
                                   " password=" + dbPassword +
                                   " dbname=" + dbName;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Starting recipe consumer..." << std::endl;
    std::cout << "Kafka brokers: " << kafkaBrokers << std::endl;
    std::cout << "Database: " << dbHost << ":" << dbPort << "/" << dbName << std::endl;

    consumer = std::make_unique<RecipeConsumer>(kafkaBrokers, "recipe_events", connectionString);
    consumer->start();

    // Бесконечное ожидание
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
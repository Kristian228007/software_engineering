#pragma once
#include <librdkafka/rdkafka.h>
#include <string>
#include <memory>
#include <atomic>
#include <thread>

class RecipeConsumer
{
private:
    rd_kafka_t *rk;
    rd_kafka_topic_t *rt;
    std::string topic;
    std::string connectionString;
    std::atomic<bool> running;
    std::unique_ptr<std::thread> workerThread;

    void processMessage(const rd_kafka_message_t *msg);
    void saveRecipeToDB(const std::string &payload);
    void saveIngredientToDB(const std::string &payload);

public:
    RecipeConsumer(const std::string &brokers,
                   const std::string &topicName,
                   const std::string &dbConnStr);
    ~RecipeConsumer();

    void start();
    void stop();
    void runLoop();
};

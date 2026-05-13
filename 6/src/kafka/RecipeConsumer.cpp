#include "RecipeConsumer.hpp"
#include <Poco/JSON/Parser.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <iostream>

RecipeConsumer::RecipeConsumer(const std::string &brokers,
                               const std::string &topicName,
                               const std::string &dbConnStr)
    : topic(topicName), connectionString(dbConnStr), running(false)
{
    rd_kafka_conf_t *conf = rd_kafka_conf_new();

    char errbuf[512];
    if (rd_kafka_conf_set(conf, "metadata.broker.list", brokers.c_str(),
                         errbuf, sizeof(errbuf)) != RD_KAFKA_CONF_OK)
    {
        std::cerr << "Kafka config error: " << errbuf << std::endl;
    }

    rd_kafka_conf_set(conf, "group.id", "recipe_consumer_group", errbuf, sizeof(errbuf));
    rd_kafka_conf_set(conf, "enable.auto.commit", "true", errbuf, sizeof(errbuf));
    rd_kafka_conf_set(conf, "auto.offset.reset", "earliest", errbuf, sizeof(errbuf));

    rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errbuf, sizeof(errbuf));
    if (!rk)
    {
        std::cerr << "Failed to create Kafka consumer: " << errbuf << std::endl;
    }

    rt = rd_kafka_topic_new(rk, topic.c_str(), nullptr);
}

RecipeConsumer::~RecipeConsumer()
{
    stop();
    if (rt)
        rd_kafka_topic_destroy(rt);
    if (rk)
        rd_kafka_destroy(rk);
}

void RecipeConsumer::start()
{
    running = true;
    workerThread = std::make_unique<std::thread>(&RecipeConsumer::runLoop, this);
}

void RecipeConsumer::stop()
{
    running = false;
    if (workerThread && workerThread->joinable())
        workerThread->join();
}

void RecipeConsumer::runLoop()
{
    Poco::Data::PostgreSQL::Connector::registerConnector();

    rd_kafka_topic_partition_list_t *topics = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(topics, topic.c_str(), RD_KAFKA_PARTITION_UA);

    rd_kafka_subscribe(rk, topics);
    rd_kafka_topic_partition_list_destroy(topics);

    while (running)
    {
        rd_kafka_message_t *msg = rd_kafka_consumer_poll(rk, 1000);
        if (!msg)
            continue;

        if (msg->err)
        {
            std::cerr << "Kafka error: " << rd_kafka_err2str(msg->err) << std::endl;
            rd_kafka_message_destroy(msg);
            continue;
        }

        processMessage(msg);
        rd_kafka_message_destroy(msg);
    }

    rd_kafka_unsubscribe(rk);
    rd_kafka_consumer_close(rk);
    Poco::Data::PostgreSQL::Connector::unregisterConnector();
}

void RecipeConsumer::processMessage(const rd_kafka_message_t *msg)
{
    try
    {
        std::string payload(reinterpret_cast<const char *>(msg->payload), msg->len);

        Poco::JSON::Parser parser;
        auto json = parser.parse(payload).extract<Poco::JSON::Object::Ptr>();

        std::string eventType = json->getValue<std::string>("event_type");

        if (eventType == "RECIPE_CREATED")
        {
            saveRecipeToDB(payload);
        }
        else if (eventType == "INGREDIENT_ADDED")
        {
            saveIngredientToDB(payload);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

void RecipeConsumer::saveRecipeToDB(const std::string &payload)
{
    Poco::JSON::Parser parser;
    auto json = parser.parse(payload).extract<Poco::JSON::Object::Ptr>();

    Poco::Data::Session session("PostgreSQL", connectionString);

    std::string id = json->getValue<std::string>("recipe_id");
    std::string title = json->getValue<std::string>("title");
    std::string description = json->getValue<std::string>("description");
    std::string authorId = json->getValue<std::string>("author_id");

    try
    {
        Poco::Data::Statement insert(session);
        insert << "INSERT INTO recipes (id, title, description, author_id) "
               << "VALUES ($1, $2, $3, $4) ON CONFLICT (id) DO NOTHING",
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::use(title),
            Poco::Data::Keywords::use(description),
            Poco::Data::Keywords::use(authorId);
        insert.execute();

        std::cout << "Saved recipe: " << title << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to save recipe: " << e.what() << std::endl;
    }
}

void RecipeConsumer::saveIngredientToDB(const std::string &payload)
{
    Poco::JSON::Parser parser;
    auto json = parser.parse(payload).extract<Poco::JSON::Object::Ptr>();

    Poco::Data::Session session("PostgreSQL", connectionString);

    std::string id = json->getValue<std::string>("ingredient_id");
    std::string recipeId = json->getValue<std::string>("recipe_id");
    std::string name = json->getValue<std::string>("name");
    std::string amount = json->getValue<std::string>("amount");
    std::string unit = json->getValue<std::string>("unit");

    try
    {
        Poco::Data::Statement insert(session);
        insert << "INSERT INTO ingredients (id, recipe_id, name, amount, unit) "
               << "VALUES ($1, $2, $3, $4, $5) ON CONFLICT (id) DO NOTHING",
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::use(recipeId),
            Poco::Data::Keywords::use(name),
            Poco::Data::Keywords::use(amount),
            Poco::Data::Keywords::use(unit);
        insert.execute();

        std::cout << "Saved ingredient: " << name << " for recipe " << recipeId << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to save ingredient: " << e.what() << std::endl;
    }
}
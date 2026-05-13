#include "RecipeProducer.hpp"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Dynamic/Var.h>
#include <iostream>
#include <sstream>
#include <cstring>

RecipeProducer::RecipeProducer(const std::string &brokers, const std::string &topicName)
    : topic(topicName)
{
    rd_kafka_conf_t *conf = rd_kafka_conf_new();

    char errbuf[512];
    if (rd_kafka_conf_set(conf, "metadata.broker.list", brokers.c_str(),
                         errbuf, sizeof(errbuf)) != RD_KAFKA_CONF_OK)
    {
        std::cerr << "Kafka config error: " << errbuf << std::endl;
    }

    rd_kafka_conf_set(conf, "acks", "all", errbuf, sizeof(errbuf));

    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errbuf, sizeof(errbuf));
    if (!rk)
    {
        std::cerr << "Failed to create Kafka producer: " << errbuf << std::endl;
    }
}

RecipeProducer::~RecipeProducer()
{
    if (rk)
    {
        rd_kafka_flush(rk, 10000);
        rd_kafka_destroy(rk);
    }
}

void RecipeProducer::sendMessage(const std::string &payload)
{
    if (!rk) return;

    rd_kafka_resp_err_t err = rd_kafka_producev(
        rk,
        RD_KAFKA_V_TOPIC(topic.c_str()),
        RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
        RD_KAFKA_V_VALUE((void *)payload.c_str(), payload.size()),
        RD_KAFKA_V_END
    );

    if (err)
    {
        std::cerr << "Failed to produce message: " << rd_kafka_err2str(err) << std::endl;
    }

    rd_kafka_poll(rk, 0);
}

void RecipeProducer::produceRecipeCreated(const std::string &recipeId,
                                          const std::string &title,
                                          const std::string &description,
                                          const std::string &authorId)
{
    Poco::JSON::Object message;
    message.set("event_type", "RECIPE_CREATED");
    message.set("recipe_id", recipeId);
    message.set("title", title);
    message.set("description", description);
    message.set("author_id", authorId);
    message.set("timestamp", (int64_t)Poco::Timestamp().epochTime());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(message, oss);
    std::string payload = oss.str();
    sendMessage(payload);
}

void RecipeProducer::produceRecipeUpdated(const std::string &recipeId,
                                          const std::string &title,
                                          const std::string &description)
{
    Poco::JSON::Object message;
    message.set("event_type", "RECIPE_UPDATED");
    message.set("recipe_id", recipeId);
    message.set("title", title);
    message.set("description", description);
    message.set("timestamp", (int64_t)Poco::Timestamp().epochTime());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(message, oss);
    std::string payload = oss.str();
    sendMessage(payload);
}

void RecipeProducer::produceRecipeDeleted(const std::string &recipeId)
{
    Poco::JSON::Object message;
    message.set("event_type", "RECIPE_DELETED");
    message.set("recipe_id", recipeId);
    message.set("timestamp", (int64_t)Poco::Timestamp().epochTime());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(message, oss);
    std::string payload = oss.str();
    sendMessage(payload);
}

void RecipeProducer::produceIngredientAdded(const std::string &recipeId,
                                            const std::string &ingredientId,
                                            const std::string &name,
                                            const std::string &amount,
                                            const std::string &unit)
{
    Poco::JSON::Object message;
    message.set("event_type", "INGREDIENT_ADDED");
    message.set("recipe_id", recipeId);
    message.set("ingredient_id", ingredientId);
    message.set("name", name);
    message.set("amount", amount);
    message.set("unit", unit);
    message.set("timestamp", (int64_t)Poco::Timestamp().epochTime());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(message, oss);
    std::string payload = oss.str();
    sendMessage(payload);
}

void RecipeProducer::produceIngredientUpdated(const std::string &recipeId,
                                              const std::string &ingredientId,
                                              const std::string &name,
                                              const std::string &amount,
                                              const std::string &unit)
{
    Poco::JSON::Object message;
    message.set("event_type", "INGREDIENT_UPDATED");
    message.set("recipe_id", recipeId);
    message.set("ingredient_id", ingredientId);
    message.set("name", name);
    message.set("amount", amount);
    message.set("unit", unit);
    message.set("timestamp", (int64_t)Poco::Timestamp().epochTime());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(message, oss);
    std::string payload = oss.str();
    sendMessage(payload);
}

void RecipeProducer::produceIngredientDeleted(const std::string &recipeId,
                                              const std::string &ingredientId)
{
    Poco::JSON::Object message;
    message.set("event_type", "INGREDIENT_DELETED");
    message.set("recipe_id", recipeId);
    message.set("ingredient_id", ingredientId);
    message.set("timestamp", (int64_t)Poco::Timestamp().epochTime());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(message, oss);
    std::string payload = oss.str();
    sendMessage(payload);
}

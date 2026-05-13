#pragma once
#include <librdkafka/rdkafka.h>
#include <string>
#include <memory>

class RecipeProducer
{
private:
    rd_kafka_t *rk;
    std::string topic;

public:
    RecipeProducer(const std::string &brokers, const std::string &topicName);
    ~RecipeProducer();

    void produceRecipeCreated(const std::string &recipeId,
                              const std::string &title,
                              const std::string &description,
                              const std::string &authorId);

    void produceRecipeUpdated(const std::string &recipeId,
                              const std::string &title,
                              const std::string &description);

    void produceRecipeDeleted(const std::string &recipeId);

    void produceIngredientAdded(const std::string &recipeId,
                                const std::string &ingredientId,
                                const std::string &name,
                                const std::string &amount,
                                const std::string &unit);

    void produceIngredientUpdated(const std::string &recipeId,
                                  const std::string &ingredientId,
                                  const std::string &name,
                                  const std::string &amount,
                                  const std::string &unit);

    void produceIngredientDeleted(const std::string &recipeId,
                                  const std::string &ingredientId);

private:
    void sendMessage(const std::string &payload);
};

#pragma once
#include <stdexcept>
#include <vector>
#include <memory>
#include <Poco/UUIDGenerator.h>

#include "../repository/RecipeRepository.hpp"
#include "../repository/UserRepository.hpp"

#include "../domain/dto/RecipeDTO.hpp"
#include "../domain/dto/IngredientDTO.hpp"

#include "../http/exceptions/NotFoundException.hpp"
#include "../http/exceptions/ConflictException.hpp"

#include "../cache/CacheService.hpp"
#include "../utils/JsonUtils.hpp"
#include "../kafka/RecipeProducer.hpp"

class RecipeService
{
private:
    RecipeRepository &recipeRepo;
    UserRepository &userRepo;
    CacheService &cache;
    std::shared_ptr<RecipeProducer> producer;

public:
    RecipeService(RecipeRepository &r, UserRepository &u, CacheService &c);

    void setProducer(std::shared_ptr<RecipeProducer> p);

    // Команды (CQRS Command side - отправляют в Kafka)
    Recipe createRecipe(const std::string authorId, const CreateRecipeRequest &dto);
    Ingredient addIngredient(std::string recipe_id, const AddIngredientRequest &dto);
    void updateRecipe(const std::string &recipeId, const CreateRecipeRequest &dto);
    void deleteRecipe(const std::string &recipeId);
    void deleteIngredient(const std::string &recipeId, const std::string &ingredientId);
    Ingredient updateIngredient(const std::string &recipeId,
                                const std::string &ingredientId,
                                const AddIngredientRequest &dto);

    // Запросы (CQRS Query side - читают из кэша или БД)
    std::vector<Recipe> listRecipes();
    std::vector<Recipe> searchRecipes(const std::string &title);
    std::vector<Recipe> getRecipesByUserId(const std::string &userId);
    std::vector<Ingredient> getIngredients(std::string recipe_id);
    Recipe getRecipeById(const std::string &recipeId);
    std::vector<Recipe> searchRecipesByIngredients(const std::vector<std::string> &ingredientNames);
};
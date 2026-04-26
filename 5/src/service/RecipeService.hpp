#pragma once
#include <stdexcept>
#include <Poco/UUIDGenerator.h>

#include "../repository/RecipeRepository.hpp"
#include "../repository/UserRepository.hpp"

#include "../domain/dto/RecipeDTO.hpp"
#include "../domain/dto/IngredientDTO.hpp"

#include "../http/exceptions/NotFoundException.hpp"
#include "../http/exceptions/ConflictException.hpp"

#include "../cache/CacheService.hpp"
#include "../utils/JsonUtils.hpp"

class RecipeService
{
private:
    RecipeRepository &recipeRepo;
    UserRepository &userRepo;
    CacheService &cache;

public:
    RecipeService(RecipeRepository &r, UserRepository &u, CacheService &c)
        : recipeRepo(r), userRepo(u), cache(c) {}

    Recipe createRecipe(const std::string authorId, const CreateRecipeRequest &dto)
    {
        auto existing = recipeRepo.findByTitle(dto.title);
        if (!existing.empty())
            throw ConflictException("Recipe with this title already exists");

        Recipe r;
        r.title = dto.title;
        r.description = dto.description;
        r.authorId = authorId;

        auto created = recipeRepo.create(r);

        cache.del("recipes:all");
        return *created;
    }

    std::vector<Recipe> listRecipes()
    {
        auto cached = cache.get("recipes:all");

        if (cached)
        {
            return JsonUtils::jsonToRecipes(*cached);
        }

        auto ptrs = recipeRepo.getAll();

        std::vector<Recipe> result;
        for (auto &p : ptrs)
            result.push_back(*p);

        cache.set("recipes:all", JsonUtils::recipesToJson(result), 60);

        return result;
    }

    std::vector<Recipe> searchRecipes(const std::string &title)
    {
        auto ptrs = recipeRepo.findByTitle(title);
        std::vector<Recipe> result;
        result.reserve(ptrs.size());

        for (auto &p : ptrs)
            result.push_back(*p);

        return result;
    }

    std::vector<Recipe> getRecipesByUserId(const std::string &userId)
    {
        auto ptrs = recipeRepo.findByAuthorId(userId);
        std::vector<Recipe> result;
        result.reserve(ptrs.size());

        for (auto &p : ptrs)
            result.push_back(*p);

        return result;
    }

    Ingredient addIngredient(std::string recipe_id, const AddIngredientRequest &dto)
    {
        auto recipe = recipeRepo.findById(recipe_id);
        if (!recipe)
            throw NotFoundException("Recipe not found");

        if (recipeRepo.ingredientExists(recipe_id, dto.name))
            throw ConflictException("Ingredient already exists in this recipe");

        Ingredient i;
        i.name = dto.name;
        i.amount = dto.amount;
        i.unit = dto.unit;

        recipeRepo.addIngredient(recipe_id, i);

        auto ingredients = recipeRepo.findIngredientsByRecipeId(recipe_id);
        for (const auto &ing : ingredients)
        {
            if (ing.name == dto.name)
            {
                return ing;
            }
        }

        cache.del("recipes:all");
        cache.del("recipe:" + recipe_id + ":ingredients");

        return i;
    }

    std::vector<Ingredient> getIngredients(std::string recipe_id)
    {
        auto recipe = recipeRepo.findById(recipe_id);
        if (!recipe)
            throw NotFoundException("Recipe not found");

        return recipeRepo.findIngredientsByRecipeId(recipe_id);
    }
};
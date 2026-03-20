#pragma once
#include <stdexcept>

#include <Poco/UUIDGenerator.h>

#include "../repository/RecipeRepository.hpp"
#include "../repository/UserRepository.hpp"

#include "../domain/dto/RecipeDTO.hpp"
#include "../domain/dto/IngredientDTO.hpp"

#include "../http/exceptions/NotFoundException.hpp"
#include "../http/exceptions/ConflictException.hpp"

class RecipeService
{
private:
    RecipeRepository &recipeRepo;
    UserRepository &userRepo;

public:
    RecipeService(RecipeRepository &r, UserRepository &u)
        : recipeRepo(r), userRepo(u) {}

    Recipe createRecipe(const std::string authorId, const CreateRecipeRequest &dto)
    {
        auto existing = recipeRepo.findByTitle(dto.title);
        if (!existing.empty())
            throw ConflictException("Recipe already exists");

        Recipe r;
        r.title = dto.title;
        r.description = dto.description;
        r.authorId = authorId;

        return *recipeRepo.create(r);
    }

    std::vector<Recipe> listRecipes()
    {
        auto ptrs = recipeRepo.getAll();
        std::vector<Recipe> result;
        result.reserve(ptrs.size());

        for (auto &p : ptrs)
            result.push_back(*p);

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

        for (const auto &ingredient : recipe->ingredients)
        {
            if (ingredient.name == dto.name)
                throw ConflictException("Ingredient already exists");
        }

        Ingredient i;
        i.id = Poco::UUIDGenerator::defaultGenerator()
                   .createRandom()
                   .toString();
        i.name = dto.name;
        i.amount = dto.amount;
        i.unit = dto.unit;

        recipe->ingredients.push_back(i);
        return i;
    }

    std::vector<Ingredient> getIngredients(std::string recipe_id)
    {
        auto recipe = recipeRepo.findById(recipe_id);
        if (!recipe)
            throw NotFoundException();

        return recipe->ingredients;
    }
};
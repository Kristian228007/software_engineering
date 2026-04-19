#pragma once

#include <stdexcept>

#include "../repository/RecipeRepositoryMongo.hpp"
#include "../repository/UserRepository.hpp"

#include "../domain/dto/RecipeDTO.hpp"
#include "../domain/dto/IngredientDTO.hpp"

#include "../domain/entities/Recipe.hpp"

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

    Recipe createRecipe(const std::string &authorId, const CreateRecipeRequest &dto)
    {
        auto user = userRepo.findById(authorId);
        if (!user)
            throw NotFoundException("User not found");

        auto existing = recipeRepo.findByTitle(dto.title);
        if (!existing.empty())
            throw ConflictException("Recipe with this title already exists");

        Recipe r;
        r.title = dto.title;
        r.description = dto.description;
        r.authorId = authorId;

        Recipe created = recipeRepo.create(r);
        return created;
    }

    std::vector<Recipe> listRecipes()
    {
        return recipeRepo.getAll();
    }

    std::vector<Recipe> searchRecipes(const std::string &title)
    {
        auto ptrs = recipeRepo.findByTitle(title);

        std::vector<Recipe> result;
        result.reserve(ptrs.size());

        for (auto &p : ptrs)
            result.push_back(p);

        return result;
    }

    std::vector<Recipe> getRecipesByUserId(const std::string &userId)
    {
        auto ptrs = recipeRepo.findByAuthorId(userId);

        std::vector<Recipe> result;
        result.reserve(ptrs.size());

        for (auto &p : ptrs)
            result.push_back(p);

        return result;
    }

    Ingredient addIngredient(const std::string &recipe_id, const AddIngredientRequest &dto)
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

        return i;
    }

    std::vector<Ingredient> getIngredients(const std::string &recipe_id)
    {
        auto recipe = recipeRepo.findById(recipe_id);
        if (!recipe)
            throw NotFoundException("Recipe not found");

        return recipeRepo.findIngredientsByRecipeId(recipe_id);
    }
};
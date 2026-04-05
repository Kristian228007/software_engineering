#pragma once
#include <vector>
#include <mutex>
#include <optional>
#include <functional>
#include <memory>
#include <Poco/UUIDGenerator.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>

#include "../domain/entities/Recipe.hpp"
#include "../domain/entities/Ingredient.hpp"

class RecipeRepository
{
private:
    Poco::Data::Session session_;
    std::mutex mutex_;

    std::vector<Ingredient> parseIngredients(const std::string &jsonStr)
    {
        std::vector<Ingredient> ingredients;
        if (jsonStr.empty() || jsonStr == "[]")
            return ingredients;

        Poco::JSON::Parser parser;
        auto arr = parser.parse(jsonStr).extract<Poco::JSON::Array::Ptr>();

        for (size_t i = 0; i < arr->size(); ++i)
        {
            auto obj = arr->getObject(i);
            Ingredient ing;
            ing.id = obj->getValue<std::string>("id");
            ing.name = obj->getValue<std::string>("name");
            ing.amount = obj->getValue<std::string>("amount");
            ing.unit = obj->getValue<std::string>("unit");
            ingredients.push_back(ing);
        }
        return ingredients;
    }

public:
    RecipeRepository(const std::string &connectionString)
        : session_("PostgreSQL", connectionString)
    {
    }

    std::shared_ptr<Recipe> create(Recipe r)
    {
        std::lock_guard lock(mutex_);

        r.id = Poco::UUIDGenerator::defaultGenerator()
                   .createRandom()
                   .toString();

        Poco::Data::Statement insert(session_);

        std::string id = r.id;
        std::string title = r.title;
        std::string desc = r.description;
        std::string author = r.authorId;

        insert << "INSERT INTO recipes (id, title, description, author_id) VALUES ($1, $2, $3, $4)",
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::use(title),
            Poco::Data::Keywords::use(desc),
            Poco::Data::Keywords::use(author);
        insert.execute();

        auto ptr = std::make_shared<Recipe>(std::move(r));
        return ptr;
    }

    std::vector<std::shared_ptr<Recipe>> getAll()
    {
        std::lock_guard lock(mutex_);

        std::vector<std::shared_ptr<Recipe>> recipes;
        Poco::Data::Statement select(session_);
        std::string id, title, description, authorId, ingredientsJson;

        select << "SELECT r.id, r.title, r.description, r.author_id, "
                  "COALESCE(json_agg(json_build_object("
                  "'id', i.id, 'name', i.name, 'amount', i.amount, 'unit', i.unit"
                  ")) FILTER (WHERE i.id IS NOT NULL), '[]') as ingredients "
                  "FROM recipes r "
                  "LEFT JOIN ingredients i ON r.id = i.recipe_id "
                  "GROUP BY r.id",
            Poco::Data::Keywords::into(id),
            Poco::Data::Keywords::into(title),
            Poco::Data::Keywords::into(description),
            Poco::Data::Keywords::into(authorId),
            Poco::Data::Keywords::into(ingredientsJson),
            Poco::Data::Keywords::range(0, 1);

        while (!select.done())
        {
            if (select.execute())
            {
                auto recipe = std::make_shared<Recipe>();
                recipe->id = id;
                recipe->title = title;
                recipe->description = description;
                recipe->authorId = authorId;
                recipe->ingredients = parseIngredients(ingredientsJson);

                recipes.push_back(recipe);
            }
        }

        return recipes;
    }

    std::shared_ptr<Recipe> findById(const std::string &id)
    {
        std::lock_guard lock(mutex_);

        Poco::Data::Statement select(session_);
        std::string title, description, authorId, ingredientsJson;

        std::string tmp_id = id;

        select << "SELECT r.title, r.description, r.author_id, "
                  "COALESCE(json_agg(json_build_object("
                  "'id', i.id, 'name', i.name, 'amount', i.amount, 'unit', i.unit"
                  ")) FILTER (WHERE i.id IS NOT NULL), '[]') as ingredients "
                  "FROM recipes r "
                  "LEFT JOIN ingredients i ON r.id = i.recipe_id "
                  "WHERE r.id = $1 "
                  "GROUP BY r.id",
            Poco::Data::Keywords::into(title),
            Poco::Data::Keywords::into(description),
            Poco::Data::Keywords::into(authorId),
            Poco::Data::Keywords::into(ingredientsJson),
            Poco::Data::Keywords::use(tmp_id);

        select.execute();
        if (select.rowsExtracted() > 0)
        {
            auto recipe = std::make_shared<Recipe>();
            recipe->id = id;
            recipe->title = title;
            recipe->description = description;
            recipe->authorId = authorId;
            recipe->ingredients = parseIngredients(ingredientsJson);
            return recipe;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Recipe>> findByTitle(const std::string &title)
    {
        std::lock_guard lock(mutex_);

        std::vector<std::shared_ptr<Recipe>> result;
        Poco::Data::Statement select(session_);
        std::string id, recTitle, description, authorId;

        std::string searchPattern = "%" + title + "%";
        select << "SELECT id, title, description, author_id FROM recipes WHERE title ILIKE $1",
            Poco::Data::Keywords::into(id),
            Poco::Data::Keywords::into(recTitle),
            Poco::Data::Keywords::into(description),
            Poco::Data::Keywords::into(authorId),
            Poco::Data::Keywords::use(searchPattern),
            Poco::Data::Keywords::range(0, 1);

        while (!select.done())
        {
            if (select.execute())
            {
                auto recipe = std::make_shared<Recipe>();
                recipe->id = id;
                recipe->title = recTitle;
                recipe->description = description;
                recipe->authorId = authorId;
                recipe->ingredients = findIngredientsByRecipeId(id);
                result.push_back(recipe);
            }
        }

        return result;
    }

    std::vector<std::shared_ptr<Recipe>> findByAuthorId(const std::string &authorId)
    {
        std::lock_guard lock(mutex_);

        std::vector<std::shared_ptr<Recipe>> result;
        Poco::Data::Statement select(session_);
        std::string id, title, description;

        std::string tmpAuthorId = authorId;

        select << "SELECT id, title, description FROM recipes WHERE author_id = $1",
            Poco::Data::Keywords::into(id),
            Poco::Data::Keywords::into(title),
            Poco::Data::Keywords::into(description),
            Poco::Data::Keywords::use(tmpAuthorId),
            Poco::Data::Keywords::range(0, 1);

        while (!select.done())
        {
            if (select.execute())
            {
                auto recipe = std::make_shared<Recipe>();
                recipe->id = id;
                recipe->title = title;
                recipe->description = description;
                recipe->authorId = authorId;
                recipe->ingredients = findIngredientsByRecipeId(id);
                result.push_back(recipe);
            }
        }

        return result;
    }

    std::vector<Ingredient> findIngredientsByRecipeId(const std::string &recipeId)
    {
        std::vector<Ingredient> ingredients;
        Poco::Data::Statement select(session_);
        Ingredient ing;

        std::string tmp = recipeId;

        select << "SELECT id, name, amount, unit FROM ingredients WHERE recipe_id = $1",
            Poco::Data::Keywords::into(ing.id),
            Poco::Data::Keywords::into(ing.name),
            Poco::Data::Keywords::into(ing.amount),
            Poco::Data::Keywords::into(ing.unit),
            Poco::Data::Keywords::use(tmp),
            Poco::Data::Keywords::range(0, 1);

        while (!select.done())
        {
            if (select.execute())
            {
                ingredients.push_back(ing);
            }
        }

        return ingredients;
    }

    void addIngredient(const std::string &recipeId, const Ingredient &ingredient)
    {
        Ingredient ing = ingredient;
        ing.id = Poco::UUIDGenerator::defaultGenerator()
                     .createRandom()
                     .toString();

        Poco::Data::Statement insert(session_);

        std::string id = ing.id;
        std::string recipe_id = recipeId;
        std::string name = ing.name;
        std::string amount = ing.amount;
        std::string unit = ing.unit;

        insert << "INSERT INTO ingredients (id, recipe_id, name, amount, unit) VALUES ($1, $2, $3, $4, $5)",
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::use(recipe_id),
            Poco::Data::Keywords::use(name),
            Poco::Data::Keywords::use(amount),
            Poco::Data::Keywords::use(unit);
        insert.execute();
    }

    bool ingredientExists(const std::string &recipeId, const std::string &name)
    {
        Poco::Data::Statement select(session_);
        int count = 0;
        auto tmp_rec = std::string(recipeId);
        auto tmp_name = std::string(name);

        select << "SELECT COUNT(*) FROM ingredients WHERE recipe_id = $1 AND name = $2",
            Poco::Data::Keywords::into(count),
            Poco::Data::Keywords::use(tmp_rec),
            Poco::Data::Keywords::use(tmp_name);
        select.execute();
        return count > 0;
    }
};
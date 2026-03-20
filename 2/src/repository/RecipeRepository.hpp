#pragma once
#include <vector>
#include <mutex>
#include <optional>
#include <functional>

#include <Poco/UUIDGenerator.h>

#include "../domain/entities/Recipe.hpp"

class RecipeRepository
{
private:
    std::vector<std::shared_ptr<Recipe>> recipes;
    std::mutex mutex_;

public:
    std::shared_ptr<Recipe> create(Recipe r)
    {
        std::lock_guard lock(mutex_);

        r.id = Poco::UUIDGenerator::defaultGenerator()
                   .createRandom()
                   .toString();

        auto ptr = std::make_shared<Recipe>(std::move(r));
        recipes.push_back(ptr);
        return ptr;
    }

    std::vector<std::shared_ptr<Recipe>> getAll()
    {
        std::lock_guard lock(mutex_);
        return recipes;
    }

    std::shared_ptr<Recipe> findById(const std::string &id)
    {
        std::lock_guard lock(mutex_);
        for (auto &r : recipes)
            if (r->id == id)
                return r;
        return nullptr;
    }

    std::vector<std::shared_ptr<Recipe>> findByTitle(const std::string &title)
    {
        std::lock_guard lock(mutex_);
        std::vector<std::shared_ptr<Recipe>> result;

        for (auto &r : recipes)
            if (r->title.find(title) != std::string::npos)
                result.push_back(r);

        return result;
    }

    std::vector<std::shared_ptr<Recipe>> findByAuthorId(const std::string &authorId)
    {
        std::lock_guard lock(mutex_);
        std::vector<std::shared_ptr<Recipe>> result;

        for (auto &r : recipes)
            if (r->authorId == authorId)
                result.push_back(r);

        return result;
    }
};
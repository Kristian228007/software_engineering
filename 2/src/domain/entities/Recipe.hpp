#pragma once
#include <string>
#include <vector>
#include "Ingredient.hpp"

struct Recipe
{
    std::string id;
    std::string title;
    std::string description;
    std::string authorId;
    std::vector<Ingredient> ingredients;
};
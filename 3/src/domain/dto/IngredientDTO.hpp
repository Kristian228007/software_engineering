#pragma once
#include <string>

struct AddIngredientRequest
{
    std::string name;
    std::string amount;
    std::string unit;
};
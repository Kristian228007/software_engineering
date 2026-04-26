#pragma once
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <sstream>

#include "../domain/entities/Recipe.hpp"

class JsonUtils
{
public:
    static std::string recipesToJson(const std::vector<Recipe> &recipes)
    {
        Poco::JSON::Array arr;

        for (const auto &r : recipes)
        {
            Poco::JSON::Object obj;
            obj.set("id", r.id);
            obj.set("title", r.title);
            obj.set("description", r.description);
            obj.set("authorId", r.authorId);

            Poco::JSON::Array ingArr;
            for (const auto &i : r.ingredients)
            {
                Poco::JSON::Object ing;
                ing.set("id", i.id);
                ing.set("name", i.name);
                ing.set("amount", i.amount);
                ing.set("unit", i.unit);
                ingArr.add(ing);
            }

            obj.set("ingredients", ingArr);
            arr.add(obj);
        }

        std::stringstream ss;
        arr.stringify(ss);
        return ss.str();
    }

    static std::vector<Recipe> jsonToRecipes(const std::string &json)
    {
        std::vector<Recipe> result;

        Poco::JSON::Parser parser;
        auto arr = parser.parse(json).extract<Poco::JSON::Array::Ptr>();

        for (size_t i = 0; i < arr->size(); ++i)
        {
            auto obj = arr->getObject(i);

            Recipe r;
            r.id = obj->getValue<std::string>("id");
            r.title = obj->getValue<std::string>("title");
            r.description = obj->getValue<std::string>("description");
            r.authorId = obj->getValue<std::string>("authorId");

            auto ingArr = obj->getArray("ingredients");
            for (size_t j = 0; j < ingArr->size(); ++j)
            {
                auto ingObj = ingArr->getObject(j);
                Ingredient ing;
                ing.id = ingObj->getValue<std::string>("id");
                ing.name = ingObj->getValue<std::string>("name");
                ing.amount = ingObj->getValue<std::string>("amount");
                ing.unit = ingObj->getValue<std::string>("unit");
                r.ingredients.push_back(ing);
            }

            result.push_back(r);
        }

        return result;
    }
};
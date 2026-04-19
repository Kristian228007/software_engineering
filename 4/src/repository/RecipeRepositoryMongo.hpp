#pragma once

#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

#include <Poco/MongoDB/Document.h>
#include <Poco/MongoDB/Element.h>
#include <Poco/MongoDB/OpMsgMessage.h>
#include <Poco/MongoDB/Array.h>

#include "../db/database.h"

#include "../domain/entities/Recipe.hpp"
#include "../domain/entities/Ingredient.hpp"

#include <optional>
#include <vector>
#include <string>

class RecipeRepository
{
private:
    Recipe recipeFromDoc(const Poco::MongoDB::Document::Ptr &doc)
    {
        Recipe r;

        r.id = doc->get("id")->toString();
        r.title = doc->get("title")->toString();
        r.description = doc->get("description")->toString();
        r.authorId = doc->get("author_id")->toString();

        if (doc->exists("ingredients"))
        {
            auto el = doc->get("ingredients");

            auto arr = dynamic_cast<Poco::MongoDB::Array *>(el.get());

            if (arr)
            {
                for (int i = 0; i < arr->size(); ++i)
                {
                    auto item = arr->get(i);
                    auto obj = dynamic_cast<Poco::MongoDB::Document *>(item.get());

                    if (!obj)
                        continue;

                    Ingredient ing;
                    ing.id = obj->get("id")->toString();
                    ing.name = obj->get("name")->toString();
                    ing.amount = obj->get("amount")->toString();
                    ing.unit = obj->get("unit")->toString();

                    r.ingredients.push_back(ing);
                }
            }
        }

        return r;
    }

    inline Poco::JSON::Object::Ptr toJson(const Recipe &r)
    {
        auto obj = new Poco::JSON::Object();
        obj->set("id", r.id);
        obj->set("title", r.title);
        obj->set("description", r.description);
        obj->set("author_id", r.authorId);

        Poco::JSON::Array ingredientsArr;
        for (const auto &ing : r.ingredients)
        {
            Poco::JSON::Object ingObj;
            ingObj.set("id", ing.id);
            ingObj.set("name", ing.name);
            ingObj.set("amount", ing.amount);
            ingObj.set("unit", ing.unit);
            ingredientsArr.add(ingObj);
        }
        obj->set("ingredients", ingredientsArr);

        return obj;
    }

    std::vector<Recipe> findMany(const std::string &field, const std::string &value)
    {
        std::vector<Recipe> result;

        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_FIND);

        req->body()
            .addNewDocument("filter")
            .add(field, value);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);
        db::mongoEnsureOk(resp, "findMany");

        for (const auto &doc : resp.documents())
        {
            result.push_back(recipeFromDoc(doc));
        }

        return result;
    }

public:
    Recipe create(const Recipe &recipe)
    {
        Recipe r = recipe;
        r.id = Poco::UUIDGenerator::defaultGenerator().createOne().toString();

        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_INSERT);

        Poco::MongoDB::Document::Ptr doc = new Poco::MongoDB::Document();
        doc->add("id", r.id);
        doc->add("title", r.title);
        doc->add("description", r.description);
        doc->add("author_id", r.authorId);

        req->documents().push_back(doc);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);
        db::mongoEnsureOk(resp, "recipe insert");

        return r;
    }

    std::vector<Recipe> getAll()
    {
        std::vector<Recipe> result;

        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_FIND);

        req->body().addNewDocument("filter");

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);
        db::mongoEnsureOk(resp, "getAll");

        for (const auto &doc : resp.documents())
        {
            result.push_back(recipeFromDoc(doc));
        }

        return result;
    }

    std::optional<Recipe> findById(const std::string &id)
    {
        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_FIND);

        req->body()
            .add("limit", 1)
            .addNewDocument("filter")
            .add("id", id);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);
        db::mongoEnsureOk(resp, "recipe findById");

        if (resp.documents().empty())
            return std::nullopt;

        return recipeFromDoc(resp.documents()[0]);
    }

    std::vector<Recipe> findByTitle(const std::string &title)
    {
        return findMany("title", title);
    }

    std::vector<Recipe> findByAuthorId(const std::string &authorId)
    {
        return findMany("author_id", authorId);
    }

    bool ingredientExists(const std::string &recipeId, const std::string &name)
    {
        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_FIND);

        req->body()
            .add("limit", 1)
            .addNewDocument("filter")
            .add("id", recipeId)
            .add("ingredients.name", name);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);

        db::mongoEnsureOk(resp, "ingredientExists");

        return !resp.documents().empty();
    }

    void addIngredient(const std::string &recipeId, const Ingredient &i)
    {
        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_UPDATE);

        Poco::MongoDB::Document::Ptr spec = new Poco::MongoDB::Document();

        spec->addNewDocument("q").add("id", recipeId);

        auto &setDoc = spec->addNewDocument("u")
                           .addNewDocument("$push");

        auto &ing = setDoc.addNewDocument("ingredients");
        ing.add("id", Poco::UUIDGenerator::defaultGenerator().createOne().toString());
        ing.add("name", i.name);
        ing.add("amount", i.amount);
        ing.add("unit", i.unit);

        spec->add("multi", false);
        spec->add("upsert", false);

        req->documents().push_back(spec);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);
        db::mongoEnsureOk(resp, "addIngredient");
    }

    std::vector<Ingredient> findIngredientsByRecipeId(const std::string &recipeId)
    {
        auto r = findById(recipeId);
        if (!r)
            return {};
        return r->ingredients;
    }
};
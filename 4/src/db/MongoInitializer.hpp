#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "database.h"
#include "../domain/entities/Recipe.hpp"
#include "../domain/entities/Ingredient.hpp"

class MongoInitializer
{
private:
    struct InitialIngredient
    {
        std::string id;
        std::string name;
        std::string amount;
        std::string unit;
    };

    struct InitialRecipe
    {
        std::string id;
        std::string title;
        std::string description;
        std::string authorId;
        std::vector<InitialIngredient> ingredients;
    };

    static bool isEmpty()
    {
        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_FIND);
        req->body().addNewDocument("filter");
        req->body().add("limit", 1);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);

        return resp.documents().empty();
    }

    static void createIndexes()
    {
        try
        {
            std::cout << "Creating MongoDB indexes..." << std::endl;
            db::Database &d = db::Database::instance();
            Poco::MongoDB::Database &mdb = d.mongoDb();

            // 1. Индекс для поиска по title (для быстрого поиска рецептов по названию)
            {
                auto req = mdb.createOpMsgMessage("recipes");
                req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_CREATE_INDEXES);

                Poco::MongoDB::Document::Ptr indexDoc = new Poco::MongoDB::Document();
                indexDoc->add("name", "idx_title");

                Poco::MongoDB::Document::Ptr keyDoc = new Poco::MongoDB::Document();
                keyDoc->add("title", 1); // 1 = ascending order
                indexDoc->add("key", keyDoc);

                req->documents().push_back(indexDoc);

                Poco::MongoDB::OpMsgMessage resp;
                d.send(*req, resp);
                db::mongoEnsureOk(resp, "create index on title");
                std::cout << "  Created index: idx_title" << std::endl;
            }

            // 2. Индекс для поиска по author_id (для получения рецептов пользователя)
            {
                auto req = mdb.createOpMsgMessage("recipes");
                req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_CREATE_INDEXES);

                Poco::MongoDB::Document::Ptr indexDoc = new Poco::MongoDB::Document();
                indexDoc->add("name", "idx_author_id");

                Poco::MongoDB::Document::Ptr keyDoc = new Poco::MongoDB::Document();
                keyDoc->add("author_id", 1);
                indexDoc->add("key", keyDoc);

                req->documents().push_back(indexDoc);

                Poco::MongoDB::OpMsgMessage resp;
                d.send(*req, resp);
                db::mongoEnsureOk(resp, "create index on author_id");
                std::cout << "  Created index: idx_author_id" << std::endl;
            }

            // 3. Текстовый индекс для полнотекстового поиска по title и description
            {
                auto req = mdb.createOpMsgMessage("recipes");
                req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_CREATE_INDEXES);

                Poco::MongoDB::Document::Ptr indexDoc = new Poco::MongoDB::Document();
                indexDoc->add("name", "idx_text_search");

                Poco::MongoDB::Document::Ptr keyDoc = new Poco::MongoDB::Document();
                keyDoc->add("title", "text");
                keyDoc->add("description", "text");
                indexDoc->add("key", keyDoc);

                req->documents().push_back(indexDoc);

                Poco::MongoDB::OpMsgMessage resp;
                d.send(*req, resp);
                db::mongoEnsureOk(resp, "create text index");
                std::cout << "  Created index: idx_text_search (full-text)" << std::endl;
            }

            // 4. Индекс для поиска ингредиентов по name
            {
                auto req = mdb.createOpMsgMessage("recipes");
                req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_CREATE_INDEXES);

                Poco::MongoDB::Document::Ptr indexDoc = new Poco::MongoDB::Document();
                indexDoc->add("name", "idx_ingredients_name");

                Poco::MongoDB::Document::Ptr keyDoc = new Poco::MongoDB::Document();
                keyDoc->add("ingredients.name", 1);
                indexDoc->add("key", keyDoc);

                req->documents().push_back(indexDoc);

                Poco::MongoDB::OpMsgMessage resp;
                d.send(*req, resp);
                db::mongoEnsureOk(resp, "create index on ingredients.name");
                std::cout << "  Created index: idx_ingredients_name" << std::endl;
            }

            // 5. Индекс для уникальности ингредиента в рецепте
            {
                auto req = mdb.createOpMsgMessage("recipes");
                req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_CREATE_INDEXES);

                Poco::MongoDB::Document::Ptr indexDoc = new Poco::MongoDB::Document();
                indexDoc->add("name", "idx_unique_ingredient");

                Poco::MongoDB::Document::Ptr keyDoc = new Poco::MongoDB::Document();
                keyDoc->add("id", 1);
                keyDoc->add("ingredients.name", 1);
                indexDoc->add("key", keyDoc);

                // Делаем индекс уникальным
                indexDoc->add("unique", true);
                indexDoc->add("sparse", true);

                req->documents().push_back(indexDoc);

                Poco::MongoDB::OpMsgMessage resp;
                d.send(*req, resp);
                db::mongoEnsureOk(resp, "create unique compound index");
                std::cout << "  Created index: idx_unique_ingredient (unique compound)" << std::endl;
            }

            std::cout << "All MongoDB indexes created successfully" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Warning: Some indexes may have failed to create: " << e.what() << std::endl;
        }
    }

    static void insertRecipe(const InitialRecipe &recipe)
    {
        db::Database &d = db::Database::instance();
        Poco::MongoDB::Database &mdb = d.mongoDb();

        auto req = mdb.createOpMsgMessage("recipes");
        req->setCommandName(Poco::MongoDB::OpMsgMessage::CMD_INSERT);

        Poco::MongoDB::Document::Ptr doc = new Poco::MongoDB::Document();
        doc->add("id", recipe.id);
        doc->add("title", recipe.title);
        doc->add("description", recipe.description);
        doc->add("author_id", recipe.authorId);

        // Добавляем ингредиенты
        if (!recipe.ingredients.empty())
        {
            Poco::MongoDB::Array::Ptr ingredientsArr = new Poco::MongoDB::Array();
            for (const auto &ing : recipe.ingredients)
            {
                Poco::MongoDB::Document::Ptr ingDoc = new Poco::MongoDB::Document();
                ingDoc->add("id", ing.id);
                ingDoc->add("name", ing.name);
                ingDoc->add("amount", ing.amount);
                ingDoc->add("unit", ing.unit);
                ingredientsArr->add(ingDoc);
            }
            doc->add("ingredients", ingredientsArr);
        }
        else
        {
            doc->add("ingredients", Poco::MongoDB::Array::Ptr(new Poco::MongoDB::Array()));
        }

        req->documents().push_back(doc);

        Poco::MongoDB::OpMsgMessage resp;
        d.send(*req, resp);
        db::mongoEnsureOk(resp, "insert recipe: " + recipe.title);
    }

public:
    static void initialize()
    {
        try
        {
            std::cout << "Checking MongoDB data..." << std::endl;

            bool empty = isEmpty();

            createIndexes();

            if (!empty)
            {
                std::cout << "MongoDB already contains data, skipping data initialization" << std::endl;
                return;
            }

            std::cout << "Initializing MongoDB with test data..." << std::endl;

            std::vector<InitialRecipe> recipes;

            // Recipe 1: Pasta Carbonara
            InitialRecipe recipe1;
            recipe1.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee1";
            recipe1.title = "Pasta Carbonara";
            recipe1.description = "Classic Italian pasta dish";
            recipe1.authorId = "11111111-1111-1111-1111-111111111111";

            InitialIngredient ing1_1 = {"11111111-1111-1111-1111-111111111001", "Spaghetti", "200", "g"};
            InitialIngredient ing1_2 = {"11111111-1111-1111-1111-111111111002", "Eggs", "2", "pcs"};
            InitialIngredient ing1_3 = {"11111111-1111-1111-1111-111111111003", "Pancetta", "100", "g"};
            recipe1.ingredients.push_back(ing1_1);
            recipe1.ingredients.push_back(ing1_2);
            recipe1.ingredients.push_back(ing1_3);
            recipes.push_back(recipe1);

            // Recipe 2: Chicken Curry
            InitialRecipe recipe2;
            recipe2.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee2";
            recipe2.title = "Chicken Curry";
            recipe2.description = "Spicy Indian curry";
            recipe2.authorId = "22222222-2222-2222-2222-222222222222";

            InitialIngredient ing2_1 = {"11111111-1111-1111-1111-111111111004", "Chicken", "300", "g"};
            InitialIngredient ing2_2 = {"11111111-1111-1111-1111-111111111005", "Curry paste", "2", "tbsp"};
            recipe2.ingredients.push_back(ing2_1);
            recipe2.ingredients.push_back(ing2_2);
            recipes.push_back(recipe2);

            // Recipe 3: Caesar Salad
            InitialRecipe recipe3;
            recipe3.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee3";
            recipe3.title = "Caesar Salad";
            recipe3.description = "Fresh romaine salad";
            recipe3.authorId = "33333333-3333-3333-3333-333333333333";

            InitialIngredient ing3_1 = {"11111111-1111-1111-1111-111111111006", "Lettuce", "1", "head"};
            InitialIngredient ing3_2 = {"11111111-1111-1111-1111-111111111007", "Croutons", "50", "g"};
            recipe3.ingredients.push_back(ing3_1);
            recipe3.ingredients.push_back(ing3_2);
            recipes.push_back(recipe3);

            // Recipe 4: Chocolate Cake
            InitialRecipe recipe4;
            recipe4.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee4";
            recipe4.title = "Chocolate Cake";
            recipe4.description = "Rich chocolate dessert";
            recipe4.authorId = "11111111-1111-1111-1111-111111111111";

            InitialIngredient ing4_1 = {"11111111-1111-1111-1111-111111111008", "Flour", "200", "g"};
            InitialIngredient ing4_2 = {"11111111-1111-1111-1111-111111111009", "Chocolate", "150", "g"};
            recipe4.ingredients.push_back(ing4_1);
            recipe4.ingredients.push_back(ing4_2);
            recipes.push_back(recipe4);

            // Recipe 5: Vegetable Soup
            InitialRecipe recipe5;
            recipe5.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee5";
            recipe5.title = "Vegetable Soup";
            recipe5.description = "Healthy vegetable soup";
            recipe5.authorId = "44444444-4444-4444-4444-444444444444";

            InitialIngredient ing5_1 = {"11111111-1111-1111-1111-111111111010", "Tomatoes", "400", "g"};
            recipe5.ingredients.push_back(ing5_1);
            recipes.push_back(recipe5);

            // Recipe 6: Sushi Roll
            InitialRecipe recipe6;
            recipe6.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee6";
            recipe6.title = "Sushi Roll";
            recipe6.description = "Japanese sushi";
            recipe6.authorId = "55555555-5555-5555-5555-555555555555";

            InitialIngredient ing6_1 = {"11111111-1111-1111-1111-111111111011", "Rice", "250", "g"};
            recipe6.ingredients.push_back(ing6_1);
            recipes.push_back(recipe6);

            // Recipe 7: Tacos
            InitialRecipe recipe7;
            recipe7.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee7";
            recipe7.title = "Tacos";
            recipe7.description = "Mexican tacos";
            recipe7.authorId = "66666666-6666-6666-6666-666666666666";

            InitialIngredient ing7_1 = {"11111111-1111-1111-1111-111111111012", "Tortilla", "4", "pcs"};
            recipe7.ingredients.push_back(ing7_1);
            recipes.push_back(recipe7);

            // Recipe 8: Omelette
            InitialRecipe recipe8;
            recipe8.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee8";
            recipe8.title = "Omelette";
            recipe8.description = "French omelette";
            recipe8.authorId = "77777777-7777-7777-7777-777777777777";

            InitialIngredient ing8_1 = {"11111111-1111-1111-1111-111111111013", "Eggs", "3", "pcs"};
            recipe8.ingredients.push_back(ing8_1);
            recipes.push_back(recipe8);

            // Recipe 9: Pancakes
            InitialRecipe recipe9;
            recipe9.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeee9";
            recipe9.title = "Pancakes";
            recipe9.description = "Fluffy breakfast pancakes";
            recipe9.authorId = "88888888-8888-8888-8888-888888888888";

            InitialIngredient ing9_1 = {"11111111-1111-1111-1111-111111111014", "Flour", "250", "g"};
            recipe9.ingredients.push_back(ing9_1);
            recipes.push_back(recipe9);

            // Recipe 10: Smoothie Bowl
            InitialRecipe recipe10;
            recipe10.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeee10";
            recipe10.title = "Smoothie Bowl";
            recipe10.description = "Healthy breakfast bowl";
            recipe10.authorId = "99999999-9999-9999-9999-999999999999";

            InitialIngredient ing10_1 = {"11111111-1111-1111-1111-111111111015", "Banana", "1", "pcs"};
            recipe10.ingredients.push_back(ing10_1);
            recipes.push_back(recipe10);

            // Recipe 11: Grilled Cheese
            InitialRecipe recipe11;
            recipe11.id = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeee11";
            recipe11.title = "Grilled Cheese";
            recipe11.description = "Classic grilled sandwich";
            recipe11.authorId = "aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa";
            recipes.push_back(recipe11);

            for (const auto &recipe : recipes)
            {
                insertRecipe(recipe);
                std::cout << "  Inserted: " << recipe.title << std::endl;
            }

            std::cout << "MongoDB initialization complete! Inserted " << recipes.size() << " recipes" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "MongoDB initialization warning: " << e.what() << std::endl;
        }
    }
};
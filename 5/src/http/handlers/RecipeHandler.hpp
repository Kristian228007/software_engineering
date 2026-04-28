#pragma once
#include "../BaseHandler.hpp"

#include "../../service/RecipeService.hpp"

#include "../../auth/JwtMiddleware.hpp"

#include "../../cache/RateLimiter.hpp"
#include "../../cache/CacheService.hpp"

class RecipeHandler : public BaseHandler
{
private:
    RecipeService &service;
    JwtMiddleware &auth;
    CacheService &cache;
    RateLimiter &rateLimiter;

public:
    RecipeHandler(RecipeService &s, JwtMiddleware &a,
                  CacheService &c, RateLimiter &rl)
        : service(s), auth(a), cache(c), rateLimiter(rl) {}

protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        if (req.getMethod() == "POST")
        {
            auto userId = auth.requireUser(req);

            Poco::JSON::Parser parser;
            auto json = parser.parse(req.stream()).extract<Poco::JSON::Object::Ptr>();

            CreateRecipeRequest dto;
            dto.title = json->getValue<std::string>("title");
            dto.description = json->getValue<std::string>("description");

            auto recipe = service.createRecipe(userId, dto);

            Poco::JSON::Object resp;
            resp.set("id", recipe.id);
            resp.set("title", recipe.title);
            resp.set("description", recipe.description);
            resp.set("authorId", recipe.authorId);
            resp.set("ingredients", recipe.ingredients);

            res.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
            res.setContentType("application/json");
            resp.stringify(res.send());
            return;
        }

        if (req.getMethod() == "GET")
        {
            int remaining;
            bool allowed = rateLimiter.allow("global:recipes", remaining);

            res.set("X-RateLimit-Limit", "100");
            res.set("X-RateLimit-Remaining", std::to_string(remaining));
            res.set("X-RateLimit-Reset", "60");

            if (!allowed)
            {
                res.setStatus(Poco::Net::HTTPResponse::HTTP_TOO_MANY_REQUESTS);
                return;
            }

            auto recipes = service.listRecipes();
            Poco::JSON::Array arr;

            for (auto &r : recipes)
            {
                Poco::JSON::Object resp;
                resp.set("id", r.id);
                resp.set("title", r.title);
                resp.set("description", r.description);
                resp.set("authorId", r.authorId);

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

                resp.set("ingredients", ingredientsArr);
                arr.add(resp);
            }

            res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            res.setContentType("application/json");
            arr.stringify(res.send());
            return;
        }

        res.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
    }
};
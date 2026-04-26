#pragma once
#include "../BaseHandler.hpp"

#include "../../service/RecipeService.hpp"

#include "../../auth/JwtMiddleware.hpp"

#include "../../cache/CacheService.hpp"

class IngredientHandler : public BaseHandler
{
private:
    RecipeService &service;
    JwtMiddleware &auth;
    CacheService &cache;

public:
    IngredientHandler(RecipeService &s, JwtMiddleware &a, CacheService &c)
        : service(s), auth(a), cache(c) {}

protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        auto uri = req.getURI();

        const std::string prefix = "/api/v1/recipes/";
        const std::string suffix = "/ingredients";

        std::string recipe_id = uri.substr(
            prefix.size(),
            uri.size() - prefix.size() - suffix.size());

        if (req.getMethod() == "POST")
        {

            auto userId = auth.requireUser(req);

            Poco::JSON::Parser parser;
            auto json = parser.parse(req.stream()).extract<Poco::JSON::Object::Ptr>();

            AddIngredientRequest dto;
            dto.name = json->getValue<std::string>("name");
            dto.amount = json->getValue<std::string>("amount");
            dto.unit = json->getValue<std::string>("unit");

            auto ing = service.addIngredient(recipe_id, dto);

            Poco::JSON::Object resp;
            resp.set("id", ing.id);
            resp.set("name", ing.name);
            resp.set("amount", ing.amount);
            resp.set("unit", ing.unit);

            res.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
            res.setContentType("application/json");
            resp.stringify(res.send());
            return;
        }

        if (req.getMethod() == "GET")
        {
            auto list = service.getIngredients(recipe_id);
            Poco::JSON::Array arr;

            for (auto &i : list)
            {
                Poco::JSON::Object resp;
                resp.set("id", i.id);
                resp.set("name", i.name);
                resp.set("amount", i.amount);
                resp.set("unit", i.unit);
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
#pragma once
#include "../BaseHandler.hpp"

#include "../../service/UserService.hpp"
#include "../../service/RecipeService.hpp"

class UserHandler : public BaseHandler
{
private:
    UserService &service;
    RecipeService &recipeService;

public:
    UserHandler(UserService &s, RecipeService &r) : service(s), recipeService(r) {}

protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        auto uri = req.getURI();

        if (req.getMethod() == "GET")
        {
            if (uri == "/api/v1/users")
            {
                auto users = service.getAll();

                Poco::JSON::Array arr;
                for (auto &u : users)
                {
                    Poco::JSON::Object obj;
                    obj.set("id", u.id);
                    obj.set("login", u.login);
                    obj.set("firstName", u.firstName);
                    obj.set("lastName", u.lastName);
                    arr.add(obj);
                }

                res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                res.setContentType("application/json");
                arr.stringify(res.send());
                return;
            }

            if (uri.find("/api/v1/users/") == 0 &&
                uri.size() > std::string("/api/v1/users/").size() &&
                uri.rfind("/recipes") == uri.size() - std::string("/recipes").size())
            {
                auto prefix = std::string("/api/v1/users/");
                auto suffix = std::string("/recipes");

                auto userId = uri.substr(
                    prefix.size(),
                    uri.size() - prefix.size() - suffix.size());

                auto recipes = recipeService.getRecipesByUserId(userId);

                Poco::JSON::Array arr;
                for (auto &r : recipes)
                {
                    Poco::JSON::Object obj;
                    obj.set("id", r.id);
                    obj.set("title", r.title);
                    obj.set("description", r.description);
                    obj.set("authorId", r.authorId);

                    Poco::JSON::Array ingredientsArr;
                    for (auto &i : r.ingredients)
                    {
                        Poco::JSON::Object ing;
                        ing.set("id", i.id);
                        ing.set("name", i.name);
                        ing.set("amount", i.amount);
                        ing.set("unit", i.unit);
                        ingredientsArr.add(ing);
                    }

                    obj.set("ingredients", ingredientsArr);
                    arr.add(obj);
                }

                res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                res.setContentType("application/json");
                arr.stringify(res.send());
                return;
            }

            auto login = uri.substr(uri.find_last_of('/') + 1);
            auto user = service.getByLogin(login);

            Poco::JSON::Object resp;
            resp.set("id", user.id);
            resp.set("login", user.login);
            resp.set("firstName", user.firstName);
            resp.set("lastName", user.lastName);

            res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            res.setContentType("application/json");
            resp.stringify(res.send());
            return;
        }

        if (req.getMethod() == "POST")
        {
            Poco::JSON::Parser parser;
            auto json = parser.parse(req.stream()).extract<Poco::JSON::Object::Ptr>();

            CreateUserRequest dto;
            dto.login = json->getValue<std::string>("login");
            dto.password = json->getValue<std::string>("password");
            dto.firstName = json->getValue<std::string>("firstName");
            dto.lastName = json->getValue<std::string>("lastName");

            auto user = service.createUser(dto);

            Poco::JSON::Object resp;
            resp.set("id", user.id);
            resp.set("login", user.login);

            res.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
            res.setContentType("application/json");
            resp.stringify(res.send());
            return;
        }

        res.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
    }
};
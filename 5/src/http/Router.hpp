#pragma once
#include <Poco/Net/HTTPRequestHandlerFactory.h>

#include "../http/handlers/UserHandler.hpp"
#include "../http/handlers/RecipeHandler.hpp"
#include "../http/handlers/AuthHandler.hpp"
#include "../http/handlers/IngredientHandler.hpp"
#include "../http/handlers/NotFoundHandler.hpp"
#include "../http/handlers/SwaggerHandler.hpp"
#include "../http/handlers/SwaggerUIHandler.hpp"

class Router : public Poco::Net::HTTPRequestHandlerFactory
{
private:
    UserService &userService;
    RecipeService &recipeService;
    AuthService &authService;
    JwtMiddleware &jwt;

public:
    Router(UserService &u, RecipeService &r, AuthService &a, JwtMiddleware &j)
        : userService(u), recipeService(r), authService(a), jwt(j) {}

    Poco::Net::HTTPRequestHandler *createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) override
    {

        auto uri = request.getURI();

        if (uri == "/api/v1/auth/login")
            return new AuthHandler(authService);

        if (uri.find("/api/v1/users") == 0)
            return new UserHandler(userService, recipeService);

        if (uri.find("/api/v1/recipes/") == 0 &&
            uri.size() >= std::string("/ingredients").size() &&
            uri.rfind("/ingredients") == uri.size() - std::string("/ingredients").size())
            return new IngredientHandler(recipeService, jwt);

        if (uri.find("/api/v1/recipes") == 0)
            return new RecipeHandler(recipeService, jwt);

        if (uri == "/swagger")
            return new SwaggerHandler();

        if (uri == "/docs")
            return new SwaggerUIHandler();

        return new NotFoundHandler();
    }
};
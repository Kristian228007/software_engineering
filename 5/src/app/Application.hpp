#pragma once
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Environment.h>

#include "../repository/UserRepository.hpp"
#include "../repository/RecipeRepository.hpp"

#include "../service/UserService.hpp"
#include "../service/RecipeService.hpp"
#include "../service/AuthService.hpp"

#include "../cache/CacheService.hpp"
#include "../cache/RateLimiter.hpp"

#include "../auth/JwtMiddleware.hpp"

#include "../http/Router.hpp"

class ServerApplication : public Poco::Util::ServerApplication
{
protected:
    int main(const std::vector<std::string> &args) override
    {
        std::string dbHost = Poco::Environment::get("DB_HOST", "localhost");
        std::string dbPort = Poco::Environment::get("DB_PORT", "5432");
        std::string dbUser = Poco::Environment::get("DB_USER", "recipe_user");
        std::string dbPassword = Poco::Environment::get("DB_PASSWORD", "recipe_pass");
        std::string dbName = Poco::Environment::get("DB_NAME", "recipe_db");

        std::string connectionString = "host=" + dbHost +
                                       " port=" + dbPort +
                                       " user=" + dbUser +
                                       " password=" + dbPassword +
                                       " dbname=" + dbName;

        UserRepository userRepo(connectionString);
        RecipeRepository recipeRepo(connectionString);

        CacheService cache;
        RateLimiter rateLimiter;

        UserService userService(userRepo);
        RecipeService recipeService(recipeRepo, userRepo, cache);
        AuthService authService(userRepo);
        JwtMiddleware jwt(authService);

        Poco::Net::ServerSocket socket(8080);
        Poco::Net::HTTPServer server(
            new Router(userService, recipeService, authService, jwt, cache, rateLimiter),
            socket,
            new Poco::Net::HTTPServerParams);

        server.start();
        waitForTerminationRequest();
        server.stop();

        return Application::EXIT_OK;
    }
};
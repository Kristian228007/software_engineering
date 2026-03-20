#pragma once
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/HTTPServer.h>

#include "../repository/UserRepository.hpp"
#include "../repository/RecipeRepository.hpp"

#include "../service/UserService.hpp"
#include "../service/RecipeService.hpp"
#include "../service/AuthService.hpp"

#include "../auth/JwtMiddleware.hpp"

#include "../http/Router.hpp"

class ServerApplication : public Poco::Util::ServerApplication
{
protected:
    int main(const std::vector<std::string> &args) override
    {
        UserRepository userRepo;
        RecipeRepository recipeRepo;

        UserService userService(userRepo);
        RecipeService recipeService(recipeRepo, userRepo);
        AuthService authService(userRepo);
        JwtMiddleware jwt(authService);

        Poco::Net::ServerSocket socket(8080);
        Poco::Net::HTTPServer server(
            new Router(userService, recipeService, authService, jwt),
            socket,
            new Poco::Net::HTTPServerParams);

        server.start();
        waitForTerminationRequest();
        server.stop();

        return Application::EXIT_OK;
    }
};
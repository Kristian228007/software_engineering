#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "../service/AuthService.hpp"
#include "../http/exceptions/UnauthorizedException.hpp"

class JwtMiddleware
{
private:
    AuthService &auth;

public:
    JwtMiddleware(AuthService &a) : auth(a) {}

    std::string requireUser(Poco::Net::HTTPServerRequest &request)
    {
        auto authHeader = request.get("Authorization", "");
        if (authHeader.find("Bearer ") != 0)
            throw UnauthorizedException();

        auto token = authHeader.substr(7);

        try
        {
            return auth.verify(token);
        }
        catch (...)
        {
            throw UnauthorizedException();
        }
    }
};
#pragma once
#include "../BaseHandler.hpp"

#include "../../service/AuthService.hpp"

class AuthHandler : public BaseHandler
{
private:
    AuthService &auth;

public:
    AuthHandler(AuthService &a) : auth(a) {}

protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        if (req.getMethod() != "POST")
        {
            res.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
            return;
        }

        Poco::JSON::Parser parser;
        auto json = parser.parse(req.stream()).extract<Poco::JSON::Object::Ptr>();

        auto login = json->getValue<std::string>("login");
        auto password = json->getValue<std::string>("password");

        auto token = auth.login(login, password);

        Poco::JSON::Object response;
        response.set("token", token);

        res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        res.setContentType("application/json");
        response.stringify(res.send());
    }
};
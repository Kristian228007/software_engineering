#pragma once
#include "../BaseHandler.hpp"

#include <fstream>
#include <sstream>

#include "../exceptions/NotFoundException.hpp"

class SwaggerHandler : public BaseHandler
{
protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        std::ifstream file("/usr/local/bin/swagger.yaml");
        if (!file)
            throw NotFoundException("Swagger not found");

        std::stringstream buffer;
        buffer << file.rdbuf();

        res.setContentType("text/yaml");
        res.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        res.send() << buffer.str();
    }
};
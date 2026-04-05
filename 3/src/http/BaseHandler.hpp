#pragma once
#include <exception>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>

#include "./exceptions/UnauthorizedException.hpp"
#include "./exceptions/NotFoundException.hpp"
#include "./exceptions/ConflictException.hpp"

class BaseHandler : public Poco::Net::HTTPRequestHandler
{
protected:
    virtual void handle(Poco::Net::HTTPServerRequest &req,
                        Poco::Net::HTTPServerResponse &res) = 0;

    void sendError(Poco::Net::HTTPServerResponse &res,
                   Poco::Net::HTTPResponse::HTTPStatus status,
                   const std::string &message)
    {
        Poco::JSON::Object err;
        err.set("error", message);

        res.setStatus(status);
        res.setContentType("application/json");
        err.stringify(res.send());
    }

public:
    void handleRequest(Poco::Net::HTTPServerRequest &req,
                       Poco::Net::HTTPServerResponse &res) override
    {
        try
        {
            handle(req, res);
        }
        catch (const UnauthorizedException &e)
        {
            sendError(res,
                      Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED,
                      e.what());
        }
        catch (const NotFoundException &e)
        {
            sendError(res,
                      Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                      e.what());
        }
        catch (const ConflictException &e)
        {
            sendError(res,
                      Poco::Net::HTTPResponse::HTTP_CONFLICT,
                      e.what());
        }
        catch (const std::exception &e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl;

            sendError(res,
                      Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                      e.what());
        }
    }
};
#pragma once
#include "../BaseHandler.hpp"

class NotFoundHandler : public BaseHandler
{
protected:
    void handle(Poco::Net::HTTPServerRequest &req,
                Poco::Net::HTTPServerResponse &res) override
    {
        Poco::JSON::Object obj;
        obj.set("error", "route not found");

        res.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        res.setContentType("application/json");
        obj.stringify(res.send());
    }
};
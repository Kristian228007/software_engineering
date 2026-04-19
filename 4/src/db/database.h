#pragma once

#include <Poco/MongoDB/Connection.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/OpMsgMessage.h>

#include <functional>
#include <string>
#include <stdexcept>

namespace db
{
    inline void mongoEnsureOk(Poco::MongoDB::OpMsgMessage &resp, const std::string &op)
    {
        if (resp.documents().empty())
            return;

        auto doc = resp.documents()[0];

        if (doc->exists("ok") && doc->getInteger("ok") != 1)
        {
            std::string err = doc->exists("errmsg")
                                  ? doc->get<std::string>("errmsg")
                                  : "unknown error";

            throw std::runtime_error("MongoDB " + op + " failed: " + err);
        }
    }

    class Database
    {
    private:
        Poco::MongoDB::Connection _mongoConn;
        Poco::MongoDB::Database _mongoDb;

        Database() : _mongoDb("test") {}

    public:
        static Database &instance()
        {
            static Database inst;
            return inst;
        }

        void initMongo(const std::string &host, int port, const std::string &dbName)
        {
            _mongoConn.connect(host, port);
            _mongoDb = Poco::MongoDB::Database(dbName);
        }

        Poco::MongoDB::Database &mongoDb()
        {
            return _mongoDb;
        }

        Poco::MongoDB::Connection &mongoConn()
        {
            return _mongoConn;
        }

        // удобный хелпер под OpMsg (чтобы не писать send везде руками)
        inline void send(Poco::MongoDB::OpMsgMessage &req, Poco::MongoDB::OpMsgMessage &resp)
        {
            _mongoConn.sendRequest(req, resp);
        }

        template <typename F>
        void withConnection(F &&f)
        {
            f(_mongoConn);
        }

        template <typename F>
        void withLockedConnection(F &&f)
        {
            f(_mongoConn);
        }
    };
}
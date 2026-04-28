#pragma once
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Array.h>
#include <optional>
#include <string>

class CacheService
{
private:
    Poco::Redis::Client client;

public:
    CacheService() : client("redis", 6379) {}

    std::optional<std::string> get(const std::string &key)
    {
        Poco::Redis::Array cmd;
        cmd.add("GET");
        cmd.add(key);

        Poco::Redis::BulkString result = client.execute<Poco::Redis::BulkString>(cmd);

        if (result.isNull())
            return std::nullopt;
        return result.value();
    }

    void set(const std::string &key, const std::string &value, int ttl)
    {
        Poco::Redis::Array setCmd;
        setCmd.add("SET");
        setCmd.add(key);
        setCmd.add(value);
        client.execute<void>(setCmd);

        Poco::Redis::Array expireCmd;
        expireCmd.add("EXPIRE");
        expireCmd.add(key);
        expireCmd.add(std::to_string(ttl));
        client.execute<void>(expireCmd);
    }

    void del(const std::string &key)
    {
        Poco::Redis::Array delCmd;
        delCmd.add("DEL");
        delCmd.add(key);
        client.execute<void>(delCmd);
    }
};
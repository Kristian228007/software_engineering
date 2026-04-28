#pragma once
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Array.h>
#include <string>

class RateLimiter
{
private:
    Poco::Redis::Client client;
    int limit;

public:
    RateLimiter(int l = 100) : client("redis", 6379), limit(l) {}

    bool allow(const std::string &key, int &remaining)
    {
        Poco::Redis::Array incrCmd;
        incrCmd.add("INCR");
        incrCmd.add(key);

        Poco::Int64 count = client.execute<Poco::Int64>(incrCmd);

        Poco::Redis::Array expireCmd;
        expireCmd.add("EXPIRE");
        expireCmd.add(key);
        expireCmd.add("60");
        client.execute<void>(expireCmd);

        remaining = limit - count;

        return count <= limit;
    }
};
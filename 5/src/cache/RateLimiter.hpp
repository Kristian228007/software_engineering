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
        // INCR command as Array
        Poco::Redis::Array incrCmd;
        incrCmd.add("INCR");
        incrCmd.add(key);

        // Execute and get result as BulkString, then convert to int
        Poco::Redis::BulkString result = client.execute<Poco::Redis::BulkString>(incrCmd);
        int count = std::stoi(result.value());

        // If first time, set expiration
        if (count == 1)
        {
            Poco::Redis::Array expireCmd;
            expireCmd.add("EXPIRE");
            expireCmd.add(key);
            expireCmd.add("60");
            client.execute<Poco::Redis::BulkString>(expireCmd);
        }

        remaining = limit - count;
        return count <= limit;
    }
};
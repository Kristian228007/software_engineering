#pragma once
#include <vector>
#include <mutex>
#include <optional>

#include <Poco/UUIDGenerator.h>

#include "../domain/entities/User.hpp"

class UserRepository
{
private:
    std::vector<User> users;
    std::mutex mutex_;

public:
    User create(User user)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        user.id = Poco::UUIDGenerator::defaultGenerator()
                      .createRandom()
                      .toString();

        users.push_back(user);
        return user;
    }

    std::optional<User> findByLogin(const std::string &login)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &u : users)
            if (u.login == login)
                return u;
        return std::nullopt;
    }

    std::vector<User> findAll()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return users;
    }
};
#pragma once
#include <vector>
#include <mutex>
#include <optional>

#include <Poco/UUIDGenerator.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/PostgreSQL/Connector.h>

#include "../domain/entities/User.hpp"

class UserRepository
{
private:
    Poco::Data::Session session_;
    std::mutex mutex_;

public:
    UserRepository(const std::string &connectionString)
        : session_("PostgreSQL", connectionString)
    {
    }

    User create(User user)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        user.id = Poco::UUIDGenerator::defaultGenerator()
                      .createRandom()
                      .toString();

        Poco::Data::Statement insert(session_);
        insert << "INSERT INTO users (id, login, password_hash, first_name, last_name) VALUES ($1, $2, $3, $4, $5)",
            Poco::Data::Keywords::use(user.id),
            Poco::Data::Keywords::use(user.login),
            Poco::Data::Keywords::use(user.password_hash),
            Poco::Data::Keywords::use(user.firstName),
            Poco::Data::Keywords::use(user.lastName);
        insert.execute();

        return user;
    }

    std::optional<User> findByLogin(const std::string &login)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string loginCopy = login;

        Poco::Data::Statement select(session_);
        User user;
        select << "SELECT id, login, password_hash, first_name, last_name FROM users WHERE login = $1",
            Poco::Data::Keywords::into(user.id),
            Poco::Data::Keywords::into(user.login),
            Poco::Data::Keywords::into(user.password_hash),
            Poco::Data::Keywords::into(user.firstName),
            Poco::Data::Keywords::into(user.lastName),
            Poco::Data::Keywords::use(loginCopy),
            Poco::Data::Keywords::range(0, 1);

        if (select.execute())
        {
            return user;
        }

        return std::nullopt;
    }

    std::optional<User> findById(const std::string &id)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string idCopy = id;

        Poco::Data::Statement select(session_);
        User user;
        select << "SELECT id, login, password_hash, first_name, last_name FROM users WHERE id = $1",
            Poco::Data::Keywords::into(user.id),
            Poco::Data::Keywords::into(user.login),
            Poco::Data::Keywords::into(user.password_hash),
            Poco::Data::Keywords::into(user.firstName),
            Poco::Data::Keywords::into(user.lastName),
            Poco::Data::Keywords::use(idCopy),
            Poco::Data::Keywords::range(0, 1);
        if (select.execute())
            return user;
        return std::nullopt;
    }

    std::vector<User> findAll()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::vector<User> users;

        Poco::Data::Statement select(session_);

        std::string id, login, firstName, lastName;

        select << "SELECT id, login, first_name, last_name FROM users",
            Poco::Data::Keywords::into(id),
            Poco::Data::Keywords::into(login),
            Poco::Data::Keywords::into(firstName),
            Poco::Data::Keywords::into(lastName),
            Poco::Data::Keywords::range(0, 1);

        while (!select.done())
        {
            if (select.execute())
            {
                User user;
                user.id = id;
                user.login = login;
                user.firstName = firstName;
                user.lastName = lastName;

                users.push_back(user);
            }
        }

        return users;
    }
};
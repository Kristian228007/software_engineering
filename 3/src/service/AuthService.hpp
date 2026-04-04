#pragma once
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/Token.h>

#include "../repository/UserRepository.hpp"

#include "../http/exceptions/UnauthorizedException.hpp"

class AuthService
{
private:
    UserRepository &repo;
    std::string secret = "SECRET_KEY";

public:
    AuthService(UserRepository &r) : repo(r) {}

    std::string login(const std::string &login, const std::string &password)
    {
        auto user = repo.findByLogin(login);
        if (!user || user->password_hash != password)
            throw UnauthorizedException("Invalid credentials");

        Poco::JWT::Token token;
        token.payload().set("sub", user->id);
        token.payload().set("login", user->login);

        Poco::JWT::Signer signer(secret);
        return signer.sign(token, Poco::JWT::Signer::ALGO_HS256);
    }

    std::string verify(const std::string &jwt)
    {
        Poco::JWT::Signer signer(secret);
        auto decoded = signer.verify(jwt);
        return decoded.payload().getValue<std::string>("sub");
    }
};
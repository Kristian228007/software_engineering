#pragma once
#include <string>
#include <vector>

struct User
{
    std::string id;
    std::string login;
    std::string password_hash;
    std::string firstName;
    std::string lastName;
};
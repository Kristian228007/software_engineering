#pragma once
#include <string>

struct CreateUserRequest
{
    std::string login;
    std::string password;
    std::string firstName;
    std::string lastName;
};
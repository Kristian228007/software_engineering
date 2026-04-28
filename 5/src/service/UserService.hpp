#pragma once
#include <stdexcept>

#include "../repository/UserRepository.hpp"

#include "../domain/dto/UserDTO.hpp"

#include "../http/exceptions/ConflictException.hpp"
#include "../http/exceptions/NotFoundException.hpp"

class UserService
{
private:
    UserRepository &repo;

public:
    UserService(UserRepository &r) : repo(r) {}

    User createUser(const CreateUserRequest &dto)
    {
        if (repo.findByLogin(dto.login))
            throw ConflictException("User already exists");

        User u;
        u.login = dto.login;
        u.password_hash = dto.password; // типо хэш
        u.firstName = dto.firstName;
        u.lastName = dto.lastName;

        return repo.create(u);
    }

    User getByLogin(const std::string &login)
    {
        auto u = repo.findByLogin(login);
        if (!u)
            throw NotFoundException("User not found");
        return *u;
    }

    std::vector<User> getAll()
    {
        return repo.findAll();
    }
};
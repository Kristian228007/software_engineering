#pragma once
#include <stdexcept>

class ConflictException : public std::runtime_error
{
public:
    ConflictException() : std::runtime_error("Conflict") {}
    ConflictException(const std::string &message) : std::runtime_error(message) {}
};
#pragma once
#include <stdexcept>

class UnauthorizedException : public std::runtime_error
{
public:
    UnauthorizedException() : std::runtime_error("Unauthorized") {}
    UnauthorizedException(const std::string &message) : std::runtime_error(message) {}
};
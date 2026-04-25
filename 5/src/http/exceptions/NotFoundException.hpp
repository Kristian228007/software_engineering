#pragma once
#include <stdexcept>

class NotFoundException : public std::runtime_error
{
public:
    NotFoundException() : std::runtime_error("Not found") {}
    NotFoundException(const std::string &message) : std::runtime_error(message) {}
};
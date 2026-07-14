#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>
#include <string>

// Project-specific exception types. All inherit from std::runtime_error, so every
// error carries a message (via .what()) and is caught by catch(const std::exception&).
// Each type prefixes its message with its name to make logs self-describing.

// Base class for every error this project throws.
class Error : public std::runtime_error
{
public:
    explicit Error(const std::string &message) : std::runtime_error(message) {}
};

// An argument has an invalid value (e.g. an empty set where a non-empty one is required).
class ValueError : public Error
{
public:
    explicit ValueError(const std::string &message) : Error("ValueError: " + message) {}
};

// Serialized / on-disk data is malformed and cannot be parsed as expected.
class ParseError : public Error
{
public:
    explicit ParseError(const std::string &message) : Error("ParseError: " + message) {}
};

// An internal invariant was violated ("this should never happen").
class ConsistencyError : public Error
{
public:
    explicit ConsistencyError(const std::string &message) : Error("ConsistencyError: " + message) {}
};

#endif // ERRORS_HPP

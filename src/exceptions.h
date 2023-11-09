#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

#include <stdexcept>

/// @brief Invalid user data
class DataException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// @brief Could not resolve asset
class ResolutionException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// @brief Problem occurred in library or OS. Used to convert C-style
/// error codes to exceptions.
class SystemException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class TimeoutException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class DeviceLostException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Unimplemented : public std::logic_error {
    using std::logic_error::logic_error;
};

class OutOfMemoryException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

#endif

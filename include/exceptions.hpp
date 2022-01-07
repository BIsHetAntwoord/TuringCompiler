#ifndef _TURINGCOMPILER_EXCEPTIONS_HPP
#define _TURINGCOMPILER_EXCEPTIONS_HPP

#include <stdexcept>

#include "utils.hpp"

class ProgramException : public std::runtime_error {
    public:
        template <typename... Args>
        ProgramException(const Args&... args) : std::runtime_error(utils_make_str(args...)) {}

        virtual ~ProgramException() = default;
};

class ParseException : public ProgramException {
    public:
        template <typename... Args>
        ParseException(const Args&... args) : ProgramException(args...) {}

        virtual ~ParseException() = default;
};

#endif

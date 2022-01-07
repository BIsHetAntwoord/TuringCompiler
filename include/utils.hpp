#ifndef _TURINGCOMPILER_UTILS_HPP
#define _TURINGCOMPILER_UTILS_HPP

#include <string>
#include <sstream>
#include <vector>

std::string utils_ltrim(const std::string&);
std::string utils_rtrim(const std::string&);
std::string utils_trim(const std::string&);

std::vector<std::string> utils_split(const std::string&, char);

template <typename... Args>
std::string utils_make_str(const Args&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

#endif

#include "utils.hpp"

std::string utils_ltrim(const std::string& str) {
    size_t i = str.size();

    while(i > 0 && (str[i-1] == ' ' || str[i-1] == '\t' || str[i-1] == '\r' || str[i-1] == '\n'))
        --i;

    return str.substr(0, i);
}

std::string utils_rtrim(const std::string& str) {
    size_t i = 0;
    while(i < str.size() && (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'))
        ++i;

    return str.substr(i);
}

std::string utils_trim(const std::string& str) {
    return utils_ltrim(utils_rtrim(str));
}

std::vector<std::string> utils_split(const std::string& str, char delim) {
    std::stringstream ss(str);
    std::string part;
    std::vector<std::string> result;
    while(std::getline(ss, part, delim)) {
        result.push_back(part);
    }
    return result;
}
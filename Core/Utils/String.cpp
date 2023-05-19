#include "String.hpp"

#include <algorithm>
#include <codecvt>
#include <locale>


namespace MapleLeaf {
std::vector<std::string> String::Split(const std::string& str, char sep)
{
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       tokenStream(str);

    while (std::getline(tokenStream, token, sep)) tokens.emplace_back(token);
    return tokens;
}

std::string String::ReplaceFirst(std::string str, std::string_view token, std::string_view to)
{
    const auto startPos = str.find(token);
    if (startPos == std::string::npos) return str;

    str.replace(startPos, token.length(), to);
    return str;
}

}   // namespace MapleLeaf
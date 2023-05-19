#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace MapleLeaf {
class String
{
public:
    /**
     * Splits a string by a separator.
     * @param str The string.
     * @param sep The separator.
     * @return The split string vector.
     */
    static std::vector<std::string> Split(const std::string& str, char sep);

    /**
     * Replaces the first token from a string.
     * @param str The string.
     * @param token The token.
     * @param to The string to replace the tokens with.
     * @return The string with the tokens replaced.
     */
    static std::string ReplaceFirst(std::string str, std::string_view token, std::string_view to);
};
}   // namespace MapleLeaf
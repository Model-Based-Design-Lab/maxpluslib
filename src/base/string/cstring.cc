/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   cstring.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 26, 2003
 *
 *  Function        :   String class
 *
 *  History         :
 *      26-09-03    :   Initial version.
 *
 *
 *  Copyright 2023 Eindhoven University of Technology
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the “Software”),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include "base/string/cstring.h"
#include <cctype>
#include <cstdio>
#include <sstream>
#include <string.h>

namespace MaxPlus {

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString() = default;

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const char s) : std::string(1, s) {}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const char *s) : std::string(s) {}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const std::string &s) : std::string(s) {}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const MPString &s) = default;

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const int n) {
    char str[32];
    snprintf(&str[0], 32, "%i", n);
    append(std::string(str));
}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const unsigned int n) {
    char str[32];
    snprintf(&str[0], 32, "%u", n);
    append(std::string(str));
}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const long int n) {
    char str[32];
    snprintf(&str[0], 32, "%ld", n);
    append(std::string(str));
}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const unsigned long int n) {
    char str[32];
    snprintf(&str[0], 32, "%ld", n);
    append(std::string(str));
}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const long long int n) {
    char str[32];
    snprintf(&str[0], 32, "%lld", n);
    append(std::string(str));
}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const unsigned long long int n) {
    char str[32];
    snprintf(&str[0], 32, "%lld", n);
    append(std::string(str));
}

/**
 * MPString ()
 * Constructor.
 */
MPString::MPString(const CDouble n) {
    char str[32];
    snprintf(&str[0], 32, "%g", n);
    append(std::string(str));
}

/**
 * ~MPString ()
 * Destructor.
 */
MPString::~MPString() = default;

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const MPString &s) {
    append(s);
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const char c) {
    push_back(c);
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const int n) {
    MPString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const unsigned int n) {
    MPString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const long int n) {
    MPString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const unsigned long int n) {
    MPString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const long long int n) {
    MPString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
MPString &MPString::operator+=(const CDouble n) {
    MPString str(n);
    append(str);

    return *this;
}

/**
 * operator const char* ()
 * Type conversion to constant character pointer.
 */
MPString::operator const char *() const { return c_str(); }

/**
 * operator int ()
 * Type conversion to integer.
 */
MPString::operator int() const { return strtol(c_str(), nullptr, 0); }

/**
 * operator uint ()
 * Type conversion to unsigned integer.
 */
MPString::operator uint() const { return strtoul(c_str(), nullptr, 0); }

/**
 * operator CDouble ()
 * Type conversion to CDouble.
 */
MPString::operator CDouble() const { return strtod(c_str(), nullptr); }

/**
 * operator long ()
 * Type conversion to long.
 */
MPString::operator long() const { return strtol(c_str(), nullptr, 0); }

/**
 * operator unsigned long ()
 * Type conversion to unsigned long.
 */
MPString::operator unsigned long() const { return strtoul(c_str(), nullptr, 0); }

/**
 * operator long long ()
 * Type conversion to long long.
 */
MPString::operator long long() const {
#ifdef _MSC_VER
    return _strtoi64(c_str(), nullptr, 0);
#else
    return strtoll(c_str(), nullptr, 0);
#endif
}

/**
 * operator unsigned long long ()
 * Type conversion to unsigned long long.
 */

MPString::operator unsigned long long() const {
#ifdef _MSC_VER
    return _strtoui64(c_str(), nullptr, 0);
#else
    return strtoull(c_str(), nullptr, 0);
#endif
}

/**
 * trim ()
 * Remove whitespace from left-hand and right-hand side of string.
 */
MPString &MPString::trim() {
    ltrim();
    rtrim();

    return *this;
}

/**
 * ltrim ()
 * Remove whitespace from left-hand side of string.
 */
MPString &MPString::ltrim() {

    MPString::size_type startPos = 0;

    // Find first non-whitespace character in the string (from left)
    while (startPos < length() && (isspace(at(startPos)) != 0)) {
        startPos++;
    }

    if (startPos == length()) {
        assign("");
    } else {
        assign(substr(startPos, length()));
    }
    return *this;
}

/**
 * rtrim ()
 * Remove whitespace from right-hand side of string.
 */
MPString &MPString::rtrim() {
    MPString::size_type endPos = length() - 1;

    // Find first non-whitespace character in the string (from right)
    while (endPos < length() && (isspace(at(endPos)) != 0)) {
        endPos--;
    }

    if (endPos > length()) {
        assign("");
    } else {
        assign(substr(0, endPos + 1));
    }

    return *this;
}

/**
 * split ()
 * Split the string on all occurrences of delim character
 */
MPStrings MPString::split(const char delim) const {
    MPStrings strings;
    MPString::size_type curDelim = 0;
    MPString::size_type nextDelim = 0;

    do {
        // Position the delimiters
        curDelim = nextDelim;
        nextDelim = find(delim, curDelim);

        // Add substring to list
        if (nextDelim >= curDelim && curDelim != length()) {
            strings.push_back(MPString(substr(curDelim, (nextDelim - curDelim))));
        }

        // Advance nextDelim position to always make progress
        if (nextDelim != MPString::npos) {
            nextDelim++;
        }
    } while (nextDelim != MPString::npos);

    return strings;
}

/**
 * join ()
 * Join the list of strings delimited with delim character
 */
MPString MPString::join(const MPStrings &strl, const char delim) {
    MPString res = "";
    MPStrings::const_iterator si;
    bool first = true;
    for (si = strl.begin(); si != strl.end(); si++) {
        if (!first) {
            res += delim;
        }
        res += (*si);
        first = false;
    }
    return res;
}

/**
 * join ()
 * Join the list of strings delimited with delim character
 */
MPString MPString::join(const MPStrings &strl, const MPString &delim) {
    MPString res = "";
    MPStrings::const_iterator si;
    bool first = true;
    for (si = strl.begin(); si != strl.end(); si++) {
        if (!first) {
            res += delim;
        }
        res += (*si);
        first = false;
    }
    return res;
}

/**
 * replace ()
 * Replace first n occurrences of string s1 with string s2 starting from
 * position sPos. If n is equal to zero, all occurrences are replaced
 */
MPString &
MPString::replace(const MPString &s1, const MPString &s2, const size_type sPos, const uint n) {
    size_type pos = sPos;
    uint nrReplaced = 0;

    for (pos = find(s1, pos); pos != MPString::npos; pos = find(s1, pos + s2.length())) {
        std::string::replace(pos, s1.size(), s2.c_str());
        nrReplaced++;

        if (n > 0 && nrReplaced == n) {
            break;
        }
    }

    return *this;
}

/**
 * isTok ()
 * The function returns true if the character is a valid token, else
 * the function returns false.
 */
bool isTok(const char c, const char *tok) { return (strchr(tok, c) != nullptr); }

/**
 * stringTok ()
 * Split a string into tokens using the given token delimiter.
 */
void stringTok(MPStrings &l, MPString &str, const char *tok) {
    const int S = static_cast<int>(str.size());
    int i = 0;

    // Clear list of strings
    l.clear();

    // Split string
    while (i < S) {
        // eat leading whitespace
        while ((i < S) && (isTok(str[i], tok))) {
            ++i;
        }
        if (i == S) {
            return; // nothing left but WS
        }

        // find end of word
        int j = i + 1;
        while ((j < S) && (!isTok(str[j], tok))) {
            ++j;
        }

        // add word
        l.push_back(MPString(str.substr(i, j - i)));

        // set up for next loop
        i = j + 1;
    }
}

/**
 * toLower ()
 * The function converts the string to lower-case.
 */
MPString &MPString::toLower() {
    MPString s = *this;
    std::transform(s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(std::tolower));
    assign(s);

    return *this;
}

/**
 * toUpper ()
 * The function converts the string to upper-case.
 */
MPString &MPString::toUpper() {
    MPString s = *this;
    std::transform(s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(std::toupper));
    assign(s);

    return *this;
}

/**
 * isNNInteger ()
 * Check if the string represents a non-negative integer number (leading + is not allowed)
 */

bool MPString::isNNInteger() {
    // naive implementation, check if the string includes a decimal separator
    // todo: replace with something smarter

    MPString::size_type pos = 0;

    // Find first non-whitespace and non-digit character in the string
    while (pos < length() && ((isspace(at(pos)) != 0) || (isdigit(at(pos)) != 0))) {
        pos++;
    }

    // If we didn't find any, the string is a non-negative integer
    return pos == length();
}

/**
 * regexReplace()
 */
MPString MPString::regexReplace(const MPString &regex, const MPString &replace) {
    try {
        std::regex pattern(regex);
        MPString res = MPString(regex_replace(*this, pattern, std::string(replace)));
        return res;
    } catch (std::runtime_error &e) {
        /// @todo add MPExceptions to string class?
        std::cout << "Failed regex: " << e.what() << std::endl;
        //      throw MPException(MPString("Failed to compile and execute regex
        //      expression.")+e.what());
        return *this;
    }
}

/**
 * regexReplaceMultiline()
 */
MPString MPString::regexReplaceMultiLine(const MPString &regex, const MPString &replace) {
    try {
        std::regex pattern(regex);

        std::stringstream ss(*this);
        std::ostringstream result;
        std::string line;

        while (std::getline(ss, line, '\n')) {
            std::string res = regex_replace(line, pattern, std::string(replace));
            result << res << '\n';
        }

        return MPString(result.str());
    } catch (std::runtime_error &e) {
        /// @todo add MPExceptions to string class?
        std::cout << "Failed regex: " << e.what() << std::endl;
        //      throw MPException(MPString("Failed to compile and execute regex
        //      expression.")+e.what());
        return *this;
    }
}

} // namespace MaxPlus

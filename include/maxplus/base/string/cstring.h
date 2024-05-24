/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   cstring.h
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

#ifndef MAXPLUS_BASE_STRING_CSTRING_H
#define MAXPLUS_BASE_STRING_CSTRING_H

#include "../basic_types.h"
#include <regex>
#include <string>

namespace MaxPlus {

// Forward class definition
class MPString;

// Types
using MPStrings = std::list<MPString>;
using MPStringsIter = MPStrings::iterator;

/**
 * MPString
 * String container class.
 * Derived from STL library class string.
 */
class MPString : public std::string {
public:
    // Constructor
    MPString();
    MPString(const char *s);
    explicit MPString(char s);
    explicit MPString(const std::string &s);
    MPString(const MPString &s);

    // Constructor (integer number)
    explicit MPString(int n);
    explicit MPString(unsigned int n);
    explicit MPString(long int n);
    explicit MPString(unsigned long int n);
    explicit MPString(long long int n);
    explicit MPString(unsigned long long int n);

    // Constructor (floating number)
    explicit MPString(CDouble n);

    // Destructor
    ~MPString();

    // Assignment
    MPString &operator+=(const MPString &s);
    MPString &operator+=(char c);
    MPString &operator+=(int n);
    MPString &operator+=(unsigned int n);
    MPString &operator+=(long int n);
    MPString &operator+=(unsigned long int n);
    MPString &operator+=(long long int n);
    MPString &operator+=(CDouble n);

    // Character access
    char operator[](int n) { return (c_str())[n]; };

    // Type conversion
    explicit operator const char *() const;
    explicit operator int() const;
    explicit operator uint() const;
    explicit operator CDouble() const;
    explicit operator long() const;
    explicit operator unsigned long() const;
    explicit operator long long() const;
    explicit operator unsigned long long() const;

    // Whitespace
    MPString &trim();
    MPString &ltrim(); // left-hand side
    MPString &rtrim(); // right-hand side

    // Regex
    MPString regexReplace(const MPString &regex, const MPString &replace);
    MPString regexReplaceMultiLine(const MPString &regex, const MPString &replace);

    // Split
    [[nodiscard]] MPStrings split(char delim) const;
    static MPString join(const MPStrings &strl, char delim);
    static MPString join(const MPStrings &strl, const MPString &delim);

    // Replacement
    MPString &replace(const MPString &s1, const MPString &s2, size_type sPos = 0, uint n = 0);

    // Case
    MPString &toLower();
    MPString &toUpper();

    // TEsting
    bool isNNInteger();
};

/**
 * operator+
 * Append operator for MPString class.
 */
inline MPString operator+(const MPString &lhs, const MPString &rhs) {
    MPString str(lhs);
    str.append(rhs);
    return str;
}

inline MPString operator+(const MPString &lhs, const std::string &rhs) {
    MPString str(lhs);
    str.append(rhs);
    return str;
}

inline MPString operator+(const MPString &lhs, const char *rhs) {
    MPString str(lhs);
    str.append(rhs);
    return str;
}

inline MPString operator+(const std::string &lhs, const MPString &rhs) {
    MPString str(lhs);
    str.append(rhs);
    return str;
}

inline MPString operator+(const char *lhs, const MPString &rhs) {
    MPString str(lhs);
    str.append(rhs);
    return str;
}

// Tokenize a string
void stringTok(MPStrings &l, const MPString &str, const char *tok = " \t\n");

} // namespace MaxPlus

#endif

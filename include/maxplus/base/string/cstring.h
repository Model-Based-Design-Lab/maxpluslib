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
class CString;

// Types
using CStrings = std::list<CString>;
using CStringsIter = CStrings::iterator;

/**
 * CString
 * String container class.
 * Derived from STL library class string.
 */
class CString : public std::string {
public:
    // Constructor
    CString();
    CString(const char *s);
    explicit CString(char s);
    explicit CString(const std::string &s);
    CString(const CString &s);

    // Constructor (integer number)
    explicit CString(int n);
    explicit CString(unsigned int n);
    explicit CString(long int n);
    explicit CString(unsigned long int n);
    explicit CString(long long int n);
    explicit CString(unsigned long long int n);

    // Constructor (floating number)
    explicit CString(CDouble n);

    // Destructor
    ~CString();

    // Assignment
    CString &operator+=(const CString &s);
    CString &operator+=(char c);
    CString &operator+=(int n);
    CString &operator+=(unsigned int n);
    CString &operator+=(long int n);
    CString &operator+=(unsigned long int n);
    CString &operator+=(long long int n);
    CString &operator+=(CDouble n);

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
    CString &trim();
    CString &ltrim(); // left-hand side
    CString &rtrim(); // right-hand side

    // Regex
    CString regexReplace(const CString &regex, const CString &replace);
    CString regexReplaceMultiLine(const CString &regex, const CString &replace);

    // Split
    [[nodiscard]] CStrings split(char delim) const;
    static CString join(const CStrings &strl, char delim);
    static CString join(const CStrings &strl, const CString &delim);

    // Replacement
    CString &replace(const CString &s1, const CString &s2, size_type sPos = 0, uint n = 0);

    // Case
    CString &toLower();
    CString &toUpper();

    // TEsting
    bool isNNInteger();
};

/**
 * operator+
 * Append operator for CString class.
 */
inline CString operator+(const CString &lhs, const CString &rhs) {
    CString str(lhs);
    str.append(rhs);
    return str;
}

inline CString operator+(const CString &lhs, const std::string &rhs) {
    CString str(lhs);
    str.append(rhs);
    return str;
}

inline CString operator+(const CString &lhs, const char *rhs) {
    CString str(lhs);
    str.append(rhs);
    return str;
}

inline CString operator+(const std::string &lhs, const CString &rhs) {
    CString str(lhs);
    str.append(rhs);
    return str;
}

inline CString operator+(const char *lhs, const CString &rhs) {
    CString str(lhs);
    str.append(rhs);
    return str;
}

// Tokenize a string
void stringTok(CStrings &l, const CString &str, const char *tok = " \t\n");

} // namespace MaxPlus

#endif

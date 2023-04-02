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

#ifndef BASE_STRING_CSTRING_H
#define BASE_STRING_CSTRING_H

#ifdef _MSC_VER
#include <regex> // requires feature pack in VS2008
#include <string>

#else
#include <cstring>
#endif

#include "../basic_types.h"

// Forward class definition
class CString;

// Types
typedef std::list<CString> CStrings;
typedef CStrings::iterator CStringsIter;

/**
 * CString
 * String container class.
 * Derived from STL library class string.
 */
class CString : public std::string {
public:
    // Constructor
    CString();
    CString(const char s);
    CString(const char *s);
    CString(const std::string &s);
    CString(const CString &s);

    // Constructor (integer number)
    CString(const int n);
    CString(const unsigned int n);
    CString(const long int n);
    CString(const unsigned long int n);
    CString(const long long int n);
    CString(const unsigned long long int n);

    // Constructor (floating number)
    CString(const CDouble n);

    // Destructor
    ~CString();

    // Assignment
    CString &operator+=(const CString &s);
    CString &operator+=(const char c);
    CString &operator+=(const int n);
    CString &operator+=(const unsigned int n);
    CString &operator+=(const long int n);
    CString &operator+=(const unsigned long int n);
    CString &operator+=(const long long int n);
    CString &operator+=(const CDouble n);

    // Character access
    char operator[](int n) { return (c_str())[n]; };

    // Type conversion
    operator const char *() const;
    operator int() const;
    operator uint() const;
    operator CDouble() const;
    operator long() const;
    operator unsigned long() const;
    operator long long() const;
    operator unsigned long long() const;

    // Whitespace
    CString &trim();
    CString &ltrim(); // left-hand side
    CString &rtrim(); // right-hand side

    // Regex
    CString regexReplace(const CString &regex, const CString &replace);
    CString regexReplaceMultiLine(const CString &regex, const CString &replace);

    // Split
    CStrings split(const char delim) const;
    static CString join(const CStrings strl, const char delim);
    static CString join(const CStrings strl, const CString delim);

    // Replacement
    CString &
    replace(const CString &s1, const CString &s2, const size_type sPos = 0, const uint n = 0);

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

#endif

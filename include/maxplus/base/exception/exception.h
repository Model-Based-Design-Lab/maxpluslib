/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   exception.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 26, 2003
 *
 *  Function        :   Exception class
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

#ifndef MAXPLUS_BASE_EXCEPTION_EXCEPTION_H
#define MAXPLUS_BASE_EXCEPTION_EXCEPTION_H

#include "../string/cstring.h"
#include <exception>

/*
 * CException
 * CException container class.
 */
class CException : std::exception {
private:
    CString message;
    CString cause;

public:
    // Constructor
    explicit CException(const CString &message) : message(message) {}

    CException(const CString &message, CException &e) : message(message) {
        cause = "    caused by: " + e.getMessage();
        if (!e.getCause().empty()) {
            cause += "\n" + e.getCause();
        }
    }

    CException(const CException &e) : message(e.getMessage()), cause(e.getCause()) {}

    // Destructor
    ~CException() override = default;

    CException &operator=(const CException &other) = delete;
    CException(CException &&) = delete;
    CException &operator=(CException &&) = delete;

    // Message
    [[nodiscard]] CString getMessage() const { return message; }

    // Cause
    [[nodiscard]] CString getCause() const { return cause; }

    // Report
    std::ostream &report(std::ostream &stream) const {
        if (!getMessage().empty()) {
            stream << getMessage() << std::endl;
        }
        if (!getCause().empty()) {
            stream << getCause();
        }

        return stream;
    }

    friend std::ostream &operator<<(std::ostream &stream, const CException &e);
};

#define ASSERT(condition, msg)                                                                     \
    {                                                                                              \
        if (!(condition))                                                                          \
            throw CException(CString(__FILE__) + CString(":") + CString(__LINE__) + CString(": ")  \
                             + CString(msg));                                                      \
    }

#define EXCEPTION(msg, ...)                                                                        \
    {                                                                                              \
        char buf[1024];                                                                            \
        sprintf(&buf[0], msg, __VA_ARGS__);                                                        \
        throw CException(buf);                                                                     \
    }

#endif

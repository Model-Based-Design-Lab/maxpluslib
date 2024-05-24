/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *  Dept. of Electrical Engineering
 *  Electronics Systems Group
 *  Model Based Design Lab (https://computationalmodeling.info/)
 *
 *  Name            :   fraction.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   Fraction class
 *
 *  History         :
 *      13-07-05    :   Initial version.
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

#ifndef BASE_FRACTION_FRACTION_H
#define BASE_FRACTION_FRACTION_H

#include "../basic_types.h"
#include "../math/cmath.h"
#include "../string/cstring.h"
#include <climits>
#include <cstdint>

namespace MaxPlus {

class CFraction {
public:
    // Constructor
    explicit CFraction(const int num = 0, const int den = 1) : num(num), den(den) {
        if (den != 0) {
            this->val = static_cast<CDouble>(num) / static_cast<CDouble>(den);
        } else {
            this->val = 0.0;
        }
    };

    CFraction(const CFraction &f) = default;

    explicit CFraction(const CDouble v) : num(0), den(0), val(0.0) {
        CDoubleToFraction(v, INFINITY);
    };

    explicit CFraction(CString &f) {
        if (f.find('/') == CString::npos) {
            num = std::int64_t(f);
            den = 1;
        } else {
            num = std::int64_t(CString(f.substr(0, f.find('/'))));
            den = std::int64_t(CString(f.substr(f.find('/') + 1)));
        }
        val = static_cast<CDouble>(num) / static_cast<CDouble>(den);
    };

    // Destructor
    ~CFraction() = default;

    CFraction &operator=(const CFraction &other) = delete;
    CFraction(CFraction &&) = default;
    CFraction &operator=(CFraction &&) = delete;

    // Fraction contains a real fraction?
    [[nodiscard]] bool isFraction() const { return fraction; };

    // Fractional parts
    [[nodiscard]] std::int64_t numerator() const { return num; };
    [[nodiscard]] std::int64_t denominator() const { return den; };

    // Real value
    [[nodiscard]] CDouble value() const { return val; };

    // Convert CDouble to fraction
    void CDoubleToFraction(const CDouble v, const CDouble precision = 1e-15) {
        if (precision == INFINITY) {
            fraction = false;
            num = 0;
            den = 1;
            val = v;
        } else {
            std::int64_t i = 0;
            std::int64_t a = 0;
            std::int64_t b = 0;

            // Floating point number
            val = v;

            // Separate integer part from the fractional part
            auto whole = static_cast<std::int64_t>(val + precision);
            val -= static_cast<CDouble>(whole);
            val = fabs(val);
            CDouble frac = val;
            CDouble diff = frac;
            num = 1;
            den = 0;

            // Compute fraction as sum of reciprocals
            while (diff >= precision) {
                val = 1.0 / val;
                i = static_cast<std::int64_t>(val + precision);
                val -= static_cast<CDouble>(i);
                if (a != 0) {
                    num = i * num + b;
                }
                den = lround(static_cast<CDouble>(num) / frac);
                diff = fabs(static_cast<CDouble>(num) / static_cast<CDouble>(den) - frac);
                b = a;
                a = num;
            }

            if (num == den) {
                whole++;
                num = 0;
                den = 1;
            } else if (den == 0) {
                num = 0;
                den = 1;
            }

            // Add integer part to numerator
            num = whole * den + num;

            // Compute CDouble value based on fraction
            val = static_cast<CDouble>(num) / static_cast<CDouble>(den);
            fraction = true;
        }
    };

    // Lowest term
    [[nodiscard]] CFraction lowestTerm() const {
        CFraction tmp;
        std::int64_t g = gcd(num, den);
        tmp.num = num / g;
        tmp.den = den / g;
        tmp.val = (static_cast<CDouble>(tmp.num)) / static_cast<CDouble>(tmp.den);
        return tmp;
    }

    // Operators
    CFraction operator+(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() + rhs.value());
        }

        tmp.num = rhs.den * num + den * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator-(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() - rhs.value());
        }

        tmp.num = rhs.den * num - den * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator*(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() * rhs.value());
        }

        tmp.num = num * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator/(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() / rhs.value());
        }

        tmp.num = num * rhs.den;
        tmp.den = den * rhs.num;
        return tmp.lowestTerm();
    }

    bool operator==(const CFraction &rhs) const {
        if (!isFraction() || !rhs.isFraction()) {
            return value() == rhs.value();
        }

        if (den == 0 && rhs.den == 0 && num == rhs.num) {
            return true;
        }

        if (den == 0 || rhs.den == 0) {
            return false;
        }

        CFraction r = rhs.lowestTerm();
        CFraction l = lowestTerm();

        return r.den == l.den && r.num == l.num;
    }

    bool operator!=(const CFraction &rhs) const { return !(*this == rhs); }

    bool operator>(const CFraction &rhs) const {
        std::int64_t l = lcm(den, rhs.den);

        if (!isFraction() || !rhs.isFraction()) {
            return value() > rhs.value();
        }

        if (den == 0 || rhs.den == 0) {
            return false;
        }
        if (num == std::numeric_limits<std::int64_t>::max()) {
            return true;
        }
        if ((num * (l / den)) > (rhs.num * (l / rhs.den))) {
            return true;
        }

        return false;
    }

    bool operator<(const CFraction &rhs) const { return !(*this == rhs || *this > rhs); }

    std::ostream &print(std::ostream &out) const {
        if (isFraction()) {
            if (denominator() == 0) {
                out << "NaN";
            } else if (denominator() == 1) {
                out << numerator();
            } else {
                out << numerator() << "/" << denominator();
            }
        } else {
            out << value();
        }

        return out;
    }

    friend std::ostream &operator<<(std::ostream &out, CFraction &f) { return f.print(out); };

private:
    bool fraction{true};
    CDouble val;
    std::int64_t num;
    std::int64_t den;
};

using CFractions = std::vector<CFraction>;
using CFractionsIter = CFractions::iterator;
using CFractionsCIter = CFractions::const_iterator;

} // namespace MaxPlus

#endif

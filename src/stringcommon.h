/*
Copyright (c) 2013 Mathias Kaerlev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef VOXIE_STRING_H
#define VOXIE_STRING_H

#include <ctype.h>
#include <string>
#include <sstream>
#include <iomanip>

inline double string_to_double(const std::string & in, double def = 0.0)
{
    std::istringstream input(in);
    double value;
    if (!(input >> value))
        return def;
    return value;
}

inline float string_to_float(const std::string & in, float def = 0.0f)
{
    std::istringstream input(in);
    float value;
    if (!(input >> value))
        return def;
    return value;
}

inline int string_to_int(const std::string & in, int def = 0)
{
    std::istringstream input(in);
    int value;
    if (!(input >> value))
        return def;
    return value;
}

template <class T>
inline T string_to_number(const std::string & in, T def = T())
{
    std::istringstream input(in);
    T value;
    if (!(input >> value))
        return def;
    return value;
}

template <class T>
inline std::string number_to_string(T value)
{
    std::ostringstream input;
    input << value;
    return input.str();
}

inline std::string string_to_hex(char * input, size_t len)
{
    static const char* const lut = "0123456789ABCDEF";

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

inline std::string string_to_hex(const std::string& input)
{
    return string_to_hex((char*)&input[0], input.size());
}

template <class T>
inline std::string number_to_string_pad(T value, int pad)
{
    std::ostringstream input;
    input << std::setw(pad) << std::setfill('0') << value;
    return input.str();
}

inline std::string to_lower(std::string str)
{
    for (std::string::iterator i = str.begin(); i != str.end(); ++i)
        *i = static_cast<char>(tolower(*i));
    return str;
}

inline std::string to_upper(std::string str)
{
    for (std::string::iterator i = str.begin(); i != str.end(); ++i)
        *i = static_cast<char>(toupper(*i));
    return str;
}

template <class T>
inline std::string seconds_to_time(T v)
{
    int v2 = int(v);
    int minutes = v2 / 60;
    int seconds = v2 - minutes * 60;
    return number_to_string(minutes) + ":" + number_to_string_pad(seconds, 2);
}

inline bool starts_with(const std::string & a, const std::string & b)
{
    return a.compare(0, b.length(), b) == 0;
}

#endif // VOXIE_STRING_H

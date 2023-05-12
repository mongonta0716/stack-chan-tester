/*
see also https://stackoverflow.com/questions/7315936/which-of-sprintf-snprintf-is-more-secure/35401865#35401865
If you leave memory management to std::string, you don't have to free it yourself.
*/

// Using template
#include <string>
template<typename ...Args> std::string formatString(const char* fmt, Args... args)
{
    size_t sz = snprintf(nullptr, 0U, fmt, args...); // calculate length
    char buf[sz + 1];
    snprintf(buf, sizeof(buf), fmt, args...);
    return std::string(buf, buf + sz + 1);
}

// Using vsnprintf
#include <stdio.h>
#include <stdarg.h>
std::string formatString(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    size_t sz = vsnprintf(nullptr, 0U, fmt, args); // calculate length
    char buf[sz + 1];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return std::string(buf, buf + sz + 1);
}


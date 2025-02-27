
#include "utility/tostring.hpp"

#include <stdio.h>

#include <cstring>

//////////////////////////////////////////////////////////////////////

#ifndef _WIN32
static void itoa(int val, char* buff, int base)
{
    sprintf(buff, "%d", val);
}
#endif // WINDOWS

std::string utlToString(int i)
{
    return utlToString(std::make_pair(i, UtlRadix(10)));
}

std::string utlToString(std::pair<int, UtlRadix> p)
{
    char buffer[8 * sizeof(int) + 1];
    itoa(p.first, buffer, p.second);
    return std::string(buffer);
}

///////////////////////////////////

std::string utlToString(unsigned i)
{
    return utlToString(std::make_pair(i, UtlRadix(10)));
}

std::string utlToString(std::pair<unsigned, UtlRadix> p)
{
    char buffer[8 * sizeof(unsigned) + 1];
    // utoa( p.first, buffer, p.second );
    itoa(p.first, buffer, p.second);
    return std::string(buffer);
}

///////////////////////////////////

std::string utlToString(long i)
{
    return utlToString(std::make_pair(i, UtlRadix(10)));
}

std::string utlToString(std::pair<long, UtlRadix> p)
{
    char buffer[33];
    // ltoa( p.first, buffer, p.second );
    itoa(p.first, buffer, p.second);
    return std::string(buffer);
}

///////////////////////////////////

std::string utlToString(unsigned long i)
{
    return utlToString(std::make_pair(i, UtlRadix(10)));
}

std::string utlToString(std::pair<unsigned long, UtlRadix> p)
{
    char buffer[33];
    // ultoa( p.first, buffer, p.second );
    itoa(p.first, buffer, p.second);
    return std::string(buffer);
}

///////////////////////////////////

std::string utlToString(float f)
{
    long double d = f;
    return utlToString(d);
}

std::string utlToString(double f)
{
    long double d = f;
    return utlToString(d);
}

std::string utlToString(long double f)
{
    static constexpr int BUFFER_SIZE = 64;
    char buffer[BUFFER_SIZE];
    int len = sprintf(buffer, "%Lf", f);
    ASSERT(len <= BUFFER_SIZE, "");
    return std::string(buffer);
}

//////////////////////////////////////////////////////////////////////

/* End TOSTRING.CPP *************************************************/

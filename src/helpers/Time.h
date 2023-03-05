#ifndef __XMOTO_TIME_H__
#define __XMOTO_TIME_H__

#include <string>

std::string iso8601Date();

// Safe for use in file paths
std::string currentDateTime();

#endif // __XMOTO_TIME_H__

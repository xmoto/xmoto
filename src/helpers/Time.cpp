#include "Time.h"
#include <ctime>

std::string iso8601Date() {
  std::time_t t = std::time(nullptr);
  char datetime[sizeof("1970-01-01T00:00:00Z")];
  std::strftime(
    datetime, sizeof(datetime), "%Y-%m-%dT%H:%M:%SZ", std::localtime(&t));
  return std::string(datetime);
}

std::string currentDateTime() {
  std::time_t t = std::time(nullptr);
  char datetime[sizeof("1970-01-01_00-00-00")];
  std::strftime(
    datetime, sizeof(datetime), "%Y-%m-%d_%H-%M-%S", std::localtime(&t));
  return std::string(datetime);
}

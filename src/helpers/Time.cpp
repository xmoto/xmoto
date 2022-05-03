#include "Time.h"
#include <ctime>

std::string iso8601Date() {
  std::time_t t = std::time(nullptr);
  char datetime[sizeof("1970-01-01T00:00:00Z")];
  std::strftime(datetime, sizeof(datetime), "%FT%TZ", std::localtime(&t));
  return std::string(datetime);
}

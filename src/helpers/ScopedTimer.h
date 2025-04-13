#ifndef __SCOPEDTIMER_H__
#define __SCOPEDTIMER_H__

#include <chrono>
#include <iostream>

template<typename T>
const char *timeSuffix() {
  return "?";
}
template<>
const char *timeSuffix<std::chrono::nanoseconds>() {
  return "ns";
}
template<>
const char *timeSuffix<std::chrono::microseconds>() {
  return "μs";
}
template<>
const char *timeSuffix<std::chrono::milliseconds>() {
  return "ms";
}
template<>
const char *timeSuffix<std::chrono::seconds>() {
  return "s";
}
template<>
const char *timeSuffix<std::chrono::minutes>() {
  return "m";
}
template<>
const char *timeSuffix<std::chrono::hours>() {
  return "h";
}

template<typename T = std::chrono::milliseconds>
class ScopedTimer {
public:
  ScopedTimer() { m_start = std::chrono::high_resolution_clock::now(); }
  ~ScopedTimer() {
    m_end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float, typename T::period> duration = m_end - m_start;
    std::cout << duration.count() << timeSuffix<T>() << std::endl;
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end;
};

#endif

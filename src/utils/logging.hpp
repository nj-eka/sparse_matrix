#pragma once

#ifndef ENABLE_LOGGING

#define LOG_PPF
#define SHOW(...)

#else

#include <iostream>

#define LOG_PPF                                                        \
  (std::cout << "\n\033[7m/*" << __PRETTY_FUNCTION__ << " */\033[0m\n" \
             << std::endl)

#define SHOW(...) \
  (std::cout << #__VA_ARGS__ << " // ", __VA_ARGS__, std::cout << std::endl)

#endif

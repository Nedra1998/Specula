#include "specula/util/check.hpp"

#include <functional>
#include <ranges>
#include <string>
#include <vector>

#include "specula/util/log.hpp"

std::vector<std::function<std::string(void)>> specula::CheckCallbackScope::callbacks;

specula::CheckCallbackScope::CheckCallbackScope(const std::function<std::string(void)> &callback) {
  callbacks.push_back(callback);
}

specula::CheckCallbackScope::~CheckCallbackScope() {
  ASSERT_GE(callbacks.size(), 0U);
  callbacks.pop_back();
}

void specula::CheckCallbackScope::fail() {
  LOG_STACKTRACE();

  std::string message;
  for (auto &callback : std::views::reverse(callbacks)) {
    message += callback() + "\n";
  }
  LOG_CRITICAL("{}\n\n", message);

#if defined(_DEBUG) && defined(_MSC_VER)
  __debugbreak();
#else
  abort();
#endif
}

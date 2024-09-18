#include "util/check.hpp"

#include <cstdlib>
#include <sstream>

#include <cpptrace/cpptrace.hpp>

#include "util/log.hpp"

std::vector<std::function<std::string(void)>> specula::CheckCallbackScope::callbacks;

specula::CheckCallbackScope::CheckCallbackScope(std::function<std::string(void)> callback) {
  callbacks.push_back(std::move(callback));
}

specula::CheckCallbackScope::~CheckCallbackScope() {
  ASSERT_GT(callbacks.size(), 0);
  callbacks.pop_back();
}

void specula::CheckCallbackScope::fail() {
  print_stack_trace();

  std::string message;
  for (auto iter = callbacks.rbegin(); iter != callbacks.rend(); ++iter)
    message += (*iter)() + "\n";
  LOG_ERROR("{}\n\n", message);

#if defined(_DEBUG) && defined(_MSC_VER)
  __debugbreak();
#else
  abort();
#endif
}

void specula::print_stack_trace(bool force) {
  const char *env = std::getenv("SPECULA_BACKTRACE");
  if (env == nullptr && !force)
    return;

  const auto &cpptrace = cpptrace::generate_trace();
  std::stringstream out;

  if (env != nullptr && std::string_view(env) == "0")
    return;
  else if (env == nullptr || std::string_view(env) == "1")
    cpptrace.print(out, logging::colored());
  else if (std::string_view(env) == "2" || std::string_view(env) == "full")
    cpptrace.print_with_snippets(out, logging::colored());

  LOG_CRITICAL("{}\n", out.str());
}

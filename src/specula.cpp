#include "specula.hpp"

#include <vector>

#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "specula/version.hpp"
#include "util/log.hpp"

bool specula::initialize(std::vector<spdlog::sink_ptr> log_sinks, bool use_color) {
  ZoneScoped;
  if (!logging::initialize(log_sinks, use_color))
    return false;

  LOG_INFO("Specula v{}", SPECULA_VERSION);
  return true;
}

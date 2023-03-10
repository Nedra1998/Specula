#include "specula.hpp"

#include <vector>

#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

#include "specula/version.hpp"
#include "util/log.hpp"

bool specula::initialize(std::vector<spdlog::sink_ptr> log_sinks) {
  ZoneScoped;
  if (!logging::initialize(log_sinks))
    return false;

  LOG_INFO("Specula v{}", SPECULA_VERSION);
  return true;
}
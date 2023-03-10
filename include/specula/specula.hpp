#ifndef SPECULA_HPP
#define SPECULA_HPP

#include <vector>

#include <spdlog/spdlog.h>

namespace specula {
  bool initialize(std::vector<spdlog::sink_ptr> log_sinks = {});
}

#endif // SPECULA_HPP
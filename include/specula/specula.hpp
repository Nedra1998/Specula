/**
 * @file specula.hpp
 * @brief Main entrypoint for the renderer
 * @date 2023-03-13
 * @copyright Copyright (c) 2023
 *
 * This file provides the main entrypoint for the renderer library, and includes some global
 * initialization and teardown logic that should be called first and last thing respectivly.
 */
#ifndef SPECULA_HPP
#define SPECULA_HPP

#include <vector>

#include <spdlog/spdlog.h>

/**
 * @brief Primary namespace
 *
 * This is the primary namespace for the specula renderer. All components of the renderer will be
 * contained within this namespace, to avoid any naming conflicts with external packages.
 */
namespace specula {
  /**
   * @brief Initializes the specula renderer
   *
   * @param log_sinks An optional list of sinks to attach to the default logger
   * @return true if all initialization was successful
   * @return false if there were any errors during initialization
   * @see logging::initialize
   */
  bool initialize(std::vector<spdlog::sink_ptr> log_sinks = {});
} // namespace specula

#endif // SPECULA_HPP

#ifndef SPECULA_TEST_GLOBALS_HPP
#define SPECULA_TEST_GLOBALS_HPP

#include <memory>

#include <spdlog/sinks/ringbuffer_sink.h>

extern std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> log_ringbuffer;

#endif // SPECULA_TEST_GLOBALS_HPP
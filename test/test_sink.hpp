#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <magic_enum.hpp>
#include <spdlog/common.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <specula/util/log.hpp>

enum LogLevels {
  TRACE = spdlog::level::level_enum::trace,
  DEBUG = spdlog::level::level_enum::debug,
  INFO = spdlog::level::level_enum::info,
  WARN = spdlog::level::level_enum::warn,
  ERR = spdlog::level::level_enum::err,
  CRITICAL = spdlog::level::level_enum::critical,
  ALL = spdlog::level::level_enum::off
};

template <typename Mutex>
class vector_sink : public std::enable_shared_from_this<vector_sink<Mutex>>,
                    public spdlog::sinks::base_sink<Mutex> {
public:
  vector_sink(const std::size_t max_message_count)
      : max_message_count(max_message_count) {}

  std::vector<std::string> get(const std::string &logger,
                               const LogLevels &level) const {
    std::vector<std::string> messages;
    for (auto &it : log_messages) {
      if (!logger.empty() && logger != it.first.first)
        continue;
      else if (level != ALL && magic_enum::enum_integer(level) !=
                                   magic_enum::enum_integer(it.first.second))
        continue;
      messages.insert(messages.end(), it.second.begin(), it.second.end());
    }
    return messages;
  }
  std::vector<std::string> get() const { return get("", ALL); }
  std::vector<std::string> get(const std::string &logger) const {
    return get(logger, ALL);
  }
  std::vector<std::string> get(const LogLevels &level) const {
    return get("", level);
  }
  std::vector<std::string> get(const LogLevels &level,
                               const std::string &logger) const {
    return get(logger, level);
  }

  std::vector<std::string> find(const std::string &query,
                                const std::string &logger,
                                const LogLevels &level) const {
    std::vector<std::string> messages;
    for (auto &it : log_messages) {
      if (!logger.empty() && logger != it.first.first)
        continue;
      else if (level != ALL && magic_enum::enum_integer(level) !=
                                   magic_enum::enum_integer(it.first.second))
        continue;
      for (auto &msg : it.second)
        if (msg.find(query) != std::string::npos)
          messages.push_back(msg);
    }
    return messages;
  }

  std::vector<std::string> find(const std::string &query) const {
    return find(query, "", ALL);
  }
  std::vector<std::string> find(const std::string &query,
                                const std::string &logger) const {
    return find(query, logger, ALL);
  }
  std::vector<std::string> find(const std::string &query,
                                const LogLevels &level) const {
    return find(query, "", level);
  }
  std::vector<std::string> find(const std::string &query,
                                const LogLevels &level,
                                const std::string &logger) const {
    return find(query, logger, level);
  }

  std::map<std::pair<std::string, spdlog::level::level_enum>,
           std::vector<std::string>>
      log_messages;

protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    std::pair<std::string, spdlog::level::level_enum> index{
        std::string(msg.logger_name.data(), msg.logger_name.size()), msg.level};

    log_messages[index].push_back(
        std::string(msg.payload.data(), msg.payload.size()));
    if (log_messages[index].size() > max_message_count + 10)
      log_messages[index].erase(log_messages[index].begin(),
                                log_messages[index].begin() + 10);
  }
  void flush_() override {}

  std::size_t max_message_count;
};

using vector_sink_mt = vector_sink<std::mutex>;
using vector_sink_st = vector_sink<spdlog::details::null_mutex>;

inline std::shared_ptr<vector_sink_mt>
get_test_sink(const std::size_t max_message_count = 50) {
  auto sink = std::make_shared<vector_sink_mt>(max_message_count);
  sink->set_level(spdlog::level::trace);
  specula::logging::dist_sink->add_sink(sink);
  return sink;
}
inline void remove_test_sink(const std::shared_ptr<vector_sink_mt> &sink) {
  specula::logging::dist_sink->remove_sink(sink);
}

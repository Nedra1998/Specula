#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

template <typename Mutex>
class test_sink : public spdlog::sinks::base_sink<Mutex> {
public:
  static constexpr spdlog::level::level_enum trace =
      spdlog::level::level_enum::trace;
  static constexpr spdlog::level::level_enum debug =
      spdlog::level::level_enum::debug;
  static constexpr spdlog::level::level_enum info =
      spdlog::level::level_enum::info;
  static constexpr spdlog::level::level_enum warn =
      spdlog::level::level_enum::warn;
  static constexpr spdlog::level::level_enum err =
      spdlog::level::level_enum::err;
  static constexpr spdlog::level::level_enum critical =
      spdlog::level::level_enum::critical;

  const std::vector<std::string> &
  get(const std::string &logger, const spdlog::level::level_enum &level) const {
    return log_messages.at({logger, level});
  }
  std::vector<std::string> find(const std::string &logger,
                                const spdlog::level::level_enum &level,
                                const std::string &query) const {
    std::vector<std::string> matches;
    for (const auto &msg : log_messages.at({logger, level})) {
      if (msg.find(query) != std::string::npos)
        matches.push_back(msg);
    }
    return matches;
  }

  inline const std::vector<std::string> &
  get_trace(const std::string &logger) const {
    return this->get(logger, this->trace);
  }
  inline const std::vector<std::string> &
  get_debug(const std::string &logger) const {
    return this->get(logger, this->debug);
  }
  inline const std::vector<std::string> &
  get_info(const std::string &logger) const {
    return this->get(logger, this->info);
  }
  inline const std::vector<std::string> &
  get_warn(const std::string &logger) const {
    return this->get(logger, this->warn);
  }
  inline const std::vector<std::string> &
  get_err(const std::string &logger) const {
    return this->get(logger, this->err);
  }
  inline const std::vector<std::string> &
  get_critical(const std::string &logger) const {
    return this->get(logger, this->critical);
  }

  inline const std::vector<std::string> &
  find_trace(const std::string &logger, const std::string &query) const {
    return this->find(logger, this->trace, query);
  }
  inline const std::vector<std::string> &
  find_debug(const std::string &logger, const std::string &query) const {
    return this->find(logger, this->debug, query);
  }
  inline const std::vector<std::string> &
  find_info(const std::string &logger, const std::string &query) const {
    return this->find(logger, this->info, query);
  }
  inline const std::vector<std::string> &
  find_warn(const std::string &logger, const std::string &query) const {
    return this->find(logger, this->warn, query);
  }
  inline const std::vector<std::string> &
  find_err(const std::string &logger, const std::string &query) const {
    return this->find(logger, this->err, query);
  }
  inline const std::vector<std::string> &
  find_critical(const std::string &logger, const std::string &query) const {
    return this->find(logger, this->critical, query);
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
    if (log_messages[index].size() > 20)
      log_messages[index].erase(log_messages[index].begin(),
                                log_messages[index].begin() + 10);
  }
  void flush_() override {}
};

using test_sink_mt = test_sink<std::mutex>;
using test_sink_st = test_sink<spdlog::details::null_mutex>;

extern std::shared_ptr<test_sink_mt> logger;

#ifndef INCLUDE_APP_HELP_HPP_
#  define INCLUDE_APP_HELP_HPP_

#  include <string>
#  include <vector>

#  include <cxxopts.hpp>

namespace specula::app {
  class RichFormatter {
  public:
    RichFormatter(std::string description, std::vector<std::string> positionals)
        : description_(std::move(description)), positionals_(std::move(positionals)) {}
    ~RichFormatter() = default;

    inline std::string operator()(const cxxopts::Options &options) const { return format(options); }
    inline std::string operator()(const cxxopts::exceptions::specification &error) const {
      return format("Specification", error);
    }
    inline std::string operator()(const cxxopts::exceptions::parsing &error) const {
      return format("Parsing", error);
    }

    inline const std::string &description() const { return description_; }
    inline const std::vector<std::string> &positionals() const { return positionals_; }

  private:
    struct Widths {
      std::uint8_t short_option = 0;
      std::uint8_t long_option = 0;
      std::uint8_t meta = 0;
    };

    std::string format(const cxxopts::Options &options) const;
    std::string format(const std::string_view type,
                       const cxxopts::exceptions::exception &error) const;

    std::string format_usage(const cxxopts::Options &options,
                             const cxxopts::HelpGroupDetails &positionals) const;
    std::string format_group(const std::string_view &group_name,
                             const cxxopts::HelpGroupDetails &group,
                             bool is_positional = false) const;
    std::string format_option(const cxxopts::HelpOptionDetails &option, Widths &widths) const;
    std::string format_positional(const cxxopts::HelpOptionDetails &option, Widths &widths) const;

    std::size_t width_ = 120;
    std::string description_;
    std::vector<std::string> positionals_;
  };
} // namespace specula::app

#endif // INCLUDE_APP_HELP_HPP_
//

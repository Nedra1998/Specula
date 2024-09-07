#include "help.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include "cxxopts.hpp"

std::string wrap_text(const std::string &text, size_t width, size_t indent = 4) {
  std::string wrapped;
  size_t line_length = 0;
  size_t last_space = 0;
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == ' ')
      last_space = i;
    if (line_length >= width) {
      wrapped += '\n' + std::string(indent, ' ');
      line_length = i - last_space;
      i = last_space; // Move back to last space
    } else {
      wrapped += text[i];
      line_length++;
    }
  }
  return wrapped;
}

std::string specula::app::RichFormatter::format(const std::string_view type,
                                                const cxxopts::exceptions::exception &error) const {
  std::string line = fmt::format(
      "{} {} {}\n", fmt::styled("╭─", fmt::emphasis::faint | fg(fmt::terminal_color::red)),
      fmt::styled(fmt::format("{} Error", type),
                  fmt::emphasis::bold | fg(fmt::terminal_color::red)),
      fmt::styled(fmt::format("─{:─<{}}╮", "", width_ - type.size() - 10),
                  fmt::emphasis::faint | fg(fmt::terminal_color::red)));

  line += fmt::format("{} {:<{}}{}\n",
                      fmt::styled("│", fmt::emphasis::faint | fg(fmt::terminal_color::red)),
                      error.what(), width_ - 1,
                      fmt::styled("│", fmt::emphasis::faint | fg(fmt::terminal_color::red)));

  line +=
      fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::red), "╰{:─<{}}╯\n", "", width_);
  return line;
}

std::string specula::app::RichFormatter::format(const cxxopts::Options &options) const {
  std::string line = "";

  cxxopts::HelpGroupDetails positional_group;
  for (const auto &pos : positionals_) {
    for (const auto &g : options.groups()) {
      for (const auto &opt : options.group_help(g).options) {
        if (!opt.l.empty() && opt.l[0] == pos) {
          positional_group.options.push_back(opt);
        }
      }
    }
  }

  line += format_usage(options, positional_group);
  line += "\n\n";

  if (!description_.empty()) {
    line += wrap_text(description_, width_, 0) + "\n\n";
  }

  if (!positional_group.options.empty()) {
    line += format_group("Arguments", positional_group, true);
  }

  for (const auto &group : options.groups()) {
    line += format_group(group, options.group_help(group));
  }

  return line;
}

std::string
specula::app::RichFormatter::format_usage(const cxxopts::Options &options,
                                          const cxxopts::HelpGroupDetails &positionals) const {
  std::string line = "";

  line += fmt::format("{} {} [{}] ",
                      fmt::styled("Usage:", fmt::emphasis::bold | fg(fmt::terminal_color::yellow)),
                      fmt::styled(options.program(), fmt::emphasis::bold),
                      fmt::styled("OPTIONS", fmt::emphasis::bold | fg(fmt::terminal_color::cyan)));

  for (const auto &opt : positionals.options) {
    std::string capitalized = opt.l[0];
    std::transform(capitalized.begin(), capitalized.end(), capitalized.begin(), ::toupper);
    line += fmt::format("{}{}{}{} ",
                        fmt::styled(opt.has_default ? "[" : "",
                                    fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                        fmt::styled(capitalized, fg(fmt::terminal_color::yellow)),
                        fmt::styled(opt.is_container ? "..." : "",
                                    fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                        fmt::styled(opt.has_default ? "]" : "",
                                    fmt::emphasis::faint | fg(fmt::terminal_color::yellow)));
  }

  return line;
}

std::string specula::app::RichFormatter::format_group(const std::string_view &group_name,
                                                      const cxxopts::HelpGroupDetails &group,
                                                      bool is_positional) const {
  std::string line = fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white),
                                 "╭─ {} ─{:─<{}}╮\n", group_name.empty() ? "Options" : group_name,
                                 "", width_ - (group_name.empty() ? 7 : group_name.size()) - 4);

  Widths widths;

  if (is_positional) {
    for (const auto &opt : group.options) {
      format_positional(opt, widths);
    }

    for (const auto &opt : group.options) {
      line += format_positional(opt, widths);
    }
  } else {
    for (const auto &opt : group.options) {
      format_option(opt, widths);
    }

    for (const auto &opt : group.options) {
      line += format_option(opt, widths);
    }
  }
  line +=
      fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "╰{:─<{}}╯\n", "", width_);
  return line;
}

std::string specula::app::RichFormatter::format_positional(const cxxopts::HelpOptionDetails &option,
                                                           Widths &widths) const {
  std::size_t length = 1;
  std::string line = fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "│ ");

  std::string capitalized = option.l[0];
  std::transform(capitalized.begin(), capitalized.end(), capitalized.begin(), ::toupper);

  std::size_t meta_length = capitalized.size() + (option.is_container ? 3 : 0);
  widths.meta = std::max(widths.meta, static_cast<std::uint8_t>(meta_length));
  line += fmt::format("{}{}{:<{}}  ", fmt::styled(capitalized, fg(fmt::terminal_color::yellow)),
                      fmt::styled(option.is_container ? "..." : "",
                                  fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                      "", std::max(std::size_t{0}, widths.meta - meta_length));
  length += 2 + widths.meta;

  std::size_t line_length = length, word_length = 0;

  const auto wrapline = [&]() {
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n│{:{}}", "",
                        width_ - line_length, "", length);
    line_length = length;
  };

  for (std::size_t i = 0; i < option.desc.size(); ++i) {
    if (option.desc[i] == ' ') {
      if (line_length + word_length >= width_) {
        wrapline();
      }
      line += option.desc.substr(i - word_length, word_length);
      line += " ";
      line_length += word_length + 1;
      word_length = 0;
    } else {
      word_length++;
    }
  }

  if (line_length + word_length >= width_) {
    wrapline();
  }
  line += option.desc.substr(option.desc.size() - word_length, word_length);
  line += " ";
  line_length += word_length + 1;

  if (option.has_default) {
    if (line_length + 12 + option.default_value.size() >= width_) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "[default: {}] ",
                        option.default_value);
    line_length += 12 + option.default_value.size();
  } else {
    if (line_length + 11 >= width_) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::red), "[required] ");
    line_length += 11;
  }

  line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n", "",
                      width_ - line_length);
  return line;
}

std::string specula::app::RichFormatter::format_option(const cxxopts::HelpOptionDetails &option,
                                                       Widths &widths) const {
  if (!option.l.empty() &&
      std::find(positionals_.begin(), positionals_.end(), option.l[0]) != positionals_.end())
    return "";
  std::size_t length = 1;
  std::string line = fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "│ ");

  if (!option.s.empty()) {
    line += fmt::format(fg(fmt::terminal_color::green), "-{:<{}}  ", option.s, widths.short_option);
    widths.short_option = std::max(widths.short_option, static_cast<std::uint8_t>(option.s.size()));
    length += 3 + widths.short_option;
  } else if (widths.short_option > 0) {
    line += fmt::format(" {:<{}}  ", "", widths.short_option);
    length += 3 + widths.short_option;
  }

  if (!option.l.empty()) {
    std::string joined = fmt::format("{}", fmt::join(option.l, " --"));
    line += fmt::format(fg(fmt::terminal_color::cyan), "--{:<{}}  ", joined, widths.long_option);
    widths.long_option = std::max(widths.long_option, static_cast<std::uint8_t>(joined.size()));
    length += 4 + widths.long_option;
  } else if (widths.long_option > 0) {
    line += fmt::format("  {:<{}} ", "", widths.long_option);
    length += 4 + widths.long_option;
  }

  if (!option.is_boolean) {
    std::size_t meta_length = (option.arg_help.empty() ? 3 : option.arg_help.size()) +
                              (option.has_implicit ? 2 : 0) + (option.is_container ? 3 : 0);
    widths.meta = std::max(widths.meta, static_cast<std::uint8_t>(meta_length));
    line += fmt::format("{}{}{}{}{:<{}}  ",
                        fmt::styled(option.has_implicit ? "[" : "",
                                    fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                        fmt::styled(option.arg_help.empty() ? "ARG" : option.arg_help,
                                    fg(fmt::terminal_color::yellow)),
                        fmt::styled(option.is_container ? "..." : "",
                                    fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                        fmt::styled(option.has_implicit ? "]" : "",
                                    fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                        "", std::max(std::size_t{0}, widths.meta - meta_length));
    length += 2 + widths.meta;
  } else if (widths.meta > 0) {
    line += fmt::format("{:<{}}  ", "", widths.meta);
    length += 2 + widths.meta;
  }

  std::size_t line_length = length, word_length = 0;

  const auto wrapline = [&]() {
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n│{:{}}", "",
                        width_ - line_length, "", length);
    line_length = length;
  };

  for (std::size_t i = 0; i < option.desc.size(); ++i) {
    if (option.desc[i] == ' ') {
      if (line_length + word_length >= width_) {
        wrapline();
      }
      line += option.desc.substr(i - word_length, word_length);
      line += " ";
      line_length += word_length + 1;
      word_length = 0;
    } else {
      word_length++;
    }
  }

  if (line_length + word_length >= width_) {
    wrapline();
  }
  line += option.desc.substr(option.desc.size() - word_length, word_length);
  line += " ";
  line_length += word_length + 1;

  if (!option.is_boolean && option.has_default) {
    if (line_length + 12 + option.default_value.size() >= width_) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "[default: {}] ",
                        option.default_value);
    line_length += 12 + option.default_value.size();
  }

  if (!option.is_boolean && option.has_implicit) {
    if (line_length + 13 + option.implicit_value.size() >= width_) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "[implicit: {}] ",
                        option.implicit_value);
    line_length += 13 + option.implicit_value.size();
  }

  line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n", "",
                      width_ - line_length);
  return line;
}

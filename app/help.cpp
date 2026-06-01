#include "help.hpp"

#include <algorithm>
#include <cctype>

#include <cxxopts.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

std::string wrap_text(const std::string &text, size_t width, size_t indent = 4) {
  std::string wrapped;
  size_t line_length = 0;
  size_t last_space = 0;
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == ' ') {
      last_space = i;
    }

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

std::string
specula::app::RichFormatter::format_exception(const std::string_view &type,
                                              const cxxopts::exceptions::exception &error) const {
  std::string line = fmt::format(
      "{} {} {}\n", fmt::styled("╭─", fmt::emphasis::faint | fg(fmt::terminal_color::red)),
      fmt::styled(fmt::format("{} Error", type),
                  fmt::emphasis::bold | fg(fmt::terminal_color::red)),
      fmt::styled(fmt::format("─{:─<{}}╮", "", width - type.size() - 10),
                  fmt::emphasis::faint | fg(fmt::terminal_color::red)));

  line += fmt::format("{} {:<{}}{}\n",
                      fmt::styled("│", fmt::emphasis::faint | fg(fmt::terminal_color::red)),
                      error.what(), width - 1,
                      fmt::styled("│", fmt::emphasis::faint | fg(fmt::terminal_color::red)));

  line +=
      fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::red), "╰{:─<{}}╯\n", "", width);
  return line;
}

std::string specula::app::RichFormatter::format_options(const cxxopts::Options &options) const {
  std::string line;

  cxxopts::HelpGroupDetails positional_group;
  for (const auto &pos : positionals) {
    for (const auto &group : options.groups()) {
      for (const auto &opt : options.group_help(group).options) {
        if (!opt.l.empty() && opt.l[0] == pos) {
          positional_group.options.push_back(opt);
        }
      }
    }
  }

  line += format_usage(options, positional_group);
  line += "\n\n";

  if (!description.empty()) {
    line += wrap_text(description, width, 0) + "\n\n";
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
  std::string line;

  line += fmt::format("{} {} [{}] ",
                      fmt::styled("Usage:", fmt::emphasis::bold | fg(fmt::terminal_color::yellow)),
                      fmt::styled(options.program(), fmt::emphasis::bold),
                      fmt::styled("OPTIONS", fmt::emphasis::bold | fg(fmt::terminal_color::cyan)));

  for (const auto &opt : positionals.options) {
    std::string capitalized = opt.l[0];
    std::ranges::transform(capitalized, capitalized.begin(), ::toupper);
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
                                 "", width - (group_name.empty() ? 7 : group_name.size()) - 4);

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
      fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "╰{:─<{}}╯\n", "", width);
  return line;
}

std::size_t specula::app::RichFormatter::append_word_wrapped(std::string &line,
                                                             const std::string &desc,
                                                             std::size_t line_length,
                                                             std::size_t column_start) const {
  std::size_t word_length = 0;

  const auto wrapline = [&]() {
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n│{:{}}", "",
                        width - line_length, "", column_start);
    line_length = column_start;
  };

  for (std::size_t i = 0; i < desc.size(); ++i) {
    if (desc[i] == ' ') {
      if (line_length + word_length >= width) {
        wrapline();
      }
      line += desc.substr(i - word_length, word_length);
      line += " ";
      line_length += word_length + 1;
      word_length = 0;
    } else {
      word_length++;
    }
  }

  if (line_length + word_length >= width) {
    wrapline();
  }
  line += desc.substr(desc.size() - word_length, word_length);
  line += " ";
  return line_length + word_length + 1;
}

std::string specula::app::RichFormatter::format_positional(const cxxopts::HelpOptionDetails &option,
                                                           Widths &widths) const {
  std::size_t length = 1;
  std::string line = fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "│ ");

  std::string capitalized = option.l[0];
  std::ranges::transform(capitalized, capitalized.begin(), ::toupper);

  std::size_t meta_length = capitalized.size() + (option.is_container ? 3 : 0);
  widths.meta = std::max(widths.meta, static_cast<std::uint8_t>(meta_length));
  line += fmt::format("{}{}{:<{}}  ", fmt::styled(capitalized, fg(fmt::terminal_color::yellow)),
                      fmt::styled(option.is_container ? "..." : "",
                                  fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
                      "", std::max(std::size_t{0}, widths.meta - meta_length));
  length += 2 + widths.meta;

  std::size_t line_length = length;

  const auto wrapline = [&]() {
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n│{:{}}", "",
                        width - line_length, "", length);
    line_length = length;
  };

  line_length = append_word_wrapped(line, option.desc, line_length, length);

  if (option.has_default) {
    if (line_length + 12 + option.default_value.size() >= width) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "[default: {}] ",
                        option.default_value);
    line_length += 12 + option.default_value.size();
  } else {
    if (line_length + 11 >= width) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::red), "[required] ");
    line_length += 11;
  }

  line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n", "",
                      width - line_length);
  return line;
}

std::string specula::app::RichFormatter::format_option(const cxxopts::HelpOptionDetails &option,
                                                       Widths &widths) const {
  if (!option.l.empty() && std::ranges::find(positionals, option.l[0]) != positionals.end()) {
    return "";
  }
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

  const std::string open_bracket = option.has_implicit ? "[" : "";
  const std::string arg_display = option.arg_help.empty() ? "ARG" : option.arg_help;
  const std::string container_dots = option.is_container ? "..." : "";
  const std::string close_bracket = option.has_implicit ? "]" : "";
  const std::size_t arg_help_len = option.arg_help.empty() ? 3 : option.arg_help.size();
  const std::size_t implicit_len = option.has_implicit ? 2 : 0;
  const std::size_t container_len = option.is_container ? 3 : 0;

  if (!option.is_boolean) {
    std::size_t meta_length = arg_help_len + implicit_len + container_len;
    widths.meta = std::max(widths.meta, static_cast<std::uint8_t>(meta_length));
    line += fmt::format(
        "{}{}{}{}{:<{}}  ",
        fmt::styled(open_bracket, fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
        fmt::styled(arg_display, fg(fmt::terminal_color::yellow)),
        fmt::styled(container_dots, fmt::emphasis::faint | fg(fmt::terminal_color::yellow)),
        fmt::styled(close_bracket, fmt::emphasis::faint | fg(fmt::terminal_color::yellow)), "",
        std::max(std::size_t{0}, widths.meta - meta_length));
    length += 2 + widths.meta;
  } else if (widths.meta > 0) {
    line += fmt::format("{:<{}}  ", "", widths.meta);
    length += 2 + widths.meta;
  }

  std::size_t line_length = length;

  const auto wrapline = [&]() {
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n│{:{}}", "",
                        width - line_length, "", length);
    line_length = length;
  };

  line_length = append_word_wrapped(line, option.desc, line_length, length);

  if (!option.is_boolean && option.has_default) {
    if (line_length + 12 + option.default_value.size() >= width) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "[default: {}] ",
                        option.default_value);
    line_length += 12 + option.default_value.size();
  }

  if (!option.is_boolean && option.has_implicit) {
    if (line_length + 13 + option.implicit_value.size() >= width) {
      wrapline();
    }
    line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "[implicit: {}] ",
                        option.implicit_value);
    line_length += 13 + option.implicit_value.size();
  }

  line += fmt::format(fmt::emphasis::faint | fg(fmt::terminal_color::white), "{:{}}│\n", "",
                      width - line_length);
  return line;
}

#ifndef INCLUDE_APP_HELP_HPP_
#define INCLUDE_APP_HELP_HPP_

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cxxopts.hpp>

namespace specula::app {
  /**
   * @class RichFormatter
   * @brief Custom cxxopts help formatter that produces a rich, colorized output using ANSI escape
   * codes.
   *
   * Rich help formatting for cxxopts, providing a visually appealing and organized display of help
   * messages, including usage, options, and error messages. It uses ANSI escape codes to add color
   * and styling to the output, making it easier to read and understand. The formatter is closly
   * inspired by [rich-click](https://github.com/ewels/rich-click) and attempts to provide a similar
   * experience for C++ command-line applications.
   */
  class RichFormatter {
  public:
    /**
     * @brief Creates a new RichFormatter instance with the given description and positional
     * arguments.
     *
     * Due to limitations in cxxopts, the description and positional arguments must be provided here
     * as well as to the `cxxopts::Options` instance. This is because cxxopts does not provide a way
     * to access the description or the list of positional arguments which is required for
     * formatting the help message.
     *
     * @param description The description of the application or command, which will be displayed in
     * the help message.
     * @param positionals A list of positional argument names that should be included in the help
     * message.
     */
    RichFormatter(std::string description, std::vector<std::string> positionals)
        : description(std::move(description)), positionals(std::move(positionals)) {}

    RichFormatter(const RichFormatter &) = default;
    RichFormatter(RichFormatter &&) = delete;
    RichFormatter &operator=(const RichFormatter &) = default;
    RichFormatter &operator=(RichFormatter &&) = delete;

    ~RichFormatter() = default;

    /**
     * @brief Generates a formatted help message for the given cxxopts options
     *
     * This is the main interface for formatting the help message. It takes a `cxxopts::Options`
     * instance and extracts the necessary information to generate the help message, including
     * usage, options, and any positional arguments.
     *
     * @param options The `cxxopts::Options` instance for which to generate the help message.
     * @return A string containing the formatted help message, including usage, options, and any
     * positional arguments.
     *
     * @see `format_options`
     */
    std::string operator()(const cxxopts::Options &options) const {
      return format_options(options);
    }

    /**
     * @brief Generates a formatted error message for the given cxxopts specification exception
     *
     * @param error The `cxxopts::exceptions::specification` exception for which to generate the
     * error message.
     * @return A string containing the formatted error message.
     *
     * @see `format_exception`
     */
    std::string operator()(const cxxopts::exceptions::specification &error) const {
      return format_exception("Specification", error);
    }

    /**
     * @brief Generates a formatted error message for the given cxxopts parsing exception.
     *
     * @param error The `cxxopts::exceptions::parsing` exception for which to generate the error
     * message.
     * @return A string containing the formatted error message.
     */
    std::string operator()(const cxxopts::exceptions::parsing &error) const {
      return format_exception("Parsing", error);
    }

    /// @brief Default width for the help message output, to control line wrapping and formatting.
    constexpr static std::uint8_t DEFAULT_WIDTH = 120;

    /// The width to use for formatting the help message. This controls the width of a line before
    /// it wraps to the next line.
    std::size_t width = RichFormatter::DEFAULT_WIDTH;

    /// The description of the application or command, which will be included in the help message.
    std::string description;

    /// A list of positional argument names that should be included in the help message.
    std::vector<std::string> positionals;

  private:
    /**
     * @class Widths
     * @brief A helper struct to keep track of the maximum widths columns in the help message.
     */
    struct Widths {
      /// Maximum width of the short option column (e.g., `-h`)
      std::uint8_t short_option = 0;
      /// Maximum width of the long option column (e.g., `--help`)
      std::uint8_t long_option = 0;
      /// Maximum width of the meta information column (e.g., `[ARG]`, `ARG...`, etc.)
      std::uint8_t meta = 0;
    };

    /**
     * @brief Internal method to format the options and generate the help message.
     *
     * @param options The `cxxopts::Options` instance for which to generate the help message.
     *
     * @return A string containing the formatted help message, including usage, options, and any
     * positional arguments.
     */
    [[nodiscard]] std::string format_options(const cxxopts::Options &options) const;

    /**
     * @brief Internal method to format an exception and generate a formatted error message.
     *
     * @param type The type of the exception (e.g., "Specification", "Parsing") to include as the
     * header of the error message.
     * @param error The `cxxopts::exceptions::exception` instance for which to generate the error
     * message.
     *
     * @return A string containing the formatted error message, including the type of error and the
     * error message from the exception.
     */
    [[nodiscard]] std::string format_exception(const std::string_view &type,
                                               const cxxopts::exceptions::exception &error) const;

    /**
     * @brief Format a usage string for the given options and positional arguments.
     *
     * @param options The `cxxopts::Options` instance for which to generate the usage string.
     * @param positionals The `cxxopts::HelpGroupDetails` containing the positional arguments to
     * include in the usage string.
     *
     * @return A string containing the formatted usage message, including the command name and any
     * positional arguments.
     */
    [[nodiscard]] static std::string format_usage(const cxxopts::Options &options,
                                                  const cxxopts::HelpGroupDetails &positionals);

    /**
     * @brief Generate the help message for a specific group of options.
     *
     * @param group_name The name of the group to format for display in the help message.
     * @param group The `cxxopts::HelpGroupDetails` containing the options in the group to format.
     * @param is_positional Flag indicating whether the group is only positional arguments.
     *
     * @return A string containing the formatted help message for the specified group.
     */
    [[nodiscard]] std::string format_group(const std::string_view &group_name,
                                           const cxxopts::HelpGroupDetails &group,
                                           bool is_positional = false) const;

    /**
     * @brief Format a single option into a line of the help message.
     *
     * @param option The `cxxopts::HelpOptionDetails` containing the details of the option.
     * @param widths The `Widths` struct to update with the maximum widths of the option columns.
     *
     * @return A string containing the formatted line for the option.
     */
    std::string format_option(const cxxopts::HelpOptionDetails &option, Widths &widths) const;

    /**
     * @brief Format a single positional argument into a line of the help message.
     *
     * @param option The `cxxopts::HelpOptionDetails` containing the details of the positional
     * argument.
     * @param widths The `Widths` struct to update with the maximum widths of the option columns.
     *
     * @return A string containing the formatted line for the positional argument.
     */
    std::string format_positional(const cxxopts::HelpOptionDetails &option, Widths &widths) const;

    /**
     * @brief Append a description to a line of the help message, wrapping the text as needed.
     *
     * This splits the `desc` string by whitespace and adding each word to `line` breaking to a new
     * line (including adding the box drawing characters) when the line exceeds the specified width.
     * The `line_length` parameter is updated to reflect the current length of the line after
     * appending the description and any necessary line breaks.
     *
     * @param line The current line to which the description should be appended. This line will be
     * modified with the appended description and any necessary line breaks.
     * @param desc The description text to append to the line.
     * @param line_length The current length of the line before appending the description.
     * @param column_start The indentation column at which the description should start.
     *
     * @return The length of the final line after appending the description and any necessary line
     * breaks.
     */
    std::size_t append_word_wrapped(std::string &line, const std::string &desc,
                                    std::size_t line_length, std::size_t column_start) const;
  };
} // namespace specula::app

#endif // INCLUDE_APP_HELP_HPP_

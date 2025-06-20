/*************************************************************************/ /**
 * \file
 * \brief  contains the declarations of the utility functions.
 * \ingroup Application
 *****************************************************************************/

#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace app {

class Utilities {
 public:
  /**
   * @brief Creates a temporary file and returns its file name.
   * @return The file name of the created temporary file if successful.
   *         An empty string if an error occurred.
   */
  [[nodiscard]] static std::string createTemporaryFile();

  /**
 * @brief Parses a configuration file and returns a key-value pair map of the configurations.
 * @param filePath The path to the configuration file to be parsed.
 * @return A key-value pair map of the configurations.
 * @throws std::runtime_error If the file cannot be opened.
 */
  [[nodiscard]] static std::unordered_map<std::string, std::string> retrieve_key_value_to_map(
      const std::filesystem::path& filePath);

  /**
   * @brief Splits a configuration string into a vector of strings using a regular expression.
   * @param inputString The input string to be parsed.
   * @param regExpStr The regular expression used to split the input string. (e.g. "[,;]")
   * @return A vector of strings parsed from the input string.
   */
  [[nodiscard]] static std::vector<std::string> split_config_string_regexp(const std::string& inputString,
                                                                           const std::string& regExpStr);

  /**
   * @brief Prints an error message to the standard error stream.
   * @param message The error message to be printed.
   */
  static void printError(const std::string& message);
};

}  // namespace app

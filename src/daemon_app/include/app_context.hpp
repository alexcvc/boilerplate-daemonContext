/**
* \file
 * \brief   contains the application context class
 * \ingroup Daemon with Application Context
 */

#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>

#include "app_context_base.hpp"

namespace app {

/**
 * @brief The UdpAddress type represents a UDP address in case of IEC61850-90-5.
 */
using UdpAddress = std::pair<std::string, uint16_t>;

/**
 * @brief The AppContext class provides implementation application context functions.
 */
class AppContext : public IAppContext {
  static constexpr std::string_view kDefaultConfigFile{"settings.xml"};  ///< The name of the XML configuration file
  std::filesystem::path m_pathConfigFile;                                ///< The path of the configuration file
  std::filesystem::path m_pathConfigFolder;                              ///< The path of the configuration folder
  std::mutex m_mutex;  ///< The mutex for the context start-restart-stop

 public:
  /// constructor
  AppContext() = default;

  /// destructor
  ~AppContext() override = default;

  /**
   * @brief Validates the configuration of the daemon.
   * @param config The configuration of the daemon to validate.
   * @return An optional boolean value.
   */
  [[nodiscard]] std::optional<bool> validate_configuration(const app::DaemonConfig& config) override;

  /**
   * @brief Process everything before reconfiguring the application.
   * @return std::optional<bool> true if the process completes successfully, otherwise false.
   */
  [[nodiscard]] std::optional<bool> process_reconfigure() override;

  /**
   * @brief Process everything before starting the application.
   * @return std::optional<bool> - true if the process_start is successful, false otherwise.
   * If the method is not implemented, the return value is not defined.
   */
  [[nodiscard]] std::optional<bool> process_start() override;

  /**
   * @brief Process everything before restarting the application
   * @return std::optional<bool> - true if the restart process was successful, false otherwise.
   * @note This function is pure virtual and must be implemented by the derived class.
   */
  [[nodiscard]] std::optional<bool> process_restart() override;

  /**
   * @brief Process everything after USER1 signal
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] std::optional<bool> process_user1() override;

  /**
   * @brief Process everything after USER2 signal
   * @return std::optional<bool> true if successful to start, otherwise false.
   * Not defined return value means that the function is not implemented.
   */
  [[nodiscard]] std::optional<bool> process_user2() override;

  /**
   * @brief Performs a graceful shutdown of the application.
   * @return An optional boolean value indicating the success of the shutdown process.
   *         The optional value will be empty if the shutdown process encountered an error.
   */
  [[nodiscard]] std::optional<bool> process_shutdown() override;

  /**
   * @brief processing the context.
   * @param min_duration minimum duration until next processing.
   * @return The earlier timeout until the next process.
   */
  [[nodiscard]] std::chrono::milliseconds process_executing(const std::chrono::milliseconds& min_duration) override;

  /**
   * @brief Set the path of the configuration file.
   * @param path The path of the configuration file.
   * @param isMandatory
   * @return true if the path exists or empty, otherwise false.
   */
  [[nodiscard]] bool validate_path(const std::string& path, const std::string& desc, bool isMandatory) const;

  [[nodiscard]] const std::filesystem::path& getPathConfigFolder() const {
    return m_pathConfigFolder;
  }
};

}  // namespace app

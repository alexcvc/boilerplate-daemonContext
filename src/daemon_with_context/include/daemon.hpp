/**
 * @file
 * @brief   contains the daemon class
 * @ingroup Daemon with Application Context
 */

#pragma once

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include <csignal>
#include <functional>
#include <optional>
#include <string>

//----------------------------------------------------------------------------
// Declarations
//----------------------------------------------------------------------------
namespace app {
class Daemon {
 public:
  /**
   * @brief The state of the daemon.
   */
  enum class State { start, running, reload, stop, user1, user2 };

  /**
   * @brief Signals to exit the daemon.
   */
  static constexpr int ExitSignal = SIGINT;        ///< Signal to exit the daemon
  static constexpr int TerminateSignal = SIGTERM;  ///< Signal to terminate the daemon
  static constexpr int ReloadSignal = SIGHUP;      ///< Signal to reload the daemon
  static constexpr int UserSignal1 = SIGUSR1;      ///< Signal to execute user defined action
  static constexpr int UserSignal2 = SIGUSR2;      ///< Signal to execute user defined action

  /**
   * @brief Gets the instance of the daemon.
   * @return The instance of the daemon.
   */
  static Daemon& instance() {
    static Daemon instance;
    return instance;
  }

  /**
   * @brief Starts the daemon.
   * @return True if the daemon is started, false otherwise.
   */
  [[nodiscard]] std::optional<bool> start_all() {
    m_state = State::running;
    if (m_handlerBeforeToStart) {
      return m_handlerBeforeToStart();
    }
    return std::nullopt;
  }

  /**
   * @brief Reloads the daemon.
   * @return True if the daemon is reloaded, false otherwise.
   */
  [[nodiscard]] std::optional<bool> reload_all() {
    m_state = State::reload;
    return std::nullopt;
  }

  /**
   * @brief Closes the daemon.
   * @return True if the daemon is closed, false otherwise.
   */
  [[nodiscard]] std::optional<bool> close_all() {
    m_state = State::stop;
    if (m_handlerBeforeToExit) {
      return m_handlerBeforeToExit();
    }
    return std::nullopt;
  }

  /**
   * @brief Sets the function to be called before the daemon starts.
   * @param func The function to be called.
   */
  void set_start_function(std::function<std::optional<bool>()> func) {
    m_handlerBeforeToStart = func;
  }

  /**
   * @brief Sets the function to be called when reloaded.
   * @param func The function to be called.
   */
  void set_reload_function(std::function<std::optional<bool>()> func) {
    m_handlerReload = func;
  }

  /**
   * @brief Sets the function to be called in case of USER1 signal.
   * @param func The function to be called.
   */
  void set_user1_function(std::function<std::optional<bool>()> func) {
    m_handlerUser1 = func;
  }

  /**
   * @brief Sets the function to be called in case of USER2 signal.
   * @param func The function to be called.
   */
  void set_user2_function(std::function<std::optional<bool>()> func) {
    m_handlerUser2 = func;
  }

  /**
   * @brief Sets the function to be called before exits.
   * @param func The function to be called.
   */
  void set_close_function(std::function<std::optional<bool>()> func) {
    m_handlerBeforeToExit = func;
  }

  /**
   * @brief Checks if the daemon is running.
   * @return True if the daemon is running, false otherwise.
   */
  [[nodiscard]] bool is_running() {
    if (m_state == State::reload) {
      perform_reload_if_required();
    } else if (m_state == State::user1) {
      perform_user1_if_required();
    } else if (m_state == State::user2) {
      perform_user2_if_required();
    }
    return m_state == State::running;
  }

  /**
   * @brief Gets the state of the daemon.
   * @return The state of the daemon.
   */
  [[nodiscard]] State get_state() const {
    return m_state;
  }

  /**
   * @brief Sets the state of the daemon.
   * @param stop The state to be set.
   */
  void set_state(State stop) {
    m_state = stop;
  }

  /**
   * @brief Makes process as daemon in background.
   * @param name of the PID file.
   */
  std::optional<bool> make_daemon(const std::string& pid_file_name);

 private:
  Daemon();
  Daemon(Daemon const&) = delete;
  void operator=(Daemon const&) = delete;

  /**
   * @brief Performs the reload if required.
   */
  void perform_reload_if_required() {
    m_state = State::running;
    if (m_handlerReload) {
      if (m_handlerReload() == false) {
        m_state = State::stop;
      }
    }
  }

  /**
   * @brief Performs the user1 operation if required.
   */
  void perform_user1_if_required() {
    m_state = State::running;
    if (m_handlerUser1) {
      if (m_handlerUser1() == false) {
        m_state = State::stop;
      }
    }
  }

  /**
   * @brief Performs the user2 operation if required.
   */
  void perform_user2_if_required() {
    m_state = State::running;
    if (m_handlerUser2) {
      if (m_handlerUser2() == false) {
        m_state = State::stop;
      }
    }
  }

  /**
   * @brief Handles the signals.
   * @param signal The signal to be handled.
   */
  static void signal_handler(int signal);

  /**
   * @brief Creates a child process.
   * @return The process ID of the child process.
   */
  pid_t create_child_process();

  /**
   * @brief Initializes the child process.
   * @return True if the child process is initialized successfully, false otherwise.
   */
  bool init_child_process();

  /**
   * @brief Writes the process ID to a file.
   * @param pid_file_name The name of the PID file.
   * @return True if the process ID is written to the file, false otherwise.
   */
  bool write_pid_to_file(const std::string& pid_file_name);

  // Member variables
  State m_state;                                                ///< State of the daemon
  std::function<std::optional<bool>()> m_handlerBeforeToStart;  ///< Function to be called before the daemon starts
  std::function<std::optional<bool>()> m_handlerReload;         ///< Function to be called when the daemon is reloaded
  std::function<std::optional<bool>()> m_handlerUser1;          ///< Function to be called by USER1 signal
  std::function<std::optional<bool>()> m_handlerUser2;          ///< Function to be called by USER2 signal
  std::function<std::optional<bool>()> m_handlerBeforeToExit;   ///< Function to be called before the daemon exits
};

}  // namespace app
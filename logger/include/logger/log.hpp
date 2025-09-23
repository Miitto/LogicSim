#pragma once

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <memory>

#define __CONCAT(a, b) a##b

#define DEFINE_LOGGER(_LOGGER_NAME, _LOGGER_LEVEL)                             \
                                                                               \
  std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;                  \
                                                                               \
  void Logger::init() noexcept {                                               \
    s_logger = spdlog::stdout_color_mt(_LOGGER_NAME);                          \
    s_logger->set_level(__CONCAT(spdlog::level::, _LOGGER_LEVEL));             \
  }                                                                            \
                                                                               \
  void Logger::ensureInit() noexcept {                                         \
    if (!s_logger) {                                                           \
      init();                                                                  \
    }                                                                          \
  }

#define DECLARE_LOGGER                                                         \
                                                                               \
  class Logger {                                                               \
  private:                                                                     \
    static std::shared_ptr<spdlog::logger> s_logger;                           \
                                                                               \
    static void ensureInit() noexcept;                                         \
                                                                               \
  public:                                                                      \
    static auto init() noexcept -> void;                                       \
                                                                               \
    template <typename... Args>                                                \
    static auto log(spdlog::level::level_enum lvl,                             \
                    spdlog::format_string_t<Args...> fmt,                      \
                    Args&&... args) noexcept -> void {                         \
      ensureInit();                                                            \
      s_logger->log(lvl, fmt, std::forward<Args>(args)...);                    \
    }                                                                          \
                                                                               \
    template <typename... Args>                                                \
    static auto trace(spdlog::format_string_t<Args...> fmt,                    \
                      Args&&... args) noexcept -> void {                       \
      ensureInit();                                                            \
      s_logger->trace(fmt, std::forward<Args>(args)...);                       \
    }                                                                          \
    template <typename... Args>                                                \
    static auto debug(spdlog::format_string_t<Args...> fmt,                    \
                      Args&&... args) noexcept -> void {                       \
      ensureInit();                                                            \
      s_logger->debug(fmt, std::forward<Args>(args)...);                       \
    }                                                                          \
    template <typename... Args>                                                \
    static auto info(spdlog::format_string_t<Args...> fmt,                     \
                     Args&&... args) noexcept -> void {                        \
      ensureInit();                                                            \
      s_logger->info(fmt, std::forward<Args>(args)...);                        \
    }                                                                          \
    template <typename... Args>                                                \
    static auto warn(spdlog::format_string_t<Args...> fmt,                     \
                     Args&&... args) noexcept -> void {                        \
      ensureInit();                                                            \
      s_logger->warn(fmt, std::forward<Args>(args)...);                        \
    }                                                                          \
    template <typename... Args>                                                \
    static auto error(spdlog::format_string_t<Args...> fmt,                    \
                      Args&&... args) noexcept -> void {                       \
      ensureInit();                                                            \
      s_logger->error(fmt, std::forward<Args>(args)...);                       \
    }                                                                          \
    template <typename... Args>                                                \
    static auto critical(spdlog::format_string_t<Args...> fmt,                 \
                         Args&&... args) noexcept -> void {                    \
      ensureInit();                                                            \
      s_logger->critical(fmt, std::forward<Args>(args)...);                    \
    }                                                                          \
  };
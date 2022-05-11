#pragma once

#include <chrono>
#include <cstdint>
#include <format>
#include <map>
#include <mutex>
#include <string>

#include "LoggingService.h"

#define LOGGER(T) auto logger = std::make_unique<Logger>(#T);

namespace UnrealVr {
    enum class LogLevel : uint8_t {
        Info = 0,
        Warn = 1,
        Error = 2
    };

    static constexpr std::map<LogLevel, std::string> LogLevelStrings = {
        { LogLevel::Info, "Info " },
        { LogLevel::Warn, "Warn " },
        { LogLevel::Error, "Error" }
    };

    /**
     * A generic logger which outputs to the app UI (UnrealVRLauncher)
     *
     * Note that this class uses C++ 20's std::format, _not_ % formatting
     */
    class Logger {
    public:
        Logger(std::string source);

        template<typename... Args>
        void Log(LogLevel logLevel, const std::string& format, Args ...args) {
            auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
            auto line = std::format("[{:%F %T}] [{}] [{}] " + format + "\n", now, source, logLevel, args...);
            {
                std::lock_guard guard(LoggingService::bufferMtx);
                LoggingService::buffer += line;
            }
            LoggingService::bufferCv.notify_all();
        }

        template<typename... Args>
        void Info(const std::string& format, Args ...args) {
            Log(LogLevel::Info, format, args...);
        }

        template<typename... Args>
        void Warn(const std::string& format, Args ...args) {
            Log(LogLevel::Warn, format, args...);
        }

        template<typename... Args>
        void Error(const std::string& format, Args ...args) {
            Log(LogLevel::Error, format, args...);
        }

    private:
        std::string source;
    };
}
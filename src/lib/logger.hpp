#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif
#include <shared_mutex>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

namespace arisu::impl
{
    class Logger
    {
        [[nodiscard]] static std::string format_time()
        {
            using std::chrono::system_clock;
            using std::chrono::time_point;
            time_point<system_clock> t = system_clock::now();
            return fmt::format("[{:%H:%M:%S}]", t);
        }

        #ifdef _WIN32
        bool _initialized{};
        void* _std_handle{};
        std::shared_mutex _mutex{};
        #endif

    public:
        Logger()                         = default;
        Logger(const Logger&)            = delete;
        Logger(Logger&&)                 = delete;
        Logger& operator=(const Logger&) = delete;
        Logger& operator=(Logger&&)      = delete;

        #ifdef _WIN32
        bool setup(std::string_view title = "P1 DEBUG")
        {
            _initialized = AllocConsole();

            freopen_s(reinterpret_cast<_iobuf**>(stdout), "CONOUT$", "w", stdout);
            SetConsoleTitleA(title.data());

            // enable color support
            _std_handle = GetStdHandle(STD_OUTPUT_HANDLE);

            // its ok if the handle is valid
            if (_std_handle == INVALID_HANDLE_VALUE)
                return true;

            DWORD mode{};
            if (!GetConsoleMode(_std_handle, &mode))
                return true;

            // already enabled
            if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                return true;

            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(_std_handle, mode);

            return true;
        }

        void destroy()
        {
            if (!_initialized)
                return;
            _initialized = false;
            fclose(stdout);
            FreeConsole();
        }
        #endif

        template <typename... Args>
        auto info(fmt::format_string<Args...> fmt, Args&&... args)
        {
            if (!_initialized)
                return;
            fmt::println("{} [-] {}", format_time(), fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        auto info(fmt::text_style text_style, fmt::format_string<Args...> fmt, Args&&... args)
        {
            if (!_initialized)
                return;
            fmt::print(text_style, "{} [-] {}\n", format_time(), fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        auto error(fmt::format_string<Args...> fmt, Args&&... args)
        {
            if (!_initialized)
                return;
            fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "{} [x] {}\n", format_time(), fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        auto success(fmt::format_string<Args...> fmt, Args&&... args)
        {
            if (!_initialized)
                return;
            fmt::print(fmt::emphasis::bold | fg(fmt::color::light_green), "{} [+] {}\n", format_time(), fmt::format(fmt, std::forward<Args>(args)...));
        }
    };
}

namespace arisu
{
    inline impl::Logger logger{};
}

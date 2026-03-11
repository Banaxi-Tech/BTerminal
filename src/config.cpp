#include "config.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#ifdef HAS_TOMLPP
#  include <toml++/toml.h>
#endif

namespace {

std::string default_config_dir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        return {};
    }
    std::filesystem::path path = home;
    path /= ".config/bterminal";
    return path.string();
}

std::string default_config_path() {
    auto dir = default_config_dir();
    if (dir.empty()) {
        return {};
    }
    std::filesystem::path path = dir;
    path /= "config.toml";
    return path.string();
}

} // namespace

void Config::load_from_default_path() {
    const auto path = default_config_path();
    if (path.empty() || !std::filesystem::exists(path)) {
        ensure_defaults();
        return;
    }

#ifdef HAS_TOMLPP
    try {
        auto table = toml::parse_file(path);

        if (auto* w = table["window"].as_table()) {
            if (auto v = (*w)["width"].value<int64_t>()) {
                window_width = static_cast<int>(*v);
            }
            if (auto v = (*w)["height"].value<int64_t>()) {
                window_height = static_cast<int>(*v);
            }
        }

        if (auto* t = table["terminal"].as_table()) {
            if (auto v = (*t)["font"].value<std::string>()) {
                font = *v;
            }
            if (auto v = (*t)["scrollback"].value<int64_t>()) {
                scrollback = static_cast<int>(*v);
            }
            if (auto v = (*t)["shell"].value<std::string>()) {
                shell = *v;
            }
            if (auto v = (*t)["cursor_shape"].value<std::string>()) {
                cursor_shape = *v;
            }
            if (auto v = (*t)["cursor_blink"].value<bool>()) {
                cursor_blink = *v;
            }
        }

        if (auto* c = table["colors"].as_table()) {
            if (auto v = (*c)["theme"].value<std::string>()) {
                theme = *v;
            }
            if (auto v = (*c)["foreground"].value<std::string>()) {
                foreground_color = *v;
            }
            if (auto v = (*c)["background"].value<std::string>()) {
                background_color = *v;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config " << path << ": " << e.what() << "\n";
    }
#else
    (void)path;
#endif
}

void Config::save_to_default_path() const {
    const auto path = default_config_path();
    if (path.empty()) {
        return;
    }

#ifdef HAS_TOMLPP
    toml::table table;
    
    toml::table window;
    if (window_width) window.insert_or_assign("width", (int64_t)*window_width);
    if (window_height) window.insert_or_assign("height", (int64_t)*window_height);
    table.insert_or_assign("window", window);

    toml::table terminal;
    if (font) terminal.insert_or_assign("font", *font);
    if (scrollback) terminal.insert_or_assign("scrollback", (int64_t)*scrollback);
    if (shell) terminal.insert_or_assign("shell", *shell);
    if (cursor_shape) terminal.insert_or_assign("cursor_shape", *cursor_shape);
    if (cursor_blink) terminal.insert_or_assign("cursor_blink", *cursor_blink);
    table.insert_or_assign("terminal", terminal);

    toml::table colors;
    if (theme) colors.insert_or_assign("theme", *theme);
    if (foreground_color) colors.insert_or_assign("foreground", *foreground_color);
    if (background_color) colors.insert_or_assign("background", *background_color);
    table.insert_or_assign("colors", colors);

    std::ofstream out(path);
    if (out) {
        out << table;
    }
#else
    std::cerr << "Cannot save config: toml++ not available.\n";
#endif
}

void Config::ensure_defaults() {
    auto dir = default_config_dir();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    const auto path = default_config_path();
    if (!path.empty() && !std::filesystem::exists(path)) {
        // Set some defaults
        window_width = 900;
        window_height = 580;
        font = "monospace 11";
        scrollback = 10000;
        shell = "/bin/zsh";
        cursor_shape = "block";
        cursor_blink = true;
        theme = "bnight";
        
        save_to_default_path();
    }
}


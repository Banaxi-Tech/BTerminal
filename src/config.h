#pragma once

#include <optional>
#include <string>

struct Config {
    // Window
    std::optional<int> window_width;
    std::optional<int> window_height;

    // Terminal
    std::optional<std::string> font; // e.g. "JetBrains Mono 13"
    std::optional<int> scrollback;
    std::optional<std::string> shell;
    std::optional<std::string> cursor_shape; // block, ibeam, underline
    std::optional<bool> cursor_blink;

    // Colors
    std::optional<std::string> theme; // bnight, dracula, nord, gruvbox
    std::optional<std::string> foreground_color;
    std::optional<std::string> background_color;

    void load_from_default_path();
    void save_to_default_path() const;
    void ensure_defaults();
};


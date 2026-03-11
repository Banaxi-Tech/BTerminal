#pragma once

#include <array>
#include <string>
#include <gdk/gdk.h>

struct TerminalColors {
    std::array<GdkRGBA, 16> palette;
    GdkRGBA foreground;
    GdkRGBA background;
    GdkRGBA cursor;
};

TerminalColors load_theme(const std::string& name);


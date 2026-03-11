#include "colors.h"

#include <gdk/gdk.h>

namespace {

GdkRGBA rgba(double r, double g, double b, double a = 1.0) {
    GdkRGBA c;
    c.red = r / 255.0;
    c.green = g / 255.0;
    c.blue = b / 255.0;
    c.alpha = a;
    return c;
}

TerminalColors catppuccin_mocha_bnight() {
    TerminalColors c{};

    c.background = rgba(30, 30, 46);
    c.foreground = rgba(205, 214, 244);
    c.cursor = rgba(245, 194, 231);

    c.palette[0]  = rgba(30, 30, 46);
    c.palette[1]  = rgba(243, 139, 168);
    c.palette[2]  = rgba(166, 227, 161);
    c.palette[3]  = rgba(249, 226, 175);
    c.palette[4]  = rgba(137, 180, 250);
    c.palette[5]  = rgba(203, 166, 247);
    c.palette[6]  = rgba(148, 226, 213);
    c.palette[7]  = rgba(205, 214, 244);

    c.palette[8]  = rgba(88, 91, 112);
    c.palette[9]  = rgba(243, 139, 168);
    c.palette[10] = rgba(166, 227, 161);
    c.palette[11] = rgba(249, 226, 175);
    c.palette[12] = rgba(137, 180, 250);
    c.palette[13] = rgba(203, 166, 247);
    c.palette[14] = rgba(148, 226, 213);
    c.palette[15] = rgba(243, 244, 248);

    return c;
}

} // namespace

TerminalColors load_theme(const std::string& name) {
    (void)name;
    return catppuccin_mocha_bnight();
}


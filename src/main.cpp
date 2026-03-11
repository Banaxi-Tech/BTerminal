#include <gtk/gtk.h>
#include <vte/vte.h>

#include <iostream>
#include <memory>
#include <string>

#include "window.h"
#include "config.h"

namespace {

void print_help(const char* argv0) {
    std::cout << "BTerminal " << BTERMINAL_VERSION << "\n"
              << "\n"
              << "Usage: " << argv0 << " [OPTIONS]\n"
              << "\n"
              << "Options:\n"
              << "  --help        Show this help message and exit\n"
              << "  --version     Show version information and exit\n";
}

} // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help(argv[0]);
            return 0;
        }
        if (arg == "--version" || arg == "-v") {
            std::cout << "BTerminal " << BTERMINAL_VERSION << "\n";
            return 0;
        }
    }

    auto config = std::make_unique<Config>();
    config->load_from_default_path();

    g_setenv("TERM", "xterm-256color", FALSE);
    g_setenv("COLORTERM", "truecolor", FALSE);

    // Application ID must be a valid reverse-DNS identifier.
    auto app = gtk_application_new("com.banaxi.bterminal", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(+[] (GtkApplication* app, gpointer user_data) {
        auto* cfg = static_cast<Config*>(user_data);
        GtkWindow* window = create_main_window(app, *cfg);
        gtk_window_present(window);
    }), config.get());

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}


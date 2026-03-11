#include "terminal.h"

#include <string>

#include <pango/pango.h>

#include "colors.h"

namespace {

const char* default_shell(const Config& config) {
    if (config.shell && !config.shell->empty()) {
        return config.shell->c_str();
    }
    return "/bin/zsh";
}

int default_scrollback(const Config& config) {
    return config.scrollback.value_or(10000);
}

std::string default_font(const Config& config) {
    if (config.font && !config.font->empty()) {
        return *config.font;
    }
    // Use generic monospace family so something sensible is always chosen.
    return "monospace 11";
}

VteCursorShape cursor_shape_from_config(const Config& config) {
    if (!config.cursor_shape) {
        return VTE_CURSOR_SHAPE_BLOCK;
    }
    const auto& s = *config.cursor_shape;
    if (s == "ibeam") {
        return VTE_CURSOR_SHAPE_IBEAM;
    }
    if (s == "underline") {
        return VTE_CURSOR_SHAPE_UNDERLINE;
    }
    return VTE_CURSOR_SHAPE_BLOCK;
}

void update_tab_title_from_terminal(GtkNotebook* notebook, GtkWidget* terminal_widget) {
    int page = gtk_notebook_page_num(notebook, terminal_widget);
    if (page < 0) {
        return;
    }

    auto* term = VTE_TERMINAL(terminal_widget);
    const char* uri = vte_terminal_get_current_directory_uri(term);
    std::string title = "BTerminal";
    if (uri) {
        std::string s(uri);
        auto pos = s.rfind('/');
        if (pos != std::string::npos) {
            title = s.substr(pos + 1);
        } else {
            title = s;
        }
    }

    GtkWidget* label = gtk_label_new(title.c_str());
    gtk_notebook_set_tab_label(notebook, terminal_widget, label);
}

void on_child_exited(VteTerminal* term, int status, gpointer user_data) {
    (void)status;
    auto* notebook = GTK_NOTEBOOK(user_data);
    GtkWidget* widget = GTK_WIDGET(term);
    int page = gtk_notebook_page_num(notebook, widget);
    if (page >= 0) {
        gtk_notebook_remove_page(notebook, page);
    }
}

void on_directory_changed(VteTerminal* term, gpointer user_data) {
    auto* notebook = GTK_NOTEBOOK(user_data);
    update_tab_title_from_terminal(notebook, GTK_WIDGET(term));
}

} // namespace

void create_new_terminal_tab(GtkNotebook* notebook, const Config& config) {
    GtkWidget* term = vte_terminal_new();
    gtk_widget_set_hexpand(term, TRUE);
    gtk_widget_set_vexpand(term, TRUE);

    TerminalColors colors = load_theme(config.theme.value_or("bnight"));
    
    // Override colors if set in config
    if (config.foreground_color) {
        gdk_rgba_parse(&colors.foreground, config.foreground_color->c_str());
    }
    if (config.background_color) {
        gdk_rgba_parse(&colors.background, config.background_color->c_str());
        // Force fully transparent for compositor if needed (matching original theme intent)
        // colors.background.alpha = 0.0;
    }

    vte_terminal_set_colors(VTE_TERMINAL(term),
                            &colors.foreground,
                            &colors.background,
                            colors.palette.data(),
                            colors.palette.size());
    vte_terminal_set_scrollback_lines(VTE_TERMINAL(term), default_scrollback(config));
    vte_terminal_set_cursor_shape(VTE_TERMINAL(term), cursor_shape_from_config(config));
    vte_terminal_set_cursor_blink_mode(
        VTE_TERMINAL(term),
        config.cursor_blink.value_or(true) ? VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF);
    vte_terminal_set_audible_bell(VTE_TERMINAL(term), FALSE);
    vte_terminal_set_allow_bold(VTE_TERMINAL(term), TRUE);
    vte_terminal_set_word_char_exceptions(VTE_TERMINAL(term), "-,./?%&#:_~");

    vte_terminal_set_font(VTE_TERMINAL(term),
                          pango_font_description_from_string(default_font(config).c_str()));

    char* argv[] = {
        const_cast<char*>(default_shell(config)),
        const_cast<char*>("-l"),
        nullptr
    };

    vte_terminal_spawn_async(
        VTE_TERMINAL(term),
        VTE_PTY_DEFAULT,
        nullptr,
        argv,
        nullptr,
        G_SPAWN_DEFAULT,
        nullptr,
        nullptr,
        nullptr,
        -1,
        nullptr,
        nullptr,
        nullptr);

    int page = gtk_notebook_append_page(notebook, term, gtk_label_new("BTerminal"));
    gtk_notebook_set_tab_reorderable(notebook, term, TRUE);
    gtk_notebook_set_tab_detachable(notebook, term, TRUE);
    gtk_widget_show(term);

    g_signal_connect(term, "child-exited", G_CALLBACK(on_child_exited), notebook);
    g_signal_connect(term, "current-directory-changed", G_CALLBACK(on_directory_changed), notebook);
}


#include "window.h"

#include <gtk/gtk.h>
#include <vte/vte.h>

#include <string>

#include "terminal.h"
#include "keybindings.h"
#include "colors.h"

namespace {

constexpr int DEFAULT_WIDTH = 900;
constexpr int DEFAULT_HEIGHT = 580;

void apply_css(GtkWidget* widget) {
    GtkCssProvider* provider = gtk_css_provider_new();

    gchar* css_path = g_build_filename(g_get_user_data_dir(), "bterminal", "bterminal.css", nullptr);
    if (!g_file_test(css_path, G_FILE_TEST_EXISTS)) {
        g_free(css_path);
        css_path = g_build_filename(PKGDATADIR, "bterminal.css", nullptr);
    }

    gtk_css_provider_load_from_path(provider, css_path);
    g_free(css_path);

    GdkDisplay* display = gtk_widget_get_display(widget);
    gtk_style_context_add_provider_for_display(display,
                                               GTK_STYLE_PROVIDER(provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

} // namespace

GtkWindow* create_main_window(GtkApplication* app, const Config& config) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "BTerminal");
    gtk_window_set_default_size(GTK_WINDOW(window),
                                config.window_width.value_or(DEFAULT_WIDTH),
                                config.window_height.value_or(DEFAULT_HEIGHT));
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);

    gtk_widget_set_name(window, "bterminal-window");
    gtk_widget_add_css_class(window, "terminal-window");

    apply_css(window);

    GtkWidget* header = gtk_header_bar_new();
    gtk_widget_set_size_request(header, -1, 28);
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(header), "icon:minimize,maximize,close");
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), gtk_label_new("BTERMINAL"));
    gtk_widget_add_css_class(header, "header-bar");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(vbox, TRUE);
    gtk_widget_set_vexpand(vbox, TRUE);

    GtkWidget* notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);

    gtk_widget_add_css_class(notebook, "terminal-notebook");

    GtkWidget* search_revealer = gtk_revealer_new();
    gtk_revealer_set_reveal_child(GTK_REVEALER(search_revealer), FALSE);

    GtkWidget* search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget* search_entry = gtk_search_entry_new();
    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_box_append(GTK_BOX(search_box), search_entry);

    gtk_revealer_set_child(GTK_REVEALER(search_revealer), search_box);

    gtk_box_append(GTK_BOX(vbox), notebook);
    gtk_box_append(GTK_BOX(vbox), search_revealer);

    gtk_window_set_child(GTK_WINDOW(window), vbox);

    create_new_terminal_tab(GTK_NOTEBOOK(notebook), config);

    setup_global_keybindings(window,
                             GTK_NOTEBOOK(notebook),
                             search_revealer,
                             search_entry,
                             config);

    gtk_window_set_default_icon_name("bterminal");
    return GTK_WINDOW(window);
}


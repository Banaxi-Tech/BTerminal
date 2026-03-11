#include "config_gui.h"

#include <iostream>
#include <vte/vte.h>
#include "colors.h"

namespace {

struct GUIContext {
    Config* config;
    GtkWindow* settings_window;
    GtkWindow* main_window;
    GtkNotebook* notebook;
    GtkFontDialogButton* font_button;
    GtkColorDialogButton* fg_button;
    GtkColorDialogButton* bg_button;
    GtkSpinButton* scrollback_spin;
};

void apply_config_live(GUIContext* ctx) {
    // Apply to all terminal tabs
    int n_pages = gtk_notebook_get_n_pages(ctx->notebook);
    for (int i = 0; i < n_pages; i++) {
        GtkWidget* term = gtk_notebook_get_nth_page(ctx->notebook, i);
        if (VTE_IS_TERMINAL(term)) {
            // Update font
            if (ctx->config->font) {
                vte_terminal_set_font(VTE_TERMINAL(term), 
                    pango_font_description_from_string(ctx->config->font->c_str()));
            }

            // Update colors
            TerminalColors colors = load_theme(ctx->config->theme.value_or("bnight"));
            if (ctx->config->foreground_color) {
                gdk_rgba_parse(&colors.foreground, ctx->config->foreground_color->c_str());
            }
            if (ctx->config->background_color) {
                gdk_rgba_parse(&colors.background, ctx->config->background_color->c_str());
            }

            vte_terminal_set_colors(VTE_TERMINAL(term),
                                    &colors.foreground,
                                    &colors.background,
                                    colors.palette.data(),
                                    colors.palette.size());
            
            // Update scrollback
            vte_terminal_set_scrollback_lines(VTE_TERMINAL(term), 
                ctx->config->scrollback.value_or(10000));
            
            // Update cursor
            vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term),
                ctx->config->cursor_blink.value_or(true) ? VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF);
        }
    }
}

void on_save_clicked(GtkButton* button, gpointer user_data) {
    auto* ctx = static_cast<GUIContext*>(user_data);
    
    // Read values from GUI
    if (ctx->font_button) {
        PangoFontDescription* desc = gtk_font_dialog_button_get_font_desc(ctx->font_button);
        if (desc) {
            ctx->config->font = pango_font_description_to_string(desc);
        }
    }
    
    ctx->config->scrollback = (int)gtk_spin_button_get_value(ctx->scrollback_spin);

    // Get colors
    const GdkRGBA* fg = gtk_color_dialog_button_get_rgba(ctx->fg_button);
    const GdkRGBA* bg = gtk_color_dialog_button_get_rgba(ctx->bg_button);
    
    if (fg) ctx->config->foreground_color = gdk_rgba_to_string(fg);
    if (bg) ctx->config->background_color = gdk_rgba_to_string(bg);

    ctx->config->save_to_default_path();
    apply_config_live(ctx);
    gtk_window_destroy(ctx->settings_window);
}

} // namespace

void show_config_gui(GtkWindow* parent, GtkNotebook* notebook, Config* config) {
    GtkWidget* window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "BTerminal Settings");
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, -1);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    auto* ctx = g_new0(GUIContext, 1);
    ctx->config = config;
    ctx->settings_window = GTK_WINDOW(window);
    ctx->main_window = parent;
    ctx->notebook = notebook;

    // Load current theme colors for default color pickers
    TerminalColors theme_colors = load_theme(config->theme.value_or("bnight"));

    // Font
    gtk_box_append(GTK_BOX(vbox), gtk_label_new("Terminal Font:"));
    GtkFontDialog* font_dialog = gtk_font_dialog_new();
    ctx->font_button = GTK_FONT_DIALOG_BUTTON(gtk_font_dialog_button_new(font_dialog));
    if (config->font) {
        gtk_font_dialog_button_set_font_desc(ctx->font_button, pango_font_description_from_string(config->font->c_str()));
    }
    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(ctx->font_button));

    // Scrollback
    gtk_box_append(GTK_BOX(vbox), gtk_label_new("Scrollback Lines:"));
    ctx->scrollback_spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(100, 100000, 100));
    gtk_spin_button_set_value(ctx->scrollback_spin, config->scrollback.value_or(10000));
    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(ctx->scrollback_spin));

    // Colors
    GtkWidget* color_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget* fg_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(fg_box), gtk_label_new("Foreground:"));
    ctx->fg_button = GTK_COLOR_DIALOG_BUTTON(gtk_color_dialog_button_new(gtk_color_dialog_new()));
    
    GdkRGBA initial_fg = theme_colors.foreground;
    if (config->foreground_color) {
        gdk_rgba_parse(&initial_fg, config->foreground_color->c_str());
    }
    gtk_color_dialog_button_set_rgba(ctx->fg_button, &initial_fg);
    
    gtk_box_append(GTK_BOX(fg_box), GTK_WIDGET(ctx->fg_button));
    gtk_box_append(GTK_BOX(color_box), fg_box);

    GtkWidget* bg_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(bg_box), gtk_label_new("Background:"));
    ctx->bg_button = GTK_COLOR_DIALOG_BUTTON(gtk_color_dialog_button_new(gtk_color_dialog_new()));
    
    GdkRGBA initial_bg = theme_colors.background;
    if (config->background_color) {
        gdk_rgba_parse(&initial_bg, config->background_color->c_str());
    }
    gtk_color_dialog_button_set_rgba(ctx->bg_button, &initial_bg);

    gtk_box_append(GTK_BOX(bg_box), GTK_WIDGET(ctx->bg_button));
    gtk_box_append(GTK_BOX(color_box), bg_box);
    
    gtk_box_append(GTK_BOX(vbox), color_box);

    // Buttons
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    
    GtkWidget* cancel_btn = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(cancel_btn, "clicked", G_CALLBACK(gtk_window_destroy), window);
    gtk_box_append(GTK_BOX(button_box), cancel_btn);

    GtkWidget* save_btn = gtk_button_new_with_label("Save");
    gtk_widget_add_css_class(save_btn, "suggested-action");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_clicked), ctx);
    gtk_box_append(GTK_BOX(button_box), save_btn);

    gtk_box_append(GTK_BOX(vbox), button_box);

    g_object_set_data_full(G_OBJECT(window), "gui-context", ctx, g_free);

    gtk_window_present(GTK_WINDOW(window));
}

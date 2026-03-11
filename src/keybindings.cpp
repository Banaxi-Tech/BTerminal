#include "keybindings.h"

#include <vte/vte.h>

#include "terminal.h"
#include "config_gui.h"

namespace {

struct KeybindingContext {
    GtkWidget* window;
    GtkNotebook* notebook;
    GtkWidget* search_revealer;
    GtkWidget* search_entry;
    const Config* config;
};

static GtkWidget* current_terminal(KeybindingContext* ctx) {
    return gtk_notebook_get_nth_page(
        ctx->notebook,
        gtk_notebook_get_current_page(ctx->notebook));
}

static void toggle_search_bar(KeybindingContext* ctx) {
    gboolean visible =
        gtk_revealer_get_reveal_child(GTK_REVEALER(ctx->search_revealer));
    gtk_revealer_set_reveal_child(GTK_REVEALER(ctx->search_revealer), !visible);
    if (!visible) {
        gtk_widget_grab_focus(ctx->search_entry);
    } else {
        GtkWidget* term = current_terminal(ctx);
        if (term) {
            gtk_widget_grab_focus(term);
        }
    }
}

static void search_next(KeybindingContext* ctx, gboolean backwards) {
    GtkWidget* term = current_terminal(ctx);
    if (!term) {
        return;
    }
    const char* text =
        gtk_editable_get_text(GTK_EDITABLE(ctx->search_entry));
    if (!text || *text == '\0') {
        return;
    }

    GError* error = nullptr;
    VteRegex* regex =
        vte_regex_new_for_search(text,
                                 -1,
                                 static_cast<GRegexCompileFlags>(G_REGEX_CASELESS),
                                 &error);
    if (!regex) {
        if (error) {
            g_error_free(error);
        }
        return;
    }

    vte_terminal_search_set_regex(VTE_TERMINAL(term),
                                  regex,
                                  G_REGEX_MATCH_NOTEMPTY);
    if (backwards) {
        vte_terminal_search_find_previous(VTE_TERMINAL(term));
    } else {
        vte_terminal_search_find_next(VTE_TERMINAL(term));
    }
    vte_regex_unref(regex);
}

gboolean handle_key(guint keyval, GdkModifierType mods, KeybindingContext* ctx) {
    guint base = gdk_keyval_to_lower(keyval);

    bool ctrl = (mods & GDK_CONTROL_MASK) != 0;
    bool alt = (mods & GDK_ALT_MASK) != 0;
    bool shift = (mods & GDK_SHIFT_MASK) != 0;

    if (ctrl && alt && shift && base == GDK_KEY_m) {
        show_config_gui(GTK_WINDOW(ctx->window), ctx->notebook, const_cast<Config*>(ctx->config));
        return TRUE;
    }

    if (ctrl && shift && base == GDK_KEY_c) {
        GtkWidget* term = current_terminal(ctx);
        if (term) {
            vte_terminal_copy_clipboard_format(VTE_TERMINAL(term), VTE_FORMAT_TEXT);
            return TRUE;
        }
    }
    if (ctrl && shift && base == GDK_KEY_v) {
        GtkWidget* term = current_terminal(ctx);
        if (term) {
            vte_terminal_paste_clipboard(VTE_TERMINAL(term));
            return TRUE;
        }
    }
    if (ctrl && shift && base == GDK_KEY_t) {
        create_new_terminal_tab(ctx->notebook, *ctx->config);
        return TRUE;
    }
    if (ctrl && shift && base == GDK_KEY_w) {
        int page = gtk_notebook_get_current_page(ctx->notebook);
        if (page >= 0) {
            gtk_notebook_remove_page(ctx->notebook, page);
        }
        return TRUE;
    }
    if (ctrl && !shift && (keyval == GDK_KEY_Tab)) {
        int n_pages = gtk_notebook_get_n_pages(ctx->notebook);
        if (n_pages > 0) {
            int page = gtk_notebook_get_current_page(ctx->notebook);
            gtk_notebook_set_current_page(ctx->notebook, (page + 1) % n_pages);
        }
        return TRUE;
    }
    if (ctrl && shift && (keyval == GDK_KEY_Tab)) {
        int n_pages = gtk_notebook_get_n_pages(ctx->notebook);
        if (n_pages > 0) {
            int page = gtk_notebook_get_current_page(ctx->notebook);
            gtk_notebook_set_current_page(ctx->notebook, (page - 1 + n_pages) % n_pages);
        }
        return TRUE;
    }
    if (ctrl && (keyval == GDK_KEY_plus || keyval == GDK_KEY_equal)) {
        GtkWidget* term = current_terminal(ctx);
        if (term) {
            const PangoFontDescription* current =
                vte_terminal_get_font(VTE_TERMINAL(term));
            if (current) {
                PangoFontDescription* desc = pango_font_description_copy(current);
                int size = pango_font_description_get_size(desc) / PANGO_SCALE;
                pango_font_description_set_size(desc, (size + 1) * PANGO_SCALE);
                vte_terminal_set_font(VTE_TERMINAL(term), desc);
                pango_font_description_free(desc);
            }
        }
        return TRUE;
    }
    if (ctrl && keyval == GDK_KEY_minus) {
        GtkWidget* term = current_terminal(ctx);
        if (term) {
            const PangoFontDescription* current =
                vte_terminal_get_font(VTE_TERMINAL(term));
            if (current) {
                PangoFontDescription* desc = pango_font_description_copy(current);
                int size = pango_font_description_get_size(desc) / PANGO_SCALE;
                if (size > 6) {
                    pango_font_description_set_size(desc, (size - 1) * PANGO_SCALE);
                    vte_terminal_set_font(VTE_TERMINAL(term), desc);
                }
                pango_font_description_free(desc);
            }
        }
        return TRUE;
    }
    if (ctrl && keyval == GDK_KEY_0) {
        GtkWidget* term = current_terminal(ctx);
        if (term) {
            PangoFontDescription* desc = pango_font_description_from_string("monospace 11");
            vte_terminal_set_font(VTE_TERMINAL(term), desc);
            pango_font_description_free(desc);
        }
        return TRUE;
    }
    if (keyval == GDK_KEY_F11) {
        gboolean fullscreen = gtk_window_is_fullscreen(GTK_WINDOW(ctx->window));
        if (fullscreen) {
            gtk_window_unfullscreen(GTK_WINDOW(ctx->window));
        } else {
            gtk_window_fullscreen(GTK_WINDOW(ctx->window));
        }
        return TRUE;
    }
    if (ctrl && shift && base == GDK_KEY_f) {
        toggle_search_bar(ctx);
        return TRUE;
    }
    if (gtk_widget_has_focus(ctx->search_entry) &&
        keyval == GDK_KEY_Return && (mods & GDK_SHIFT_MASK)) {
        search_next(ctx, TRUE);
        return TRUE;
    }
    if (gtk_widget_has_focus(ctx->search_entry) &&
        keyval == GDK_KEY_Return) {
        search_next(ctx, FALSE);
        return TRUE;
    }

    return FALSE;
}

gboolean key_controller_cb(GtkEventControllerKey* controller,
                           guint keyval,
                           guint keycode,
                           GdkModifierType state,
                           gpointer user_data) {
    (void)controller;
    (void)keycode;
    return handle_key(keyval, state, static_cast<KeybindingContext*>(user_data));
}

} // namespace

void setup_global_keybindings(GtkWidget* window,
                              GtkNotebook* notebook,
                              GtkWidget* search_revealer,
                              GtkWidget* search_entry,
                              const Config& config) {
    auto* ctx = g_new0(KeybindingContext, 1);
    ctx->window = window;
    ctx->notebook = notebook;
    ctx->search_revealer = search_revealer;
    ctx->search_entry = search_entry;
    ctx->config = &config;

    GtkEventController* controller =
        GTK_EVENT_CONTROLLER(gtk_event_controller_key_new());
    gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
    g_signal_connect(controller, "key-pressed", G_CALLBACK(key_controller_cb), ctx);
    gtk_widget_add_controller(window, controller);
}


#pragma once

#include <gtk/gtk.h>

#include "config.h"

void setup_global_keybindings(GtkWidget* window,
                              GtkNotebook* notebook,
                              GtkWidget* search_revealer,
                              GtkWidget* search_entry,
                              const Config& config);


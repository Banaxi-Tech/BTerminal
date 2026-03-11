#pragma once

#include <gtk/gtk.h>
#include <vte/vte.h>

#include "config.h"

void create_new_terminal_tab(GtkNotebook* notebook, const Config& config);


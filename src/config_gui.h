#pragma once

#include <gtk/gtk.h>
#include "config.h"

void show_config_gui(GtkWindow* parent, GtkNotebook* notebook, Config* config);

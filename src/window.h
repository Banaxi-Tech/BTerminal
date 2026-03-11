#pragma once

#include <gtk/gtk.h>

#include "config.h"

GtkWindow* create_main_window(GtkApplication* app, const Config& config);


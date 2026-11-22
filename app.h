#ifndef APP_H
#define APP_H

#include <gtk/gtk.h>
#include <cairo.h>

typedef enum {
    TOOL_PENCIL,
    TOOL_ERASER,
    TOOL_LINE,
    TOOL_RECT,
    TOOL_ELLIPSE,
    TOOL_TEXT
} ToolMode;

typedef struct {
    double r, g, b;
    const char *name;
} ColorPreset;

typedef struct {
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *text_popover;
    GtkWidget *text_entry;
    cairo_surface_t *surface;
    int width;
    int height;

    ToolMode tool;
    double color_r, color_g, color_b;
    double stroke_width;

    gboolean drawing;
    double start_x;
    double start_y;
    double last_x;
    double last_y;
    double curr_x;
    double curr_y;

    gboolean has_text_anchor;
    double text_x;
    double text_y;
} AppState;

/* Paleta compartilhada entre UI e desenho */
extern const ColorPreset PALETTE[4];

/* Funções do canvas (definidas em canvas.c) */
void canvas_clear(AppState *app);
void canvas_ensure_surface(AppState *app, int width, int height);
void canvas_destroy_surface(AppState *app);
gboolean canvas_is_shape_tool(ToolMode tool);
void canvas_draw_freehand_segment(AppState *app, double x0, double y0, double x1, double y1);
void canvas_draw_shape(AppState *app, cairo_t *cr, double x0, double y0, double x1, double y1);
void canvas_draw_text(AppState *app, const char *text, double x, double y);
void canvas_render_view(AppState *app, cairo_t *cr, int width, int height);
void canvas_save_png_dialog(AppState *app);

/* Funções da interface (definidas em ui.c) */
void ui_build(AppState *app, GtkApplication *gtk_app);
void ui_cleanup(AppState *app);

#endif
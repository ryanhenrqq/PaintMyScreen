#include "app.h"
#include <pango/pangocairo.h>
#include <math.h>

const ColorPreset PALETTE[4] = {
    { 0.08, 0.08, 0.08, "Preto" },
    { 0.85, 0.15, 0.15, "Vermelho" },
    { 0.12, 0.45, 0.85, "Azul" },
    { 0.12, 0.65, 0.25, "Verde" }
};

/* Pinta a superfície de desenho inteira de branco */
void canvas_clear(AppState *app) {
    if (!app->surface) return;
    cairo_t *cr = cairo_create(app->surface);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_destroy(cr);
}

/* Cria a superfície branca na primeira vez que o tamanho do canvas é conhecido */
void canvas_ensure_surface(AppState *app, int width, int height) {
    if (width <= 0 || height <= 0) return;

    if (!app->surface) {
        app->width = width;
        app->height = height;
        app->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        canvas_clear(app);
    }
}

/* Libera a memória da superfície ao encerrar o programa */
void canvas_destroy_surface(AppState *app) {
    if (app->surface) {
        cairo_surface_destroy(app->surface);
        app->surface = NULL;
    }
}

/* Retorna TRUE se a ferramenta atual for de forma geométrica (com preview) */
gboolean canvas_is_shape_tool(ToolMode tool) {
    return tool == TOOL_LINE || tool == TOOL_RECT || tool == TOOL_ELLIPSE;
}

/* Aplica cor, espessura e pontas arredondadas conforme ferramenta e estado */
static void apply_stroke_style(AppState *app, cairo_t *cr) {
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    if (app->tool == TOOL_ERASER) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_line_width(cr, app->stroke_width * 4.0);
    } else {
        cairo_set_source_rgb(cr, app->color_r, app->color_g, app->color_b);
        cairo_set_line_width(cr, app->stroke_width);
    }
}

/* Desenha um pedaço de traço livre (lápis ou borracha) direto na superfície */
void canvas_draw_freehand_segment(AppState *app, double x0, double y0, double x1, double y1) {
    if (!app->surface) return;
    cairo_t *cr = cairo_create(app->surface);
    apply_stroke_style(app, cr);
    cairo_move_to(cr, x0, y0);
    cairo_line_to(cr, x1, y1);
    cairo_stroke(cr);
    cairo_destroy(cr);
    gtk_widget_queue_draw(app->drawing_area);
}

/* Desenha linha, retângulo ou elipse no contexto Cairo passado (surface ou preview) */
void canvas_draw_shape(AppState *app, cairo_t *cr,
                       double x0, double y0, double x1, double y1) {
    apply_stroke_style(app, cr);

    if (app->tool == TOOL_LINE) {
        cairo_move_to(cr, x0, y0);
        cairo_line_to(cr, x1, y1);
        cairo_stroke(cr);
    } else if (app->tool == TOOL_RECT) {
        double rx = fmin(x0, x1);
        double ry = fmin(y0, y1);
        double rw = fabs(x1 - x0);
        double rh = fabs(y1 - y0);
        cairo_rectangle(cr, rx, ry, rw, rh);
        cairo_stroke(cr);
    } else if (app->tool == TOOL_ELLIPSE) {
        double cx = (x0 + x1) / 2.0;
        double cy = (y0 + y1) / 2.0;
        double rx = fabs(x1 - x0) / 2.0;
        double ry = fabs(y1 - y0) / 2.0;
        if (rx > 0.5 && ry > 0.5) {
            cairo_save(cr);
            cairo_translate(cr, cx, cy);
            cairo_scale(cr, rx, ry);
            cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0 * G_PI);
            cairo_restore(cr);
            cairo_stroke(cr);
        }
    }
}

/* Renderiza o texto usando Pango + Cairo na posição (x, y) da superfície */
void canvas_draw_text(AppState *app, const char *text, double x, double y) {
    if (!app->surface || !text || text[0] == '\0') return;

    cairo_t *cr = cairo_create(app->surface);
    cairo_set_source_rgb(cr, app->color_r, app->color_g, app->color_b);

    int font_size = (int)fmax(12.0, app->stroke_width * 5.0 + 10.0);
    char font_desc_str[64];
    g_snprintf(font_desc_str, sizeof(font_desc_str), "Sans %d", font_size);

    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, text, -1);

    PangoFontDescription *desc = pango_font_description_from_string(font_desc_str);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);

    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
    cairo_destroy(cr);
    gtk_widget_queue_draw(app->drawing_area);
}

/* Compõe a tela: superfície salva + preview da forma + marcador da caixa de texto */
void canvas_render_view(AppState *app, cairo_t *cr, int width, int height) {
    canvas_ensure_surface(app, width, height);

    if (app->surface) {
        cairo_set_source_surface(cr, app->surface, 0, 0);
        cairo_paint(cr);
    }

    if (app->drawing && canvas_is_shape_tool(app->tool)) {
        canvas_draw_shape(app, cr, app->start_x, app->start_y, app->curr_x, app->curr_y);
    }

    if (app->has_text_anchor) {
        const double dashes[] = { 4.0, 4.0 };
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0.2, 0.4, 0.8, 0.8);
        cairo_set_line_width(cr, 1.5);
        cairo_set_dash(cr, dashes, 2, 0.0);
        cairo_rectangle(cr, app->text_x - 2, app->text_y - 2, 80, 24);
        cairo_stroke(cr);
        cairo_restore(cr);
    }
}

/* Callback assíncrono chamado quando o usuário escolhe o arquivo no diálogo de salvar */
static void on_save_file_finished(GObject *source, GAsyncResult *res, gpointer user_data) {
    AppState *app = user_data;
    GError *error = NULL;

    GFile *file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), res, &error);
    if (!file) {
        if (error) {
            if (!g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED)) {
                g_printerr("Diálogo cancelado ou com erro: %s\n", error->message);
            }
            g_error_free(error);
        }
        return;
    }

    char *path = g_file_get_path(file);
    if (path && app->surface) {
        char *final_path = g_str_has_suffix(path, ".png")
                             ? g_strdup(path)
                             : g_strconcat(path, ".png", NULL);

        cairo_status_t status = cairo_surface_write_to_png(app->surface, final_path);
        if (status == CAIRO_STATUS_SUCCESS) {
            g_print("Imagem salva em: %s\n", final_path);
        } else {
            g_printerr("Erro ao salvar PNG: %s\n", cairo_status_to_string(status));
        }
        g_free(final_path);
    }

    g_free(path);
    g_object_unref(file);
}

/* Abre a janela nativa para escolher nome e pasta do arquivo PNG */
void canvas_save_png_dialog(AppState *app) {
    if (!app->surface) return;

    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Salvar imagem como PNG");
    gtk_file_dialog_set_initial_name(dialog, "desenho.png");

    gtk_file_dialog_save(dialog,
                         GTK_WINDOW(app->window),
                         NULL,
                         on_save_file_finished,
                         app);

    g_object_unref(dialog);
}
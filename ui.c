#include "app.h"

/* Confirma o texto digitado no popover e desenha na posição marcada */
static void commit_text_from_entry(AppState *app) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(app->text_entry));
    if (app->has_text_anchor && text && text[0] != '\0') {
        canvas_draw_text(app, text, app->text_x, app->text_y);
    }
    app->has_text_anchor = FALSE;
    gtk_editable_set_text(GTK_EDITABLE(app->text_entry), "");
    gtk_popover_popdown(GTK_POPOVER(app->text_popover));
    gtk_widget_queue_draw(app->drawing_area);
}

/* Dispara quando o usuário aperta Enter dentro da caixa de texto */
static void on_text_entry_activate(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    commit_text_from_entry(user_data);
}

/* Dispara quando o usuário clica no botão "Inserir" */
static void on_text_insert_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    commit_text_from_entry(user_data);
}

/* Posiciona e abre o popover de texto no ponto (x, y) clicado */
static void open_text_box_at(AppState *app, double x, double y) {
    app->has_text_anchor = TRUE;
    app->text_x = x;
    app->text_y = y;
    gtk_widget_queue_draw(app->drawing_area);

    GdkRectangle rect = {
        .x = (int)x,
        .y = (int)y,
        .width = 1,
        .height = 1
    };
    gtk_popover_set_pointing_to(GTK_POPOVER(app->text_popover), &rect);
    gtk_editable_set_text(GTK_EDITABLE(app->text_entry), "");
    gtk_popover_popup(GTK_POPOVER(app->text_popover));
    gtk_widget_grab_focus(app->text_entry);
}

/* Início do arraste: abre texto ou começa um traço/forma */
static void on_drag_begin(GtkGestureDrag *gesture, double start_x, double start_y, gpointer user_data) {
    (void)gesture;
    AppState *app = user_data;
    canvas_ensure_surface(app,
                          gtk_widget_get_width(app->drawing_area),
                          gtk_widget_get_height(app->drawing_area));

    if (app->tool == TOOL_TEXT) {
        open_text_box_at(app, start_x, start_y);
        return;
    }

    app->drawing = TRUE;
    app->start_x = start_x;
    app->start_y = start_y;
    app->last_x = start_x;
    app->last_y = start_y;
    app->curr_x = start_x;
    app->curr_y = start_y;

    if (!canvas_is_shape_tool(app->tool)) {
        canvas_draw_freehand_segment(app, start_x, start_y, start_x, start_y);
    }
}

/* Durante o arraste: atualiza preview de forma ou traça segmento livre */
static void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
    (void)gesture;
    AppState *app = user_data;
    if (!app->drawing || app->tool == TOOL_TEXT) return;

    double current_x = app->start_x + offset_x;
    double current_y = app->start_y + offset_y;
    app->curr_x = current_x;
    app->curr_y = current_y;

    if (canvas_is_shape_tool(app->tool)) {
        gtk_widget_queue_draw(app->drawing_area);
    } else {
        canvas_draw_freehand_segment(app, app->last_x, app->last_y, current_x, current_y);
        app->last_x = current_x;
        app->last_y = current_y;
    }
}

/* Fim do arraste: carimba a forma definitiva na superfície e encerra o gesto */
static void on_drag_end(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
    (void)gesture;
    AppState *app = user_data;
    if (!app->drawing || app->tool == TOOL_TEXT) return;

    double end_x = app->start_x + offset_x;
    double end_y = app->start_y + offset_y;

    if (canvas_is_shape_tool(app->tool) && app->surface) {
        cairo_t *cr = cairo_create(app->surface);
        canvas_draw_shape(app, cr, app->start_x, app->start_y, end_x, end_y);
        cairo_destroy(cr);
        gtk_widget_queue_draw(app->drawing_area);
    }

    app->drawing = FALSE;
}

/* Callback de desenho do GtkDrawingArea */
static void draw_cb(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    (void)area;
    canvas_render_view(user_data, cr, width, height);
}

/* Troca a ferramenta ativa conforme o botão clicado */
static void on_tool_selected(GtkButton *btn, gpointer user_data) {
    AppState *app = user_data;
    ToolMode mode = (ToolMode)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "tool_mode"));
    app->tool = mode;
}

/* Troca a cor ativa conforme o botão da paleta clicado */
static void on_color_selected(GtkButton *btn, gpointer user_data) {
    AppState *app = user_data;
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "color_idx"));
    app->color_r = PALETTE[idx].r;
    app->color_g = PALETTE[idx].g;
    app->color_b = PALETTE[idx].b;
}

/* Atualiza a espessura do traço quando o slider muda */
static void on_size_changed(GtkRange *range, gpointer user_data) {
    AppState *app = user_data;
    app->stroke_width = gtk_range_get_value(range);
}

/* Limpa todo o canvas voltando para o branco */
static void on_clear_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    AppState *app = user_data;
    canvas_clear(app);
    gtk_widget_queue_draw(app->drawing_area);
}

/* Dispara o diálogo para salvar o desenho em PNG */
static void on_save_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    canvas_save_png_dialog(user_data);
}

/* Cria um botão de ferramenta já associado ao seu ToolMode */
static GtkWidget *make_tool_button(const char *label, ToolMode mode, AppState *app) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    g_object_set_data(G_OBJECT(btn), "tool_mode", GINT_TO_POINTER(mode));
    g_signal_connect(btn, "clicked", G_CALLBACK(on_tool_selected), app);
    return btn;
}

/* Monta toda a interface gráfica e conecta os sinais/gestos */
void ui_build(AppState *app, GtkApplication *gtk_app) {
    app->tool = TOOL_PENCIL;
    app->color_r = PALETTE[0].r;
    app->color_g = PALETTE[0].g;
    app->color_b = PALETTE[0].b;
    app->stroke_width = 3.0;

    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "PaintMyScreen");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 980, 620);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(app->window), vbox);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top(toolbar, 6);
    gtk_widget_set_margin_bottom(toolbar, 6);
    gtk_widget_set_margin_start(toolbar, 6);
    gtk_widget_set_margin_end(toolbar, 6);
    gtk_box_append(GTK_BOX(vbox), toolbar);

    gtk_box_append(GTK_BOX(toolbar), make_tool_button("Lápis", TOOL_PENCIL, app));
    gtk_box_append(GTK_BOX(toolbar), make_tool_button("Borracha", TOOL_ERASER, app));
    gtk_box_append(GTK_BOX(toolbar), make_tool_button("Linha", TOOL_LINE, app));
    gtk_box_append(GTK_BOX(toolbar), make_tool_button("Retângulo", TOOL_RECT, app));
    gtk_box_append(GTK_BOX(toolbar), make_tool_button("Elipse", TOOL_ELLIPSE, app));
    gtk_box_append(GTK_BOX(toolbar), make_tool_button("Texto", TOOL_TEXT, app));

    gtk_box_append(GTK_BOX(toolbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

    for (int i = 0; i < 4; i++) {
        GtkWidget *cbtn = gtk_button_new_with_label(PALETTE[i].name);
        g_object_set_data(G_OBJECT(cbtn), "color_idx", GINT_TO_POINTER(i));
        g_signal_connect(cbtn, "clicked", G_CALLBACK(on_color_selected), app);
        gtk_box_append(GTK_BOX(toolbar), cbtn);
    }

    gtk_box_append(GTK_BOX(toolbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

    GtkWidget *size_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 20.0, 1.0);
    gtk_range_set_value(GTK_RANGE(size_scale), app->stroke_width);
    gtk_widget_set_size_request(size_scale, 110, -1);
    g_signal_connect(size_scale, "value-changed", G_CALLBACK(on_size_changed), app);
    gtk_box_append(GTK_BOX(toolbar), size_scale);

    GtkWidget *clear_btn = gtk_button_new_with_label("Limpar");
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_clicked), app);
    gtk_box_append(GTK_BOX(toolbar), clear_btn);

    GtkWidget *save_btn = gtk_button_new_with_label("Salvar PNG");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_clicked), app);
    gtk_box_append(GTK_BOX(toolbar), save_btn);

    app->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_vexpand(app->drawing_area, TRUE);
    gtk_widget_set_hexpand(app->drawing_area, TRUE);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app->drawing_area), draw_cb, app, NULL);
    gtk_box_append(GTK_BOX(vbox), app->drawing_area);

    app->text_popover = gtk_popover_new();
    gtk_widget_set_parent(app->text_popover, app->drawing_area);
    gtk_popover_set_autohide(GTK_POPOVER(app->text_popover), FALSE);

    GtkWidget *pop_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top(pop_box, 6);
    gtk_widget_set_margin_bottom(pop_box, 6);
    gtk_widget_set_margin_start(pop_box, 6);
    gtk_widget_set_margin_end(pop_box, 6);
    gtk_popover_set_child(GTK_POPOVER(app->text_popover), pop_box);

    app->text_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->text_entry), "Digite e aperte Enter...");
    g_signal_connect(app->text_entry, "activate", G_CALLBACK(on_text_entry_activate), app);
    gtk_box_append(GTK_BOX(pop_box), app->text_entry);

    GtkWidget *insert_btn = gtk_button_new_with_label("Inserir");
    g_signal_connect(insert_btn, "clicked", G_CALLBACK(on_text_insert_clicked), app);
    gtk_box_append(GTK_BOX(pop_box), insert_btn);

    GtkGesture *drag = gtk_gesture_drag_new();
    g_signal_connect(drag, "drag-begin", G_CALLBACK(on_drag_begin), app);
    g_signal_connect(drag, "drag-update", G_CALLBACK(on_drag_update), app);
    g_signal_connect(drag, "drag-end", G_CALLBACK(on_drag_end), app);
    gtk_widget_add_controller(app->drawing_area, GTK_EVENT_CONTROLLER(drag));

    gtk_window_present(GTK_WINDOW(app->window));
}

/* Desanexa widgets auxiliares antes de destruir o aplicativo */
void ui_cleanup(AppState *app) {
    if (app->text_popover) {
        gtk_widget_unparent(app->text_popover);
        app->text_popover = NULL;
    }
}
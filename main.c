#include "app.h"

/* Chamado quando a janela do aplicativo é ativada/aberta */
static void on_activate(GtkApplication *gtk_app, gpointer user_data) {
    ui_build(user_data, gtk_app);
}

/* Chamado quando o aplicativo está encerrando para liberar recursos */
static void on_shutdown(GtkApplication *gtk_app, gpointer user_data) {
    (void)gtk_app;
    AppState *app = user_data;
    ui_cleanup(app);
    canvas_destroy_surface(app);
}

/* Ponto de entrada do programa em C */
int main(int argc, char **argv) {
    AppState app = {0};

    GtkApplication *gtk_app = gtk_application_new("com.exemplo.vibepaint", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(on_activate), &app);
    g_signal_connect(gtk_app, "shutdown", G_CALLBACK(on_shutdown), &app);

    int status = g_application_run(G_APPLICATION(gtk_app), argc, argv);
    g_object_unref(gtk_app);
    return status;
}
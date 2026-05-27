#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#define SCRIPT_URL "https://raw.githubusercontent.com/Deadboy666/h3adcr-b/refs/heads/main/headcrab.sh"

typedef struct {
    GtkWidget *window;
    GtkWidget *btn;
    GtkWidget *logo_image;
    GtkWidget *log_view;
    GtkTextBuffer *log_buf;
    GtkWidget *status_label;
    GtkCssProvider *css_provider;
    int current_theme; /* 0 = red, 1 = blue */
    char icon_path_red[512];
    char icon_path_blue[512];
} AppWidgets;

static const char *CSS_RED =
    "window { background-color: #000000; }"
    "image { background-color: #000000; }"
    "#title { color: #cc2200; font-size: 24px; font-weight: bold; letter-spacing: 4px; }"
    "#subtitle { color: #444444; font-size: 10px; letter-spacing: 5px; }"
    "#run_btn { background: #0d0000; color: #cc2200; border: 2px solid #cc2200;"
    "  font-size: 15px; font-weight: bold; letter-spacing: 3px; padding: 10px 40px; border-radius: 0; }"
    "#run_btn:hover { background-color: #1a0000; color: #ff3300; }"
    "#run_btn:disabled { background-color: #0d0d0d; color: #333; border-color: #333; }"
    "#status { color: #444; font-size: 11px; letter-spacing: 2px; }"
    "#status_done { color: #228822; font-size: 11px; letter-spacing: 2px; }"
    "#status_error { color: #cc2200; font-size: 11px; letter-spacing: 2px; }"
    "#log { background-color: #cc2200; color: #cc4422; font-family: monospace; font-size: 12px; }"
    "#log text { background-color: #cc2200; }"
    "#footer { color: #222222; font-size: 10px; }";

static const char *CSS_BLUE =
    "window { background-color: #E49427; }"
    "image { background-color: #E49427; }"
    "#title { color: #1a6abf; font-size: 24px; font-weight: bold; letter-spacing: 4px; }"
    "#subtitle { color: #444444; font-size: 10px; letter-spacing: 5px; }"
    "#run_btn { background: #00060d; color: #1a6abf; border: 2px solid #1a6abf;"
    "  font-size: 15px; font-weight: bold; letter-spacing: 3px; padding: 10px 40px; border-radius: 0; }"
    "#run_btn:hover { background-color: #001a33; color: #3399ff; }"
    "#run_btn:disabled { background-color: #0d0d0d; color: #333; border-color: #333; }"
    "#status { color: #444; font-size: 11px; letter-spacing: 2px; }"
    "#status_done { color: #228822; font-size: 11px; letter-spacing: 2px; }"
    "#status_error { color: #1a6abf; font-size: 11px; letter-spacing: 2px; }"
    "#log { background-color: #1a6abf; color: #1a6abf; font-family: monospace; font-size: 12px; }"
    "#log text { background-color: #1a6abf; }"
    "#footer { color: #222222; font-size: 10px; }";

static gboolean append_log(gpointer data) {
    char **args = (char **)data;
    AppWidgets *w = (AppWidgets *)args[0];
    const char *text = args[1];
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(w->log_buf, &end);
    gtk_text_buffer_insert(w->log_buf, &end, text, -1);
    gtk_text_buffer_insert(w->log_buf, &end, "\n", -1);
    GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(gtk_widget_get_parent(w->log_view));
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(scroll);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
    free(args[1]);
    free(args);
    return G_SOURCE_REMOVE;
}

static gboolean on_done(gpointer data) {
    char **args = (char **)data;
    AppWidgets *w = (AppWidgets *)args[0];
    int code = atoi(args[1]);
    gtk_widget_set_sensitive(w->btn, TRUE);
    if (code == 0) {
        gtk_label_set_text(GTK_LABEL(w->status_label), "✓ DONE");
        gtk_widget_set_name(w->status_label, "status_done");
    } else {
        gtk_label_set_text(GTK_LABEL(w->status_label), "✗ ERROR");
        gtk_widget_set_name(w->status_label, "status_error");
    }
    free(args[1]);
    free(args);
    return G_SOURCE_REMOVE;
}

static void log_from_thread(AppWidgets *w, const char *text) {
    char **args = malloc(sizeof(char *) * 2);
    args[0] = (char *)w;
    args[1] = strdup(text);
    g_idle_add(append_log, args);
}

static void done_from_thread(AppWidgets *w, int code) {
    char **args = malloc(sizeof(char *) * 2);
    args[0] = (char *)w;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", code);
    args[1] = strdup(buf);
    g_idle_add(on_done, args);
}

static gpointer run_thread(gpointer data) {
    AppWidgets *w = (AppWidgets *)data;
    log_from_thread(w, "[ FETCHING SCRIPT FROM GITHUB... ]");
    FILE *fp = popen("curl -fsSL " SCRIPT_URL " | bash 2>&1", "r");
    if (!fp) {
        log_from_thread(w, "[ ERROR: failed to run curl ]");
        done_from_thread(w, 1);
        return NULL;
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        log_from_thread(w, line);
    }
    int ret = pclose(fp);
    done_from_thread(w, WEXITSTATUS(ret));
    return NULL;
}

static void on_update_clicked(GtkWidget *btn, gpointer data) {
    AppWidgets *w = (AppWidgets *)data;
    gtk_widget_set_sensitive(w->btn, FALSE);
    gtk_label_set_text(GTK_LABEL(w->status_label), "RUNNING...");
    gtk_widget_set_name(w->status_label, "status");
    gtk_text_buffer_set_text(w->log_buf, "", -1);
    GThread *thread = g_thread_new("updater", run_thread, w);
    g_thread_unref(thread);
}

static void apply_theme(AppWidgets *w) {
    const char *css = (w->current_theme == 0) ? CSS_RED : CSS_BLUE;
    gtk_css_provider_load_from_data(w->css_provider, css, -1, NULL);
    const char *logo_path = (w->current_theme == 0) ? w->icon_path_red : w->icon_path_blue;
    GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_scale(logo_path, 110, 110, TRUE, NULL);
    if (pb) gtk_image_set_from_pixbuf(GTK_IMAGE(w->logo_image), pb);
}

static gboolean on_logo_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppWidgets *w = (AppWidgets *)data;
    w->current_theme = (w->current_theme == 0) ? 1 : 0;
    apply_theme(w);
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppWidgets *w = g_new0(AppWidgets, 1);
    w->current_theme = 0;

    char *dir = g_path_get_dirname(argv[0]);
    snprintf(w->icon_path_red,  sizeof(w->icon_path_red),  "%s/headcrab.png", dir);
    snprintf(w->icon_path_blue, sizeof(w->icon_path_blue), "%s/headcrab_blue.png", dir);
    g_free(dir);

    /* CSS */
    w->css_provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(w->css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    gtk_css_provider_load_from_data(w->css_provider, CSS_RED, -1, NULL);

    /* Window */
    w->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(w->window), "Headcrab Updater");
    gtk_window_set_default_size(GTK_WINDOW(w->window), 500, 540);
    gtk_window_set_resizable(GTK_WINDOW(w->window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(w->window), 0);
    gtk_window_set_icon_from_file(GTK_WINDOW(w->window), w->icon_path_red, NULL);
    g_signal_connect(w->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* Main box */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 24);
    gtk_widget_set_margin_end(vbox, 24);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(w->window), vbox);

    /* Logo - clickable to toggle theme */
    GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_scale(w->icon_path_red, 110, 110, TRUE, NULL);
    w->logo_image = gtk_image_new_from_pixbuf(pb);
    gtk_widget_set_app_paintable(w->logo_image, TRUE);
    gtk_widget_add_events(w->logo_image, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(w->logo_image, "button-press-event", G_CALLBACK(on_logo_clicked), w);
    gtk_box_pack_start(GTK_BOX(vbox), w->logo_image, FALSE, FALSE, 0);

    /* Title */
    GtkWidget *title = gtk_label_new("HEADCRAB UPDATER");
    gtk_widget_set_name(title, "title");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    /* Subtitle */
    GtkWidget *subtitle = gtk_label_new("The Headcrab Approaches..");
    gtk_widget_set_name(subtitle, "subtitle");
    gtk_box_pack_start(GTK_BOX(vbox), subtitle, FALSE, FALSE, 0);

    /* Update button */
    w->btn = gtk_button_new_with_label("▶   UPDATE");
    gtk_widget_set_name(w->btn, "run_btn");
    gtk_widget_set_size_request(w->btn, 220, 50);
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(btn_box), w->btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_box, FALSE, FALSE, 6);
    g_signal_connect(w->btn, "clicked", G_CALLBACK(on_update_clicked), w);

    /* Status */
    w->status_label = gtk_label_new("READY");
    gtk_widget_set_name(w->status_label, "status");
    gtk_box_pack_start(GTK_BOX(vbox), w->status_label, FALSE, FALSE, 0);

    /* Log */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 180);
    w->log_buf = gtk_text_buffer_new(NULL);
    w->log_view = gtk_text_view_new_with_buffer(w->log_buf);
    gtk_widget_set_name(w->log_view, "log");
    gtk_text_view_set_editable(GTK_TEXT_VIEW(w->log_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(w->log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(w->log_view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), w->log_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(w->log_buf, &end);
    gtk_text_buffer_insert(w->log_buf, &end,
        "[ HEADCRAB UPDATER INITIALIZED ]\n[ PRESS UPDATE TO FETCH LATEST PATCH ]", -1);

    /* Footer */
    GtkWidget *footer = gtk_label_new("<a href=\"https://github.com/Deadboy666/h3adcr-b\"><span foreground=\"#444444\" size=\"medium\" underline=\"none\">github.com/Deadboy666/h3adcr-b</span></a>");
    gtk_label_set_use_markup(GTK_LABEL(footer), TRUE);
    gtk_label_set_track_visited_links(GTK_LABEL(footer), FALSE);
    gtk_widget_set_name(footer, "footer");
    gtk_box_pack_start(GTK_BOX(vbox), footer, FALSE, FALSE, 0);

    gtk_widget_show_all(w->window);
    gtk_main();

    g_free(w);
    return 0;
}

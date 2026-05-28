#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#define SCRIPT_URL  "https://raw.githubusercontent.com/Deadboy666/h3adcr-b/refs/heads/main/headcrab.sh"
#define RESET_URL   "curl -fsSL headcrab.pages.dev/reset | bash"
#define REPATCH_URL "curl -fsSL headcrab.pages.dev | bash"
#define CONFIG_DIR  "/.config/headcrab-updater"
#define CONFIG_FILE "/.config/headcrab-updater/theme"

typedef struct {
    GtkWidget *window;
    GtkWidget *btn;
    GtkWidget *close_btn;
    GtkWidget *logo_image;
    GtkWidget *log_view;
    GtkTextBuffer *log_buf;
    GtkWidget *status_label;
    GtkCssProvider *css_provider;
    GtkWidget *outer_frame;
    GtkWidget *footer_red;
    GtkWidget *footer_blue;
    int current_theme; /* 0 = red, 1 = blue */
    char icon_path_red[512];
    char icon_path_blue[512];
} AppWidgets;

static const char *CSS_RED =
    "window { background-color: #cc2200; }"
    "image { background-color: #000000; }"
    "#logo_box { background-color: transparent; }"
    "#outer_frame { background-color: #000000; margin: 3px; }"
    "#title { color: #cc2200; font-size: 24px; font-weight: bold; letter-spacing: 4px; }"
    "#subtitle { color: #aaaaaa; font-size: 10px; letter-spacing: 5px; }"
    "#run_btn { background: #0d0000; color: #cc2200; border: 2px solid #cc2200;"
    "  font-size: 15px; font-weight: bold; letter-spacing: 3px; padding: 10px 40px; border-radius: 0; }"
    "#run_btn:hover { background-color: #1a0000; color: #ff3300; }"
    "#run_btn:active { background-color: #330000; color: #ff3300; }"
    "#run_btn:disabled { background-color: #0d0d0d; color: #333; border-color: #333; }"
    "#trouble_btn { background: #0d0000; color: #cc2200; border: 2px solid #cc2200;"
    "  font-size: 13px; font-weight: bold; letter-spacing: 2px; padding: 8px 20px; border-radius: 0; }"
    "#trouble_btn:hover { background-color: #1a0000; color: #ff3300; }"
    "#trouble_btn:active { background-color: #330000; color: #ff3300; }"
    "#sub_btn { background: #0d0000; color: #cc2200; border: 2px solid #cc2200;"
    "  font-size: 13px; font-weight: bold; letter-spacing: 2px; padding: 8px 20px; border-radius: 0; }"
    "#sub_btn:hover { background-color: #1a0000; color: #ff3300; }"
    "#sub_btn:active { background-color: #330000; color: #ff3300; }"
    "#close_btn { background: transparent; color: #cc2200; border: none;"
    "  font-size: 18px; font-weight: bold; padding: 0 8px; min-width: 0; min-height: 0; }"
    "#close_btn:hover { color: #ff3300; }"
    "#close_btn:active { color: #880000; }"
    "#topbar { background-color: #000000; }"
    "#status { color: #cc2200; font-size: 16px; font-weight: bold; letter-spacing: 2px; }"
    "#status_done { color: #228822; font-size: 16px; font-weight: bold; letter-spacing: 2px; }"
    "#status_error { color: #ff3300; font-size: 16px; font-weight: bold; letter-spacing: 2px; }"
    "#log { background-color: #cc2200; color: #000000; font-family: monospace; font-size: 12px; border: 2px solid #ffffff; }"
    "#log text { background-color: #cc2200; }"
    "scrolledwindow { border: 2px solid #ffffff; }"
    "#note { color: #aaaaaa; font-size: 10px; font-style: italic; }"
    "#footer { color: #aaaaaa; font-size: 10px; }";

static const char *CSS_BLUE =
    "window { background-color: #1a6abf; }"
    "image { background-color: #E49427; }"
    "#logo_box { background-color: transparent; }"
    "#outer_frame { background-color: #E49427; margin: 3px; }"
    "#title { color: #1a6abf; font-size: 24px; font-weight: bold; letter-spacing: 4px; }"
    "#subtitle { color: #444444; font-size: 10px; letter-spacing: 5px; }"
    "#run_btn { background: #E49427; color: #1a6abf; border: 2px solid #1a6abf;"
    "  font-size: 15px; font-weight: bold; letter-spacing: 3px; padding: 10px 40px; border-radius: 0; }"
    "#run_btn:hover { background-color: #c07d1a; color: #003d80; }"
    "#run_btn:active { background-color: #a06010; color: #003d80; }"
    "#run_btn:disabled { background-color: #c0a060; color: #888; border-color: #888; }"
    "#trouble_btn { background: #E49427; color: #1a6abf; border: 2px solid #1a6abf;"
    "  font-size: 13px; font-weight: bold; letter-spacing: 2px; padding: 8px 20px; border-radius: 0; }"
    "#trouble_btn:hover { background-color: #c07d1a; color: #003d80; }"
    "#trouble_btn:active { background-color: #a06010; color: #003d80; }"
    "#sub_btn { background: #E49427; color: #1a6abf; border: 2px solid #1a6abf;"
    "  font-size: 13px; font-weight: bold; letter-spacing: 2px; padding: 8px 20px; border-radius: 0; }"
    "#sub_btn:hover { background-color: #c07d1a; color: #003d80; }"
    "#sub_btn:active { background-color: #a06010; color: #003d80; }"
    "#close_btn { background: transparent; color: #1a6abf; border: none;"
    "  font-size: 18px; font-weight: bold; padding: 0 8px; min-width: 0; min-height: 0; }"
    "#close_btn:hover { color: #3399ff; }"
    "#close_btn:active { color: #0d3d80; }"
    "#topbar { background-color: #E49427; }"
    "#status { color: #1a6abf; font-size: 16px; font-weight: bold; letter-spacing: 2px; }"
    "#status_done { color: #228822; font-size: 16px; font-weight: bold; letter-spacing: 2px; }"
    "#status_error { color: #cc0000; font-size: 16px; font-weight: bold; letter-spacing: 2px; }"
    "#log { background-color: #1a6abf; color: #ffffff; font-family: monospace; font-size: 12px; border: 2px solid #ffffff; }"
    "#log text { background-color: #1a6abf; }"
    "scrolledwindow { border: 2px solid #ffffff; }"
    "#note { color: #444444; font-size: 10px; font-style: italic; }"
    "#footer { color: #444444; font-size: 10px; }";

static void save_theme(int theme) {
    const char *home = g_get_home_dir();
    char dir_path[512], file_path[512];
    snprintf(dir_path,  sizeof(dir_path),  "%s%s", home, CONFIG_DIR);
    snprintf(file_path, sizeof(file_path), "%s%s", home, CONFIG_FILE);
    mkdir(dir_path, 0755);
    FILE *f = fopen(file_path, "w");
    if (f) { fprintf(f, "%d", theme); fclose(f); }
}

static int load_theme() {
    const char *home = g_get_home_dir();
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s%s", home, CONFIG_FILE);
    FILE *f = fopen(file_path, "r");
    if (!f) return 0;
    int theme = 0;
    fscanf(f, "%d", &theme);
    fclose(f);
    return (theme == 1) ? 1 : 0;
}

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
    if (w->footer_red && w->footer_blue) {
        if (w->current_theme == 0) {
            gtk_widget_show(w->footer_red);
            gtk_widget_hide(w->footer_blue);
        } else {
            gtk_widget_hide(w->footer_red);
            gtk_widget_show(w->footer_blue);
        }
    }
}

static gboolean on_logo_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppWidgets *w = (AppWidgets *)data;
    static int click_count = 0;
    static time_t first_click_time = 0;

    time_t now = time(NULL);

    if (click_count > 0 && difftime(now, first_click_time) > 3.0) {
        click_count = 0;
        first_click_time = 0;
    }

    if (click_count == 0)
        first_click_time = now;

    click_count++;

    if (click_count >= 5) {
        click_count = 0;
        first_click_time = 0;
        w->current_theme = (w->current_theme == 0) ? 1 : 0;
        apply_theme(w);
        save_theme(w->current_theme);
    }
    return FALSE;
}

static gboolean on_topbar_drag(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    AppWidgets *w = (AppWidgets *)data;
    if (event->button == 1)
        gtk_window_begin_move_drag(GTK_WINDOW(w->window),
            event->button, event->x_root, event->y_root, event->time);
    return FALSE;
}

static void run_cmd(const char *cmd) {
    char full[512];
    snprintf(full, sizeof(full), "%s &", cmd);
    system(full);
}

static void open_troubleshoot(GtkWidget *btn, gpointer data) {
    AppWidgets *w = (AppWidgets *)data;

    GtkWidget *twin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(twin), "SLS Troubleshooting");
    gtk_window_set_default_size(GTK_WINDOW(twin), 380, 330);
    gtk_window_set_resizable(GTK_WINDOW(twin), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(twin), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(twin), GTK_WINDOW(w->window));
    gtk_window_set_position(GTK_WINDOW(twin), GTK_WIN_POS_CENTER);

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(outer, "outer_frame");
    gtk_container_add(GTK_CONTAINER(twin), outer);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(outer), vbox);

    GtkWidget *topbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(topbar, "topbar");
    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(topbar), spacer, TRUE, TRUE, 0);
    GtkWidget *close = gtk_button_new_with_label("✕");
    gtk_widget_set_name(close, "close_btn");
    g_signal_connect_swapped(close, "clicked", G_CALLBACK(gtk_widget_destroy), twin);
    gtk_box_pack_end(GTK_BOX(topbar), close, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), topbar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(content, 40);
    gtk_widget_set_margin_end(content, 40);
    gtk_widget_set_margin_top(content, 16);
    gtk_widget_set_margin_bottom(content, 30);
    gtk_box_pack_start(GTK_BOX(vbox), content, TRUE, TRUE, 0);

    GtkWidget *title = gtk_label_new("SLS TROUBLESHOOTING");
    gtk_widget_set_name(title, "title");
    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);

    GtkWidget *note = gtk_label_new("Do these steps in order from top to bottom.");
    gtk_widget_set_name(note, "note");
    gtk_box_pack_start(GTK_BOX(content), note, FALSE, FALSE, 0);

    GtkWidget *btn_reset = gtk_button_new_with_label("Reset Steam");
    gtk_widget_set_name(btn_reset, "sub_btn");
    gtk_widget_set_size_request(btn_reset, 260, 44);
    GtkWidget *row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(row1, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(row1), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), row1, FALSE, FALSE, 0);
    g_signal_connect_swapped(btn_reset, "clicked", G_CALLBACK(run_cmd), RESET_URL);

    GtkWidget *btn_steam = gtk_button_new_with_label("Open Steam");
    gtk_widget_set_name(btn_steam, "sub_btn");
    gtk_widget_set_size_request(btn_steam, 260, 44);
    GtkWidget *row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(row2, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(row2), btn_steam, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), row2, FALSE, FALSE, 0);
    g_signal_connect_swapped(btn_steam, "clicked", G_CALLBACK(run_cmd), "steam");

    GtkWidget *btn_repatch = gtk_button_new_with_label("Repatch Steam");
    gtk_widget_set_name(btn_repatch, "sub_btn");
    gtk_widget_set_size_request(btn_repatch, 260, 44);
    GtkWidget *row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(row3, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(row3), btn_repatch, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), row3, FALSE, FALSE, 0);
    g_signal_connect_swapped(btn_repatch, "clicked", G_CALLBACK(run_cmd), REPATCH_URL);

    gtk_widget_show_all(twin);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppWidgets *w = g_new0(AppWidgets, 1);
    w->current_theme = load_theme();

    char *dir = g_path_get_dirname(argv[0]);
    snprintf(w->icon_path_red,  sizeof(w->icon_path_red),  "%s/headcrab.png", dir);
    snprintf(w->icon_path_blue, sizeof(w->icon_path_blue), "%s/headcrab_blue.png", dir);
    g_free(dir);

    w->css_provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(w->css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    gtk_css_provider_load_from_data(w->css_provider,
        (w->current_theme == 0) ? CSS_RED : CSS_BLUE, -1, NULL);

    w->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(w->window), "Headcrab Updater");
    gtk_window_set_default_size(GTK_WINDOW(w->window), 500, 560);
    gtk_window_set_resizable(GTK_WINDOW(w->window), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(w->window), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(w->window), 0);
    gtk_window_set_icon_from_file(GTK_WINDOW(w->window), w->icon_path_red, NULL);
    g_signal_connect(w->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    w->outer_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(w->outer_frame, "outer_frame");
    gtk_container_add(GTK_CONTAINER(w->window), w->outer_frame);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(w->outer_frame), vbox);

    GtkWidget *topbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(topbar, "topbar");
    gtk_widget_add_events(topbar, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(topbar, "button-press-event", G_CALLBACK(on_topbar_drag), w);
    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(topbar), spacer, TRUE, TRUE, 0);
    w->close_btn = gtk_button_new_with_label("✕");
    gtk_widget_set_name(w->close_btn, "close_btn");
    g_signal_connect(w->close_btn, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_end(GTK_BOX(topbar), w->close_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), topbar, FALSE, FALSE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(content, 24);
    gtk_widget_set_margin_end(content, 24);
    gtk_widget_set_margin_top(content, 4);
    gtk_box_pack_start(GTK_BOX(vbox), content, TRUE, TRUE, 0);

    const char *initial_logo = (w->current_theme == 0) ? w->icon_path_red : w->icon_path_blue;
    GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_scale(initial_logo, 110, 110, TRUE, NULL);
    w->logo_image = gtk_image_new_from_pixbuf(pb);
    gtk_widget_set_app_paintable(w->logo_image, TRUE);
    GtkWidget *event_box = gtk_event_box_new();
    gtk_widget_set_name(event_box, "logo_box");
    gtk_container_add(GTK_CONTAINER(event_box), w->logo_image);
    gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_logo_clicked), w);
    GtkWidget *logo_center = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(logo_center, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(logo_center), event_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), logo_center, FALSE, FALSE, 0);

    GtkWidget *title = gtk_label_new("HEADCRAB UPDATER");
    gtk_widget_set_name(title, "title");
    gtk_box_pack_start(GTK_BOX(content), title, FALSE, FALSE, 0);

    GtkWidget *subtitle = gtk_label_new("The Headcrab Approaches..");
    gtk_widget_set_name(subtitle, "subtitle");
    gtk_box_pack_start(GTK_BOX(content), subtitle, FALSE, FALSE, 0);

    w->btn = gtk_button_new_with_label("▶   UPDATE");
    gtk_widget_set_name(w->btn, "run_btn");
    gtk_widget_set_size_request(w->btn, 220, 50);
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(btn_box), w->btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), btn_box, FALSE, FALSE, 6);
    g_signal_connect(w->btn, "clicked", G_CALLBACK(on_update_clicked), w);

    GtkWidget *trouble_btn = gtk_button_new_with_label("TROUBLESHOOT SLS");
    gtk_widget_set_name(trouble_btn, "trouble_btn");
    gtk_widget_set_size_request(trouble_btn, 220, 40);
    GtkWidget *trouble_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(trouble_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(trouble_box), trouble_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), trouble_box, FALSE, FALSE, 0);
    g_signal_connect(trouble_btn, "clicked", G_CALLBACK(open_troubleshoot), w);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 160);
    w->log_buf = gtk_text_buffer_new(NULL);
    w->log_view = gtk_text_view_new_with_buffer(w->log_buf);
    gtk_widget_set_name(w->log_view, "log");
    gtk_text_view_set_editable(GTK_TEXT_VIEW(w->log_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(w->log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(w->log_view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), w->log_view);
    gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(w->log_buf, &end);
    gtk_text_buffer_insert(w->log_buf, &end,
        "[ HEADCRAB UPDATER INITIALIZED ]\n[ PRESS UPDATE TO FETCH LATEST PATCH ]", -1);

    w->status_label = gtk_label_new("READY");
    gtk_widget_set_name(w->status_label, "status");
    gtk_label_set_xalign(GTK_LABEL(w->status_label), 0.5);
    gtk_box_pack_start(GTK_BOX(content), w->status_label, FALSE, FALSE, 4);

    GtkWidget *filler = gtk_label_new("");
    gtk_widget_set_vexpand(filler, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), filler, TRUE, TRUE, 0);

    GtkWidget *footer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(footer_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_bottom(footer_box, 10);

    w->footer_red = gtk_label_new(
        "<a href='https://github.com/Deadboy666/h3adcr-b'>"
        "<span foreground='#aaaaaa' size='medium' underline='none'>h3adcr-b</span></a>"
        " ❖ "
        "<a href='https://github.com/Ke619/UI-CRAB'>"
        "<span foreground='#aaaaaa' size='medium' underline='none'>UI-CRAB</span></a>");
    gtk_label_set_use_markup(GTK_LABEL(w->footer_red), TRUE);
    gtk_label_set_track_visited_links(GTK_LABEL(w->footer_red), FALSE);
    gtk_widget_set_name(w->footer_red, "footer");
    gtk_box_pack_start(GTK_BOX(footer_box), w->footer_red, FALSE, FALSE, 0);

    w->footer_blue = gtk_label_new(
        "<a href='https://github.com/Deadboy666/h3adcr-b'>"
        "<span foreground='#444444' size='medium' underline='none'>h3adcr-b</span></a>"
        " ❖ "
        "<a href='https://github.com/Ke619/UI-CRAB'>"
        "<span foreground='#444444' size='medium' underline='none'>UI-CRAB</span></a>");
    gtk_label_set_use_markup(GTK_LABEL(w->footer_blue), TRUE);
    gtk_label_set_track_visited_links(GTK_LABEL(w->footer_blue), FALSE);
    gtk_widget_set_name(w->footer_blue, "footer");
    gtk_box_pack_start(GTK_BOX(footer_box), w->footer_blue, FALSE, FALSE, 0);

    /* Hide the correct footer based on saved theme */
    if (w->current_theme == 1) {
        gtk_widget_set_no_show_all(w->footer_red, TRUE);
    } else {
        gtk_widget_set_no_show_all(w->footer_blue, TRUE);
    }

    gtk_box_pack_start(GTK_BOX(vbox), footer_box, FALSE, FALSE, 0);

        gtk_widget_show_all(w->window);
    gtk_main();

    g_free(w);
    return 0;
}

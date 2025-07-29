#include "ui.h"
#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>

static GtkWidget *window;
static GtkWidget *label_key_event;
static GtkWidget *label_capture_block;
static GtkWidget *label_defense_alert;

static int key_event_count = 0;
static int capture_block_count = 0;
static char defense_alert_msg[256] = "";

static pthread_t ui_thread;

static void update_labels() {
    char buf[256];

    snprintf(buf, sizeof(buf), "암호화 키 이벤트 수: %d", key_event_count);
    gtk_label_set_text(GTK_LABEL(label_key_event), buf);

    snprintf(buf, sizeof(buf), "캡처 차단 횟수: %d", capture_block_count);
    gtk_label_set_text(GTK_LABEL(label_capture_block), buf);

    gtk_label_set_text(GTK_LABEL(label_defense_alert), defense_alert_msg);
}

static gboolean ui_timer_callback(gpointer data) {
    update_labels();
    return TRUE; // 계속 호출
}

static void *ui_main(void *arg) {
    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "키보드 보안 프로그램 상태");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 150);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    label_key_event = gtk_label_new("암호화 키 이벤트 수: 0");
    label_capture_block = gtk_label_new("캡처 차단 횟수: 0");
    label_defense_alert = gtk_label_new("");

    gtk_box_pack_start(GTK_BOX(vbox), label_key_event, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), label_capture_block, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), label_defense_alert, FALSE, FALSE, 5);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_timeout_add(500, ui_timer_callback, NULL);

    gtk_widget_show_all(window);

    gtk_main();
    return NULL;
}

int start_ui() {
    return pthread_create(&ui_thread, NULL, ui_main, NULL);
}

// 외부에서 UI에 수치 업데이트 함수
void update_key_event_count(int count) {
    key_event_count = count;
}

void update_capture_block_count(int count) {
    capture_block_count = count;
}

void update_defense_alert(const char *msg) {
    strncpy(defense_alert_msg, msg, sizeof(defense_alert_msg)-1);
    defense_alert_msg[sizeof(defense_alert_msg)-1] = '\0';
}

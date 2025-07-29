#include "capture_guard.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_t monitor_thread;

void* capture_monitor_loop(void *arg) {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "X11 디스플레이 열기 실패\n");
        return NULL;
    }

    Window root = DefaultRootWindow(display);
    int prtsc_keycode = XKeysymToKeycode(display, XK_Print);

    // PrintScreen 키를 grab
    XGrabKey(display, prtsc_keycode, AnyModifier, root, True,
             GrabModeAsync, GrabModeAsync);
    XSelectInput(display, root, KeyPressMask);

    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);
        if (ev.type == KeyPress && ev.xkey.keycode == prtsc_keycode) {
            printf("[캡처 차단] PrintScreen 키 감지됨\n");
        }
        usleep(50000);
    }

    XUngrabKey(display, prtsc_keycode, AnyModifier, root);
    XCloseDisplay(display);
    return NULL;
}

int init_capture_guard() {
    if (pthread_create(&monitor_thread, NULL, capture_monitor_loop, NULL) != 0) {
        fprintf(stderr, "캡처 모니터 스레드 생성 실패\n");
        return -1;
    }
    return 0;
}

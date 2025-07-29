#define _GNU_SOURCE
#include "self_defense.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAPS_PATH "/proc/self/maps"
#define CHECK_INTERVAL_SEC 5

// 허용된 라이브러리 이름 목록 (간단 예시)
static const char *allowed_libs[] = {
    "libc-", "libpthread-", "libssl", "libcrypto", "libgtk", "libX11",
    NULL
};

static pthread_t defense_thread;

static int is_library_allowed(const char *line) {
    for (int i = 0; allowed_libs[i] != NULL; i++) {
        if (strstr(line, allowed_libs[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

static void* defense_loop(void *arg) {
    while (1) {
        FILE *fp = fopen(MAPS_PATH, "r");
        if (!fp) {
            perror("maps 파일 열기 실패");
            sleep(CHECK_INTERVAL_SEC);
            continue;
        }

        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            // 라이브러리 경로가 있을 경우만 검사
            char *path = strchr(line, '/');
            if (path) {
                if (!is_library_allowed(path)) {
                    fprintf(stderr, "[Self-Defense] 의심 라이브러리 감지: %s", path);
                    // 필요시 프로세스 종료 가능:
                    // exit(1);
                }
            }
        }
        fclose(fp);
        sleep(CHECK_INTERVAL_SEC);
    }
    return NULL;
}

int init_self_defense() {
    if (pthread_create(&defense_thread, NULL, defense_loop, NULL) != 0) {
        fprintf(stderr, "Self-Defense 스레드 생성 실패\n");
        return -1;
    }
    return 0;
}

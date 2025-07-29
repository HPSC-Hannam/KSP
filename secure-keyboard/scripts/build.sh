#!/bin/bash

gcc -Wall -g -Iinclude -o build/secure-keyboard \
    src/main.c src/input_monitor.c src/encryptor.c src/capture_guard.c \
    src/self_defense.c src/logger.c src/ui.c \
    `pkg-config --cflags --libs gtk+-3.0` -lssl -lcrypto -lX11 -lpthread

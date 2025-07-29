#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <linux/input.h>

int encrypt_event(struct input_event *ev);

#endif

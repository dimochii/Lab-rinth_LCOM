#ifndef _LCOM_KEYBOARD_H_
#define _LCOM_KEYBOARD_H_

#include <minix/sysutil.h>
#include <lcom/lcf.h>
#include "i8042.h"

int (keyboard_subscribe_int)(uint8_t *bit_no);

int (keyboard_unsubscribe_int)();

int (read_scancode)(uint8_t port, uint8_t* code);

void (kbc_ih)();

#endif

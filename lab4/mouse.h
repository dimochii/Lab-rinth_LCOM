#ifndef _LCOM_MOUSE_H_
#define _LCOM_MOUSE_H_

#include <minix/sysutil.h>
#include <lcom/lcf.h>
#include "i8042.h"

int (write_kbc)(uint8_t port, uint8_t command);
int (write_mouse) (uint8_t command);

int (mouse_subscribe_int)(uint8_t* irq_set);
int (mouse_unsubscribe_int)();

void (mouse_ih)();
int (read_kbc)(uint8_t* byte, uint8_t status_aux);

void (store_byte_mouse)();
void (store_bytes_packet)();

#endif

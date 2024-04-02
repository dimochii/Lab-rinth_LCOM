#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"


int hookId = 0;
int counter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (freq > TIMER_FREQ || freq < 19) {
     return 1;
  }
  uint8_t control = 0x00;
  if (timer_get_conf(timer, &control) != 0) {
    return 1;
  } 

  control = (control & 0x0F) | TIMER_LSB_MSB;  // control = ((control & 0x0F ) | BIT(4) | BIT(5)); 

  uint32_t INIT_VAL = TIMER_FREQ / freq;
  uint8_t MSB, LSB;
  util_get_MSB(INIT_VAL, &MSB);
  util_get_LSB(INIT_VAL, &LSB);

  // printf("The initial value is %d\n", INIT_VAL);

  uint8_t controlWords[] = {TIMER_SEL0, TIMER_SEL1, TIMER_SEL2};
     
  switch (timer) {  
    case 0: 
      control |= controlWords[0];
      break;
    case 1:
      control |= controlWords[1];
      break;
    case 2:
      control |= controlWords[2];
      break;
    default:
      return 1;
  }

  if (sys_outb(TIMER_CTRL, control) != 0) return 1;
  if (sys_outb(TIMER_0 + timer, LSB) != 0) return 1;
  if (sys_outb(TIMER_0 +timer, MSB) != 0) return 1;

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  if (!bit_no) {
    return 1;
  } 
  *bit_no = BIT(hookId);
  if (sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, &hookId) != 0) {
    return 1;
  }

  return 0;
}

int (timer_unsubscribe_int)() {
  if (sys_irqrmpolicy( &hookId) != 0) {
    return 1;
  }

  return 0;
}

void (timer_int_handler)() {
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (st == NULL || timer < 0 || timer > 2) {
    return 1;
  }

  uint8_t rbcom = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)); // rbcm = (BIT(7) | BIT(6)) | BIT((timer)+1)

  if (sys_outb(TIMER_CTRL, rbcom) != 0){
    return 1;
  }
 
  int port = TIMER_0 + timer;

  if (util_sys_inb(port,st) != 0){
    return 1;
  } 
  return 0; 
}


int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union  timer_status_field_val controlWord;

  switch (field) {
    case tsf_all :
      controlWord.byte = st;
      break;
    case tsf_initial :
      st = (st >> 4) & 0x03;
      if (st == 1) 
        controlWord.in_mode = LSB_only;
      else if (st == 2) 
        controlWord.in_mode = MSB_only;
      else if (st == 3) 
        controlWord.in_mode = MSB_after_LSB;
      break;
    case tsf_mode :
      st = (st >> 1) & 0x07;
      if(st == 6) 
        controlWord.count_mode = 2;
      else if(st == 7) 
        controlWord.count_mode = 3;
      else controlWord.count_mode = st;
      break;
    case tsf_base:
      controlWord.bcd = st & TIMER_BCD;
      break;
    default:
      return  1;
  }

  if (timer_print_config(timer, field, controlWord) != 0) return 1;
  return 0;
}



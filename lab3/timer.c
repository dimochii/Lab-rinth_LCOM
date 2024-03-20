#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int hook_id = 0;

int counter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  uint8_t msb, lsb;
  if ( timer < 0 || timer > 2 || freq > TIMER_FREQ ) {
      return 1;
  }

  uint8_t controlWord;
  if (timer_get_conf(timer, &controlWord) != 0) {
    return 1;
  } 

  controlWord = (controlWord & 0x0F) | TIMER_LSB_MSB; 

  uint32_t INIT_VALUE = TIMER_FREQ / freq;

  util_get_MSB(INIT_VALUE, &msb);
  util_get_LSB(INIT_VALUE, &lsb);

  uint8_t sTimer;      
  uint8_t controlWords[] = {TIMER_SEL0, TIMER_SEL1, TIMER_SEL2};
  uint8_t sTimers[] = {TIMER_0, TIMER_1, TIMER_2};

  switch (timer) {  
    case 0: 
      controlWord |= controlWords[0];
      sTimer = sTimers[0];
      break;
    case 1:
      controlWord |= controlWords[1];
      sTimer = sTimers[1];
      break;
    case 2:
      controlWord |= controlWords[2];
      sTimer = sTimers[2];
      break;
    default:
      return 1;
  }
  
  if (sTimer == 0 || sys_outb(TIMER_CTRL, controlWord) != 0)  {
      return 1;
  }

  if (sys_outb(sTimer, msb) != 0 || sys_outb(sTimer, lsb) != 0) {
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    if(bit_no == NULL){return 1;}
    *bit_no = BIT(hook_id);

    if (sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, & hook_id) != 0) { 
        return 1; 
    }
    return 0; 

}

int (timer_unsubscribe_int)() {

if(sys_irqrmpolicy(& hook_id)!= 0){return 1;}

  return 0;
}

void (timer_int_handler)() {
counter ++;

}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (st == NULL || timer < 0 || timer > 2) {
    return 1;
  }

  uint8_t rbcom = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)); 

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
  union  	timer_status_field_val controlWord;

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
    case tsf_base:
      controlWord.bcd = st & TIMER_BCD;
      break;
    default:
      return  1;
  }

  if (timer_print_config(timer, field, controlWord) != 0) return 1;
  return 0;
}


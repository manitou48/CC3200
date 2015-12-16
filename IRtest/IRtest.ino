// cc3200 demo of IR remote xmit recv
//  version 2 use analogWrite() to set clock,timer,pin
//   Sony remote tests
// can run both xmit/recv at once
// transmit with IR LED, recvr GP1UX311QS
// ref https://github.com/z3t0/Arduino-IRremote
#include <Energia.h>
#include <driverlib/timer.h>
#include <driverlib/prcm.h>

extern "C" void PWMWrite(uint8_t pin, uint32_t analog_res, uint32_t duty, unsigned int freq);

#include "IR_remote.h"

#define RECVPIN 7
#define PWMPIN 9
static int pwmhz;

#define FREQ 20000
#define PERIOD (F_CPU/FREQ)

int rawbuf[RAWBUF], rawlen;
uint8_t rcvstate;
int results_decode_type; // NEC, SONY, RC5, UNKNOWN
unsigned long results_value;
int results_bits; // Number of bits in decoded value



void pwm_init(int khz) {
  pwmhz = 1000*khz;
}

void enable_pwm() {
  PWMWrite(PWMPIN, 256, 128, pwmhz);  
}

void disable_pwm() {
  analogWrite(PWMPIN,0);  // off
}


void tick_init() {
        MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralReset(PRCM_TIMERA1);
	MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC_UP);
	MAP_TimerIntRegister(TIMERA1_BASE, TIMER_B, myisr);
	MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMB_TIMEOUT);
	MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_B, (PERIOD>>16) & 0xff);  // 8 more bits
	MAP_TimerLoadSet(TIMERA1_BASE, TIMER_B, PERIOD & 0xffff);
	MAP_TimerEnable(TIMERA1_BASE, TIMER_B);
}


unsigned long myticks;

void myisr() {
	uint8_t irdata = (uint8_t)digitalRead(RECVPIN);
	myticks++;
	MAP_TimerIntClear(TIMERA1_BASE, TIMER_B);  // clear interrupt
  if (rawlen >= RAWBUF) {
    // Buffer overflow
    rcvstate = STATE_STOP;
  }
  switch(rcvstate) {
  case STATE_IDLE: // In the middle of a gap
    if (irdata == MARK) {
      if (myticks < GAP_TICKS) {
        // Not big enough to be a gap.
        myticks = 0;
      }
      else {
        // gap just ended, record duration and start recording transmission
        rawlen = 0;
        rawbuf[rawlen++] = myticks;
        myticks = 0;
        rcvstate = STATE_MARK;
      }
     }
    break;
  case STATE_MARK: // timing MARK
    if (irdata == SPACE) {   // MARK ended, record time
      rawbuf[rawlen++] = myticks;
      myticks = 0;
      rcvstate = STATE_SPACE;
    }
    break;
  case STATE_SPACE: // timing SPACE
    if (irdata == MARK) { // SPACE just ended, record it
      rawbuf[rawlen++] = myticks;
      myticks = 0;
      rcvstate = STATE_MARK;
    }
    else { // SPACE
      if (myticks > GAP_TICKS) {
        // big SPACE, indicates gap between codes
        // Mark current code as ready for processing
        // Switch to STOP
        // Don't reset timer; keep counting space width
        rcvstate = STATE_STOP;
      }
    }
    break;
  case STATE_STOP: // waiting, measuring gap
    if (irdata == MARK) { // reset gap timer
      myticks = 0;
    }
    break;
  }

}

void enableIROut(int khz) {
	pwm_init(khz);
}

void enableIRIn() {
	tick_init();
	rcvstate = STATE_IDLE;
	rawlen = 0;
	pinMode(RECVPIN, INPUT);
}

void irrecv_resume() {
    rcvstate = STATE_IDLE;
    rawlen = 0;
}


void mark(int time) {
	//TIMER_ENABLE_PWM;
	enable_pwm();
	if (time > 0) delayMicroseconds(time);
}

void space(int time){
	//TIMER_DISABLE_PWM;
	disable_pwm();
	if (time > 0) delayMicroseconds(time);
}

long decodeSony() {
  long data = 0;
  if (rawlen < 2 * SONY_BITS + 2) {
    return IRERR;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(rawbuf[offset], SONY_HDR_MARK)) {
    return IRERR;
  }
  offset++;

  while (offset + 1 < rawlen) {
    if (!MATCH_SPACE(rawbuf[offset], SONY_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(rawbuf[offset], SONY_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(rawbuf[offset], SONY_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return IRERR;
    }
    offset++;
  }

  // Success
  results_bits = (offset - 1) / 2;
  if (results_bits < 12) {
    results_bits = 0;
    return IRERR;
  }
  results_value = data;
  results_decode_type = SONY;
  return DECODED;
}

void sendSony(unsigned long data, int nbits) {
  enableIROut(40);
  mark(SONY_HDR_MARK);
  space(SONY_HDR_SPACE);
  data = data << (32 - nbits);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SONY_ONE_MARK);
      space(SONY_HDR_SPACE);
    }
    else {
      mark(SONY_ZERO_MARK);
      space(SONY_HDR_SPACE);
    }
    data <<= 1;
  }
}

void setup() {
	pinMode(GREEN_LED,OUTPUT);
	Serial.begin(9600);
	enableIRIn();
}

void loop() {
  long sonycmd[] = {0xA9A,0x91A,0x61A}; // power 0 7
  long cnt;

  Serial.println(" xmit");
  digitalWrite(GREEN_LED,HIGH);
  sendSony(sonycmd[0],SONY_BITS);
  digitalWrite(GREEN_LED,LOW);
  delay(6);   // let gap time grow

  if (rcvstate == STATE_STOP) {
    if (decodeSony() ) {
        char str[128];
        sprintf(str,"sony decoded. value %0x  %d bits",results_value, results_bits);
        Serial.println(str);
    }
    Serial.print("rawlen "); Serial.println(rawlen);
    for (int i=0; i < rawlen; i++) {
        if (i%2) Serial.print(" ");
        Serial.println(rawbuf[i]*USECPERTICK);
        }
    irrecv_resume();
  }

  delay(2000);
}


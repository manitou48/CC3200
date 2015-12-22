// timed PWM  timer trigger DMA to PWM duty reg ping-pong
//  can only update a 16-bit or 32-bit duty, not 24bit
// default Hz for analogWrite is 490hz
// PWMpin 10 TIMERA3B

#include <Energia.h>
#include <driverlib/timer.h>
#include <driverlib/prcm.h>
#include <inc/hw_timer.h>
#include <driverlib/udma.h>
#include "udma_if.h"

extern "C" void PWMWrite(uint8_t pin, uint32_t analog_res, uint32_t duty, unsigned int freq);

#define MINHZ 1333
#define PWMHZ 2000
#define PWM_PERIOD (F_CPU/PWMHZ)
#define PWMPIN 10

#define DUTYREG (TIMERA3_BASE + TIMER_O_TBMATCHR)

#define DMACHNL UDMA_CH3_TIMERA1_B

#define FREQ 1000
#define PERIOD (F_CPU/FREQ)

// ping pong DMA buffers  primary/alternate
#define DUTYSA 3
#define DUTYSB 4
uint16_t dutysA[DUTYSA] = {PWM_PERIOD/4, PWM_PERIOD/2, 3*PWM_PERIOD/4};   
uint16_t dutysB[DUTYSB] = {PWM_PERIOD/10, PWM_PERIOD/3, 2*PWM_PERIOD/3, 9*PWM_PERIOD/10};

volatile uint32_t ticks;
void timerISR(){
  // restart ping pong
  uint32_t mode;
  
  ticks++;
  MAP_TimerIntClear(TIMERA1_BASE, TIMER_B);
  
  mode = MAP_uDMAChannelModeGet(DMACHNL | UDMA_PRI_SELECT);
  if (mode == UDMA_MODE_STOP) {
    SetupTransfer(DMACHNL | UDMA_PRI_SELECT,UDMA_MODE_PINGPONG,DUTYSA,
               UDMA_SIZE_16,UDMA_ARB_1,
               dutysA,UDMA_SRC_INC_16,
               (void *)(DUTYREG),UDMA_DST_INC_NONE);
  }
  
  mode = MAP_uDMAChannelModeGet(DMACHNL | UDMA_ALT_SELECT);
  if (mode == UDMA_MODE_STOP) {
    SetupTransfer(DMACHNL | UDMA_ALT_SELECT,UDMA_MODE_PINGPONG,DUTYSB,
               UDMA_SIZE_16,UDMA_ARB_1,
               dutysB,UDMA_SRC_INC_16,
               (void *)(DUTYREG),UDMA_DST_INC_NONE);
  }
  
}

void setup()
{
  
  Serial.begin(9600);
  PWMWrite(PWMPIN, 256, 128, PWMHZ);  // start PWM
	 
    UDMAInit();  // init DMA
    SetupTransfer(DMACHNL | UDMA_PRI_SELECT,UDMA_MODE_PINGPONG,DUTYSA,
               UDMA_SIZE_16,UDMA_ARB_1,
               dutysA,UDMA_SRC_INC_16,
               (void *)(DUTYREG),UDMA_DST_INC_NONE);
    SetupTransfer(DMACHNL | UDMA_ALT_SELECT,UDMA_MODE_PINGPONG,DUTYSB,
               UDMA_SIZE_16,UDMA_ARB_1,
               dutysB,UDMA_SRC_INC_16,
               (void *)(DUTYREG),UDMA_DST_INC_NONE);
     // init timer
     MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
     MAP_PRCMPeripheralReset(PRCM_TIMERA1);
     MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC_UP);
     MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_B, (PERIOD>>16) & 0xff);  // 8 more bits
     MAP_TimerLoadSet(TIMERA1_BASE, TIMER_B, PERIOD & 0xffff);
     MAP_TimerIntRegister(TIMERA1_BASE, TIMER_B, timerISR);
     MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMB_TIMEOUT);
     MAP_TimerDMAEventSet(TIMERA1_BASE,TIMER_DMA_TIMEOUT_B);
     MAP_TimerEnable(TIMERA1_BASE, TIMER_B);  // go  Disable to stop
}

void loop()
{
  Serial.println(ticks);
  delay(3000);
}

// timed ADC  not on a pin, PWM pin 9 TIMERA3A, timer trigger DMA from ADC to buf
// UP counter, if period > 16-bit, need prescale bits (makes it 24-bit)
//   1khz, UP counter hex(80,000) 0x1 3880
// ADC bits 31, 0, 1 reserved, 17-bit timer, 12 bit ADC val

#include <Energia.h>
#include <driverlib/timer.h>
#include <driverlib/prcm.h>
#include <inc/hw_timer.h>
#include <driverlib/udma.h>
#include "udma_if.h"
#include <inc/hw_adc.h>

#define DMACHNL UDMA_CH3_TIMERA1_B
#define ADCval (ADC_BASE + ADC_O_channel0FIFODATA + ADC_CH_1)

#define FREQ 1000
#define PERIOD (F_CPU/FREQ)

#define SAMPLES 100
uint32_t samples[SAMPLES]; // 32 bits for timer and ADC val

void setup()
{
  
  Serial.begin(9600);
  analogRead(A1);   // set up pin, clock etc.
  Serial.println(analogRead(A1));
  ADCChannelEnable(ADC_BASE, ADC_CH_1);
  ADCEnable(ADC_BASE);
//  ADCTimerEnable(ADC_BASE);  // need mask, if enabled
  // init clock
        MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralReset(PRCM_TIMERA1);
	MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC_UP);
	MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_B, (PERIOD>>16) & 0xff);  // 8 more bits
	MAP_TimerLoadSet(TIMERA1_BASE, TIMER_B, PERIOD & 0xffff);
        MAP_TimerDMAEventSet(TIMERA1_BASE,TIMER_DMA_TIMEOUT_B);
	 
    UDMAInit();  // init DMA
}

void loop()
{
  uint32_t t;
  
  memset(samples,0,sizeof(samples));
  t=micros();
  SetupTransfer(DMACHNL,UDMA_MODE_BASIC,SAMPLES,
               UDMA_SIZE_32,UDMA_ARB_1,
               (void *)(ADCval),UDMA_SRC_INC_NONE,
               samples,UDMA_DST_INC_32);
  MAP_TimerEnable(TIMERA1_BASE, TIMER_B);  // go
  while(MAP_uDMAChannelModeGet(DMACHNL) != UDMA_MODE_STOP);
  t= micros()-t;
  MAP_TimerDisable(TIMERA1_BASE, TIMER_B);
  Serial.print(t); Serial.print("us  ");
  Serial.println(samples[3]>>2);
  delay(3000);

}

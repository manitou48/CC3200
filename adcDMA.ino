// adcDMA
// use channel 1 A1 pin 2, trigger on ADC sample ready in FIFO 
//  read 32bits in case timer enabled, bits 0 1 31 reserved ?
#include <Energia.h>
#include <driverlib/udma.h>
#include "udma_if.h"
#include <inc/hw_adc.h>

#define ADCval (ADC_BASE + ADC_O_channel0FIFODATA + ADC_CH_1)
#define DMACHNL UDMA_CH15_ADC_CH1

#define SAMPLES 1000
uint32_t samples[SAMPLES];

void ADCinit(){
  analogRead(A1);  // setup pin clock etc.
  Serial.println(analogRead(A1));
  ADCDMAEnable(ADC_BASE, ADC_CH_1);
  ADCChannelEnable(ADC_BASE, ADC_CH_1);
  ADCEnable(ADC_BASE);
  ADCTimerEnable(ADC_BASE);
  
  // DMA
  UDMAInit();

}

void setup()
{
  Serial.begin(9600);
  ADCinit();
  
}

void loop()
{
  uint32_t t, d,d1;
  
    memset(samples,55,sizeof(samples));
    t=micros();
    SetupTransfer(DMACHNL,UDMA_MODE_BASIC,SAMPLES,
               UDMA_SIZE_32,UDMA_ARB_1,
               (void *)(ADCval),UDMA_SRC_INC_NONE,
               samples,UDMA_DST_INC_32);
     while(MAP_uDMAChannelModeGet(DMACHNL) != UDMA_MODE_STOP);
     t= micros()-t;
     d = samples[5]>>14 ;
     d1 = (samples[6]>>14) - d ;
     Serial.print(t); Serial.print(" us  ");
     Serial.print((samples[5]>>2) & 0xfff); Serial.print(" ");
     Serial.print(d); Serial.print(" ");
     Serial.println(d1);
     delay(3000);
}

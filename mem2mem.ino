// mem2mem  DMA 
#include <Energia.h>
#include <driverlib/udma.h>
#include "udma_if.h"

#define DMACHNL UDMA_CH22_SW
#define NBYTES 1024
uint8_t src[NBYTES] __attribute__((aligned(0x10)));
uint8_t dst[NBYTES] __attribute__((aligned(0x10)));

volatile unsigned char iDone;
volatile int dmadone;
void dmaisr() {
  dmadone=1;
 // UDMAStopTransfer(DMACHNL);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);delay(5000);
  Serial.println((long)&src[0],HEX);
  Serial.println((long)&dst[0],HEX);
  for(int i = 0; i< NBYTES; i++) src[i]=i;
  UDMAInit();
  UDMAChannelSelect(DMACHNL,NULL);
}

void loop()
{
  volatile uint8_t * p = &dst[NBYTES-3];
  unsigned long t;
  
  memset(dst,0,NBYTES);
  Serial.println("go"); delay(2);
    iDone=0;
    t = micros();
 // UDMASetupAutoMemTransfer(DMACHNL,src,dst,NBYTES);
#if 1
  SetupTransfer(DMACHNL,UDMA_MODE_AUTO,NBYTES/4,
                UDMA_SIZE_32,UDMA_ARB_8,
                src,UDMA_SRC_INC_32,
                dst,UDMA_DST_INC_32);
#endif

  UDMAStartTransfer(DMACHNL);
  //while(!dmadone);
  //while(! *p);
  while(!iDone);
  t= micros()-t;
  Serial.print(t); Serial.print(" us  ");
  Serial.println(dst[3]);
  Serial.println(iDone);
  delay(3000);
}

// spi dma
// https://github.com/severin-kacianka/cc3200_dma_spi_example
#include <Energia.h>
#include <driverlib/udma.h>
#include <driverlib/spi.h>
#include <driverlib/prcm.h>
#include "udma_if.h"
#include "inc/hw_mcspi.h"

#define NBYTES 1024
#define SPIhz		(20000000 / 1)
volatile int spidone;

void SPIisr() {
  	uint32_t status = MAP_SPIIntStatus(GSPI_BASE,true);
	MAP_SPIIntClear(GSPI_BASE,SPI_INT_EOW);
	MAP_SPIIntClear(GSPI_BASE,status);
        spidone = 1;
}

void SPIinit() {
        MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

	MAP_PinTypeSPI(PIN_05, PIN_MODE_7);  // SCLK pin 7
	MAP_PinTypeSPI(PIN_06, PIN_MODE_7);  // MISO pin 14
	MAP_PinTypeSPI(PIN_07, PIN_MODE_7);  // MOSI pin 15
	MAP_PinTypeSPI(PIN_08, PIN_MODE_7);  //  CS  pin 18

  	MAP_SPIReset(GSPI_BASE);

	UDMAInit();

        MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPIhz,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_32));	
	MAP_SPIIntRegister(GSPI_BASE,SPIisr);

	MAP_SPIWordCountSet(GSPI_BASE, NBYTES/4);
	MAP_SPIFIFOLevelSet(GSPI_BASE, 1, 1);
	MAP_SPIFIFOEnable(GSPI_BASE, SPI_RX_FIFO);
        MAP_SPIFIFOEnable(GSPI_BASE, SPI_TX_FIFO); 
	MAP_SPIDmaEnable(GSPI_BASE,SPI_RX_DMA);
	MAP_SPIDmaEnable(GSPI_BASE,SPI_TX_DMA);
	MAP_SPIIntEnable(GSPI_BASE, SPI_INT_EOW);
	MAP_SPIEnable(GSPI_BASE);
}

void spi_transfer(uint8_t* tx, uint8_t* rx)
{
	spidone=0;
	SetupTransfer(UDMA_CH30_GSPI_RX,UDMA_MODE_BASIC,NBYTES/4,
                UDMA_SIZE_32,UDMA_ARB_1,
                (void *)(GSPI_BASE + MCSPI_O_RX0),UDMA_SRC_INC_NONE,
                rx,UDMA_DST_INC_32);

	SetupTransfer(UDMA_CH31_GSPI_TX,UDMA_MODE_BASIC,NBYTES/4,
                UDMA_SIZE_32,UDMA_ARB_1,
                tx,UDMA_SRC_INC_32,
		(void *)(GSPI_BASE + MCSPI_O_TX0),UDMA_DST_INC_NONE);	

	SPICSEnable(GSPI_BASE);
	while(!spidone);
	SPICSDisable(GSPI_BASE);
}

static uint8_t rxbuf[NBYTES] __attribute__((aligned(0x10)));
static uint8_t txbuf[NBYTES] __attribute__((aligned(0x10)));

void setup()
{
  Serial.begin(9600);
  Serial.println();Serial.print(__TIME__);Serial.print(" ");Serial.println(__DATE__);
  for(int i=0;i<NBYTES;i++) txbuf[i]=i;
  SPIinit();
}

void loop()
{
  uint32_t t;
  
  memset(rxbuf,0,NBYTES);
  t=micros();
  spi_transfer(txbuf,rxbuf);
  t=micros()-t;
  Serial.print(t); Serial.print(" us  ");
  Serial.print(8.*NBYTES/t,2); Serial.println(" mbs");
  Serial.println(rxbuf[3],HEX);   // jumper MOSI MISO
  delay(3000);
  
}

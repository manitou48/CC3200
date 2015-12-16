// rng.pde
//  use slow clock and SYSTICK
// based on arduino/teensy Entropy lib
// https://sites.google.com/site/astudyofentropy/project-definition/timer-jitter-entropy-sources/entropy-library

#include <Energia.h>
#include <driverlib/prcm.h>
#include <driverlib/systick.h>

#define DTICKS 200

 const uint8_t gWDT_buffer_SIZE=32;
 const uint8_t WDT_POOL_SIZE=8;
 uint8_t gWDT_buffer[gWDT_buffer_SIZE];
 uint8_t gWDT_buffer_position;
 uint8_t gWDT_loop_counter;
 volatile uint8_t gWDT_pool_start;
 volatile uint8_t gWDT_pool_end;
 volatile uint8_t gWDT_pool_count;
 volatile uint32_t gWDT_entropy_pool[WDT_POOL_SIZE];



uint32_t random() {
  uint32_t retVal;
  uint8_t waiting;
  while (gWDT_pool_count < 1) waiting += 1;
  noInterrupts();  // crtical section
    retVal = gWDT_entropy_pool[gWDT_pool_start];
    gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
    --gWDT_pool_count;
  interrupts();
  return(retVal);
}

static void tick() {
  long long t = PRCMSlowClkCtrGet();
  PRCMSlowClkCtrMatchSet(t + DTICKS); // next interrupt time
  PRCMIntStatus(); // clear
  
  gWDT_buffer[gWDT_buffer_position] = SysTickValueGet();
  gWDT_buffer_position++;                     // every time the WDT interrupt is triggered
  if (gWDT_buffer_position >= gWDT_buffer_SIZE)
  {
    gWDT_pool_end = (gWDT_pool_start + gWDT_pool_count) % WDT_POOL_SIZE;
    // The following code is an implementation of Jenkin's one at a time hash
    // This hash function has had preliminary testing to verify that it
    // produces reasonably uniform random results when using WDT jitter
    // on a variety of Arduino platforms
    for(gWDT_loop_counter = 0; gWDT_loop_counter < gWDT_buffer_SIZE; ++gWDT_loop_counter)
      {
    gWDT_entropy_pool[gWDT_pool_end] += gWDT_buffer[gWDT_loop_counter];
    gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 10);
    gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 6);
      }
    gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 3);
    gWDT_entropy_pool[gWDT_pool_end] ^= (gWDT_entropy_pool[gWDT_pool_end] >> 11);
    gWDT_entropy_pool[gWDT_pool_end] += (gWDT_entropy_pool[gWDT_pool_end] << 15);
    gWDT_entropy_pool[gWDT_pool_end] = gWDT_entropy_pool[gWDT_pool_end];
    gWDT_buffer_position = 0; // Start collecting the next 32 bytes of Timer 1 counts
    if (gWDT_pool_count == WDT_POOL_SIZE) // The entropy pool is full
      gWDT_pool_start = (gWDT_pool_start + 1) % WDT_POOL_SIZE;
    else // Add another unsigned long (32 bits) to the entropy pool
      ++gWDT_pool_count;
  }

}

void setup() {
  Serial.begin(9600);
  long long t = PRCMSlowClkCtrGet();
  PRCMSlowClkCtrMatchSet(t + DTICKS); // next interrupt time
  PRCMIntRegister(tick);
  PRCMIntEnable(PRCM_INT_SLOW_CLK_CTR);
  gWDT_buffer_position=0;
  gWDT_pool_start = 0;
  gWDT_pool_end = 0;
  gWDT_pool_count = 0;
}

#define REPS 50
void display() {
	uint32_t r,t;
	int i;
	float bps;

	t=micros();
	for (i=0;i<REPS;i++)r=random();
	t= micros() -t;
	bps = REPS*32.e6/t;
	Serial.println(bps,2);

	r=random();
	Serial.println(r,HEX);
}

void logger() {
    // await start byte from host then start sending random numbers
    //  ./logger 5000000  for diehard tests
    unsigned int rng;

    while(!Serial.available()){}   // wait for byte
    Serial.read();
    while(1) {
        rng = random();
        Serial.write((uint8_t *)&rng,4);
    }
}

void loop() {
	//display();
	 logger();
}

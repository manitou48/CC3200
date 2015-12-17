// slow clock testing
#include <Energia.h>
#include <driverlib/prcm.h>

#define DTICKS 200

volatile unsigned int tick;

void myisr() {
  PRCMIntStatus(); // clear
  long long t = PRCMSlowClkCtrGet();
  PRCMSlowClkCtrMatchSet(t + DTICKS); // next interrupt time
  tick++;
  
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  long long t = PRCMSlowClkCtrGet();
  PRCMSlowClkCtrMatchSet(t + DTICKS); // next interrupt time
  PRCMIntRegister(myisr);
  PRCMIntEnable(PRCM_INT_SLOW_CLK_CTR);
}

void loop()
{
  static unsigned int prev = PRCMSlowClkCtrGet();
  static unsigned int mprev = millis();
  delay(4000);
  unsigned int t = PRCMSlowClkCtrGet();
  unsigned int m = millis();
  Serial.print(t); Serial.print(" ");
  Serial.print((t-prev)/32768.,3); Serial.print(" s  ");
  Serial.print(m-mprev); Serial.print(" ms   ");
  Serial.println(tick);
  prev = t;
  mprev=m;
}

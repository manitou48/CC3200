// slow clock testing
#include <Energia.h>
#include <driverlib/prcm.h>

#define DTICKS 200

volatile unsigned int tick;

void myisr() {
  long long t = PRCMSlowClkCtrGet();
  PRCMSlowClkCtrMatchSet(t + DTICKS); // next interrupt time
  tick++;
  PRCMIntStatus(); // clear
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
  // put your main code here, to run repeatedly:
  delay(4000);
  unsigned int t = PRCMSlowClkCtrGet();
  Serial.print(t); Serial.print(" ");
  Serial.println(tick);
}

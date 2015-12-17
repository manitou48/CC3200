// test sha md5 hardware
// data 64-byte aligned ?
#include <Energia.h>
#include <driverlib/shamd5.h>
#include <driverlib/prcm.h>

uint8_t buf[1024],hash[16];

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);  delay(5000);
  Serial.println(SHAMD5_BASE,HEX);
  MAP_PRCMPeripheralClkEnable(PRCM_DTHE, PRCM_RUN_MODE_CLK);
  MAP_SHAMD5ConfigSet(SHAMD5_BASE,SHAMD5_ALGO_MD5);
  Serial.println("configed");
  Serial.println(MAP_SHAMD5IntStatus(SHAMD5_BASE,0),HEX);
}

void loop()
{
  // put your main code here, to run repeatedly:
  unsigned int t;
  char *str ="hello";
  
  Serial.println("hashing");
  t=micros();
  MAP_SHAMD5DataProcess(SHAMD5_BASE,buf,sizeof(buf),hash);
  t=micros()-t;
  Serial.print(t); Serial.print(" us  ");
  Serial.print(8.*1024/t,1); Serial.print(" mbs  ");
  Serial.println(1000*1024/t);
  MAP_SHAMD5DataProcess(SHAMD5_BASE,(uint8_t *)str,5,hash);
  Serial.print(hash[0],HEX); Serial.print(" ");Serial.println(hash[15],HEX);
  delay(3000);
  
}

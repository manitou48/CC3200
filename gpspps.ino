
// gpspps
//  5v,grnd,  pps to pin 5  
//     serial interface NOT used
#include <Energia.h>
#include <driverlib/prcm.h>

volatile unsigned long us, sc;
volatile int tick=0;

void handler() {
	us = micros();
        sc = PRCMSlowClkCtrGet();
	tick=1;
}

void setup() {
	Serial.begin(9600);
	pinMode(5,INPUT);   // teensy 3 needs this
	attachInterrupt(5,handler,RISING);
}

void loop() {
    static unsigned long prev = 0, sc0=0, us0=0;
    unsigned long t;
    float ppm,secs, ts;
    
    char str[32];
	if (tick)  {
            if (sc0==0) { sc0 = sc; us0 = us;}
            t= us-prev;
         //   sprintf(str,"%ld us  %ld ppm",t,t-1000000);
	  //  Serial.println(str);
            Serial.println(t-1000000);
            ts = (sc-sc0)/32768.;
            secs = 1.e-6*(us-us0);
            ppm = 1.e6*(ts-secs)/secs;
            Serial.println(ppm,1);
	    tick=0;
            prev=us;
	}
}

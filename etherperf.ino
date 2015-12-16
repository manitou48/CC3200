

// etherperf
#include <WiFi.h>
//#include <WiFiUdp.h>

//////////////////////
// WiFi Definitions //
//////////////////////
 char WiFiSSID[] = "FIXME";
 char WiFiPSK[] = "FIXME";

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = GREEN_LED; // Thing's onboard, green LED
#define REPS 10

IPAddress server_ip(192,168,1,4); // manitou

#define NBYTES 100000
#define RECLTH 1000
#define TTCP_PORT 5001

static uint8_t buf[RECLTH];

WiFiServer server(TTCP_PORT);
WiFiClient client;
WiFiUDP Udp;

unsigned long t1;

void setup() 
{
  initHardware();
  connectWiFi();
  Udp.begin(8888); // local port
}

void loop() 
{
 // tcpsend();
  //tcprecv();
  //udpsend();
  udpecho();
  //udpechosrv();
  //uvdelay(10,8);
  //udprecv();
  delay(7000);
}

void tcpsend() {
  long i,bytes=0,n,sndlth;
  float mbs;
  char str[64];

  if (!client.connect(server_ip, TTCP_PORT)) {
    Serial.println("connect failed");
    return;
  }
  t1 = millis();
  while(bytes < NBYTES) {
    sndlth = NBYTES-bytes;
    if (sndlth > RECLTH) sndlth = RECLTH;
    n = client.write((const unsigned char *)buf,sndlth); 
  //  client.write(buf[0]); n=1;        //hack
    bytes += n;
  }
  client.stop();
  t1 = millis() - t1;
  mbs = 8*NBYTES*.001/t1;
  sprintf(str,"send  %ld bytes %ld ms mbs: ",bytes,t1);
  Serial.print(str);
  Serial.println(mbs,3);
}


void tcprecv() {
  long n,bytes=0;;
  char str[64];
  WiFiClient sender;
  float mbs;

  Serial.println("server listening");
  server.begin();
  while (! ( sender = server.available()) ) {}   // await connect

  t1 = millis();
  while(sender.connected()) {
    if ((n=sender.available()) > 0) {
     if (n > RECLTH)  n = RECLTH;
      sender.read(buf,n);
      bytes += n;
     }
  }
  t1 = millis() - t1;
  mbs = 8*NBYTES*.001/t1;
  sprintf(str,"recv  %ld bytes %ld ms  %d  mbs: ",bytes,t1,n);
  Serial.print(str);
  Serial.println(mbs,3);
  sender.flush();
  sender.stop();
}

void udpsend() {
    unsigned int t1,t2,tw,i,n;
    t1=micros();
    for(i=0;i<REPS;i++){
        Udp.beginPacket(server_ip, 2000);   // to udpsink
        Udp.write(buf,1000);   // ? max of 512 bytes unless hack UdpContext.h
        Udp.endPacket();
  //      tw = micros();  // yield micro delay
     //   while(micros()-tw < 10000) yield();
        delay(1);  // without delay only 5 pkts make it out
     }
    t2= micros()-t1;
    Serial.println(t2);
}

void udpecho() {
    unsigned int t1,t2,i,n;
    static int lost=0;   // first packet lost
    
    t1=micros();
    Udp.beginPacket(server_ip, 7654);   // to uechosrv
    Udp.write(buf,8);
    Udp.endPacket();
    while (!Udp.parsePacket()) {  // wait to see if a reply is available
      if (micros() - t1 > 1000000) {
            lost++;
            Serial.print("lost "); Serial.println(lost);
            return;
        }
    }
    Udp.read(buf,8);
    t2= micros()-t1;
    Serial.println(t2);
}

void uvdelay(int cnt, int lth) {
  unsigned int t1,t2,i,n,tmin=99999,tmax=0,times[cnt];
  float avrg=0;

    Udp.beginPacket(server_ip, 2000);   // prime it
    Udp.write(buf,8);
    Udp.endPacket();
    delay(200);
    
  for (i=0;i<cnt;i++) {
    t1=micros();
    Udp.beginPacket(server_ip, 7654);   // to uechosrv
    Udp.write(buf,lth);
    Udp.endPacket();
    while((n = Udp.parsePacket()) == 0) ;
    Udp.read(buf,n);
    t2= micros()-t1;
    times[i]=t2;
  }
  for (i=0;i<cnt;i++) {
    int v = times[i];
    if (v < tmin) tmin = v;
    if (v > tmax) tmax = v;
    avrg += v;
    Serial.print(v); Serial.print(" ");
  }
  Serial.println();
  avrg /=cnt;
  Serial.print(tmin); Serial.print(" ");
  Serial.print(tmax); Serial.print(" ");
  Serial.println(avrg,2);
}

void udprecv() {
  int pkts,bytes;
  unsigned int t1,t2;
  char buff[128];
  
  // wireless router will buffer pkts from wired box
  while(Serial.available()) Serial.read();  // consume
  Serial.print(WiFi.localIP());
  Serial.println(" listening on 8888, hit key to stop");
  pkts = bytes = 0;
  while(!Serial.available()) {
    int n = Udp.parsePacket();
    if (n){
	  if (pkts == 0) t1 = micros();
	  t2=micros();
      bytes += Udp.read(buf,1000);  
      pkts++;
    }
  }
  t1 = t2-t1;
  sprintf(buff,"%ld pkts %ld bytes %d us  ",pkts,bytes,t1); Serial.print(buff);
  Serial.println(8.*bytes/t1,3);    // mbs
    while(Serial.available()) Serial.read();  // consume
}

void udpechosrv() {
  int n;

  Serial.println("uechosrv on 8888");
  while(1){ 
    while((n = Udp.parsePacket()) == 0) ;
    Udp.read(buf,n);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());   // to uechosrv
    Udp.write(buf,n);
    Udp.endPacket();
  }
}

void connectWiFi()
{
  byte ledStatus = LOW;
  Serial.println();
  Serial.println("Connecting to: " + String(WiFiSSID));


  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.print("WiFi connected "); 
   while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  } 
  Serial.println(WiFi.localIP());
    // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void initHardware()
{
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}


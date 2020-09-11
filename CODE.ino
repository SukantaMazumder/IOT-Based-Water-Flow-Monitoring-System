#define flowsensor D2 
#define Calibration 4.5 
#define VPIN_TOTAL_LITERS       V1
#define VPIN_FLOW_RATE          V2
#define VPIN_FLOW_MILLI_LITERS  V3
#define VPIN_RESET              V4

char auth[] = ""; //from blynk app
char ssid[] = ""; //your wifi ssid
char pass[] = ""; //your wifi password

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;

volatile long pulseCount = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
float totalLitres;
float totalLitresold;
unsigned long oldTime;

BLYNK_CONNECTED() {

  Blynk.syncVirtual(VPIN_TOTAL_LITERS);

}

BLYNK_WRITE(VPIN_TOTAL_LITERS)
{
  totalLitresold = param.asFloat();

}

BLYNK_WRITE(VPIN_RESET) {  
  int resetdata = param.asInt();
  if (resetdata == 0) {
    Serial.println("Clearing Data");
    Blynk.virtualWrite(VPIN_TOTAL_LITERS, 0);
    Blynk.virtualWrite(VPIN_FLOW_RATE, 0);
    flowRate = 0;
    flowMilliLitres = 0;
    totalMilliLitres = 0;
    totalLitres = 0;
    totalLitresold = 0;
  }
}

void pulseCounter()
{
  pulseCount++;
}

void flow()
{

  if ((millis() - oldTime) > 1000)  
  {
    detachInterrupt(flowsensor);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / Calibration;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    totalLitres = totalLitresold + totalMilliLitres * 0.001;
    if (flowRate>=20)
    {
      Blynk.notify("Overflow Detected");
    }
    
    unsigned int frac;

    //caudal de litros por minuto
    Serial.print("flowrate: ");
    Serial.print(int(flowRate));  

    Serial.print(".");             
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      
    Serial.print("L/min");

    Serial.print("  Current Liquid Flowing: "); 
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");

    Serial.print("  Output Liquid Quantity: "); 
    Serial.print(totalLitres);
    Serial.println("L");

    pulseCount = 0;  

    attachInterrupt(flowsensor, pulseCounter, FALLING); 
  }

}

void sendtoBlynk()  // enviando valores al servidor blynk
{
  Blynk.virtualWrite(VPIN_TOTAL_LITERS, totalLitres);          // Consumo total de agua en litros (L)
  Blynk.virtualWrite(VPIN_FLOW_RATE, flowRate);            // Muestra el caudal para este segundo en litros / minuto (L / min)
  //  Blynk.virtualWrite(VPIN_FLOW_RATE, flowMilliLitres);  // Muestra el número de litros fluidos en segundo (ml / seg)
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  totalLitresold = 0;

  pinMode(flowsensor, INPUT);

  attachInterrupt(flowsensor, pulseCounter, FALLING);

  timer.setInterval(2000L, sendtoBlynk); // envía valores al servidor blynk cada 10 segundos

}


void loop()
{

  Blynk.run();
  timer.run();
  flow();

}

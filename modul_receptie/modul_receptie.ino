#include <SPI.h>   
#include "RF24.h" 
#include <Arduino_FreeRTOS.h>

byte addresses1[][6] = {"AdrTX"};
byte addresses2[][6] = {"AdrRx"};

typedef struct {  
  float temperature;
  float humidity;
} PACKET;
PACKET data;

void TaskRecData(void *pvParameters);

TaskHandle_t Task_Handle1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  xTaskCreate(TaskRecData,"Task1",512,NULL,2,&Task_Handle1);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void TaskRecData(void *pvParameters){
  (void) pvParameters;
  RF24 radio (7, 8);
  radio.begin();
  radio.setChannel(100);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setCRCLength(RF24_CRC_8);
  radio.openReadingPipe(1, addresses1[0]);
  radio.startListening();
  
  for (;;) {
    if (radio.available())
    {
      radio.read( &data, sizeof(data));
      Serial.print("Temperatura: ");
      Serial.println(data.temperature);
      Serial.print("Umiditate: ");
      Serial.println(data.humidity);
    }
  }
}

#include <SPI.h>   
#include "RF24.h" 
#include <Arduino_FreeRTOS.h>

RF24 radio (7, 8);

byte addresses1[][6] = {"AdrTX"};
byte addresses2[][6] = {"AdrRx"};



typedef struct {  
  boolean TElow;
  boolean TEhigh;
  boolean HUlow;
  boolean HUhigh;
} PACKET;
PACKET data_levels;

void TaskRecData(void *pvParameters);
void TaskTemp(void *pvParameters);
void TaskHum(void *pvParameters);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;
TaskHandle_t Task_Handle3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
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
  
  xTaskCreate(TaskRecData,"Task1",512,NULL,2,&Task_Handle1);
  xTaskCreate(TaskTemp,"Task1",256,NULL,2,&Task_Handle2);
  xTaskCreate(TaskHum,"Task1",256,NULL,2,&Task_Handle3);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void TaskRecData(void *pvParameters){
  (void) pvParameters;
  
  for (;;) {
    if (radio.available())
    {
      radio.read( &data_levels, sizeof(data_levels));
      Serial.println(data_levels.TElow);
      Serial.println(data_levels.TEhigh);   
  }
}

void TaskTemp(void *pvParameters){
  (void) pvParameters;
  
}

void TaskHum(void *pvParameters){
  (void) pvParameters;
  
}

#include <SPI.h>   
#include "RF24.h" 
#include <Arduino_FreeRTOS.h>
#include "semphr.h"

RF24 radio (7, 8);

byte addresses1[][6] = {"AdrTX"};
byte addresses2[][6] = {"AdrRx"};

typedef struct {  
  boolean tempL = 0;
  boolean humL = 0;
} STRUCT;
STRUCT limits;

SemaphoreHandle_t xBinarySemaphore;

void TaskRecData(void *pvParameters);
void TaskCheck(void *pvParameters);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;

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
  
  xBinarySemaphore = xSemaphoreCreateBinary();
  xTaskCreate(TaskRecData,"Task1",512,NULL,0,&Task_Handle1);
  xTaskCreate(TaskCheck,"Task2",256,NULL,0,&Task_Handle2);
  xSemaphoreGive(xBinarySemaphore);

}

void loop() {
  // put your main code here, to run repeatedly:

}

void TaskRecData(void *pvParameters){
  (void) pvParameters;
  
  for (;;) {
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    if (radio.available())
    {
      radio.read( &limits, sizeof(limits));
      Serial.println(limits.tempL);
      Serial.println(limits.humL);   
    }
    xSemaphoreGive(xBinarySemaphore);
  }
}

void TaskCheck(void *pvParameters){
  (void) pvParameters;
  
  for (;;) {
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    if(limits.tempL=1){
      Serial.println("turning the cooler on for 10 sec");
      delay(10000);
      Serial.println("Turning the cooler off");
    }
    xSemaphoreGive(xBinarySemaphore);
  }
}

#include <SPI.h>   
#include "RF24.h" 
#include <Arduino_FreeRTOS.h>
#include "semphr.h"

RF24 radio (7, 8);
int led_temp = 2;
int led_hum = 3;
byte addresses1[][6] = {"AdrTX"};
byte addresses2[][6] = {"AdrRx"};

typedef struct {  
  boolean tempL ;
  boolean humL ;
} STRUCT;
STRUCT limits;

SemaphoreHandle_t xBinarySemaphore;

void TaskRecData(void *pvParameters);
void TaskCheckTemp(void *pvParameters);
void TaskCheckHum(void *pvParameters);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;
TaskHandle_t Task_Handle3;

void setup() {
  // put your setup code here, to run once:
  pinMode(led_temp,OUTPUT);
  pinMode(led_hum,OUTPUT);
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(100);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  /*
  radio.setAutoAck(1);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setCRCLength(RF24_CRC_8);
  */
  radio.openReadingPipe(1, addresses1[0]);
  radio.startListening();
  
  xBinarySemaphore = xSemaphoreCreateBinary();
  xTaskCreate(TaskRecData,"Task1",256,NULL,0,&Task_Handle1);
  xTaskCreate(TaskCheckTemp,"Task2",256,NULL,0,&Task_Handle2);
  xTaskCreate(TaskCheckHum,"Task3",256,NULL,0,&Task_Handle3);
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
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void TaskCheckTemp(void *pvParameters){
  (void) pvParameters;
  
  for (;;) {
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    if(limits.tempL == true){
      Serial.println("Pornire sistem de racire");
      digitalWrite(led_temp, HIGH);
    }else{
      Serial.println("Oprire sistem de racire");
      digitalWrite(led_temp, LOW);
    }
    xSemaphoreGive(xBinarySemaphore);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void TaskCheckHum(void *pvParameters){
  (void) pvParameters;
  
  for (;;) {
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    if(limits.humL == true){
      Serial.println("Pornire dezumidificator ");
      digitalWrite(led_hum, HIGH);
    }else{
      Serial.println("Oprire dezumidificator ");
      digitalWrite(led_hum, LOW);
    }
    xSemaphoreGive(xBinarySemaphore);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

#include <SPI.h>   
#include "RF24.h" 
#include <DHT.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"

#define DHTPIN 4       
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
RF24 radio (7, 8);

byte addresses1[][6] = {"AdrTX"};
byte addresses2[][6] = {"AdrRx"};

typedef struct {  
  float temperature;
  float humidity;
} PACKET;
PACKET data_rec;

typedef struct {  
  boolean tempL = 0;
  boolean humL = 0;
} STRUCT;
STRUCT limits;

SemaphoreHandle_t xBinarySemaphore;

void TaskReadData(void *pvParameters);
void TaskCheckData(void *pvParameters);
void TaskSendData(void *pvParameters);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;
TaskHandle_t Task_Handle3;

void setup() {
  Serial.begin(9600);
  
  dht.begin();
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  
  radio.begin();
  radio.setChannel(100);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.setCRCLength(RF24_CRC_8);
  radio.openWritingPipe( addresses1[0]);

  xBinarySemaphore = xSemaphoreCreateBinary();
  xTaskCreate(TaskReadData,"Task1",512,NULL,2,&Task_Handle1);
  xTaskCreate(TaskCheckData,"Task2",100,NULL,1,&Task_Handle2);
  xTaskCreate(TaskSendData,"Task3",128,NULL,1,&Task_Handle3);
  xSemaphoreGive(xBinarySemaphore);
}
void loop() {
  // put your main code here, to run repeatedly:
}


void TaskReadData(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  for (;;) // A Task shall never return or exit.
  { 
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    data_rec.temperature= dht.readTemperature();
    data_rec.humidity = dht.readHumidity();
    Serial.print("Temperatura:");
    Serial.println(data_rec.temperature);
    Serial.print("Umiditate:");
    Serial.println(data_rec.humidity);
    xSemaphoreGive(xBinarySemaphore);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void TaskCheckData(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    float temp_limit = 22;
    float hum_limit = 70;
    if(data_rec.temperature > temp_limit){
      //Serial.println("Temp = HIGH");
      limits.tempL= 1;
    }else{
      //Serial.println("Temp = LOW");
      limits.tempL= 0;
    }
    
    if(data_rec.humidity > hum_limit){
      //Serial.println("Hum = HIGH");
      limits.humL= 1;
    }else{
      //Serial.println("Hum = LOW");
      limits.humL= 0;
    }

    Serial.println(limits.tempL);
    Serial.println(limits.humL);
    xSemaphoreGive(xBinarySemaphore);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}


void TaskSendData(void *pvParameters)  // This is a task.
{
  (void) pvParameters;


  for (;;) // A Task shall never return or exit.
  {
    xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    Serial.println("Sending data");

    if (isnan(data_rec.temperature)|| isnan(data_rec.humidity)) { 
    Serial.println(F("Eroare citire senzor!"));
    }else {
    radio.write( &limits, sizeof(limits));
    Serial.println("Data sent");
    }
    
    xSemaphoreGive(xBinarySemaphore);
    
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

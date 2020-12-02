#include <SPI.h>   
#include "RF24.h" 
#include <DHT.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"



RF24 radio (7, 8);
#define DHTPIN 4       
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float TE_low_limit = 20;
float TE_high_limit = 27;
float HU_low_limit = 60;
float HU_high_limit = 80;

byte addresses1[][6] = {"AdrTX"};
byte addresses2[][6] = {"AdrRx"};

typedef struct {  
  float temperature;
  float humidity;
} PACKET;
PACKET data_rec;

typedef struct {  
  boolean TElow = false;
  boolean TEhigh = false;
  boolean HUlow = false;
  boolean HUhigh = false;
} STRUCT;
STRUCT data_levels;

void TaskReadData(void *pvParameters);
void TaskCheckData(void *pvParameters);
void TaskSendData(void *pvParameters);

//SemaphoreHandle_t xBinarySemaphore;

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
  
  //xBinarySemaphore = xSemaphoreCreateBinary();
  //stiva trebuie pusa 192 + cat ocupa restul de functii/variabile
  xTaskCreate(TaskReadData,"Task1",512,NULL,2,&Task_Handle1);
  xTaskCreate(TaskCheckData,"Task2",256,NULL,1,&Task_Handle2);
  xTaskCreate(TaskSendData,"Task3",128,NULL,1,&Task_Handle3);

  //xSemaphoreGive(xBinarySemaphore);
  

}
void loop() {
  // put your main code here, to run repeatedly:
}


void TaskReadData(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  for (;;) // A Task shall never return or exit.
  { 
    //xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    data_rec.temperature= dht.readTemperature();
    data_rec.humidity = dht.readHumidity();

    //trebuie sa fac mutex-uri pentru fiecare Serial.print
    Serial.print("Temperatura:");
    Serial.println(data_rec.temperature);
    Serial.print("Umiditate:");
    Serial.println(data_rec.humidity);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    //xSemaphoreGive(xBinarySemaphore);
  }
  
}


void TaskCheckData(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  

  for (;;) // A Task shall never return or exit.
  {
    //xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    if(data_rec.temperature > TE_high_limit){
      data_levels.TEhigh = true;
      data_levels.TElow = false;
      Serial.println("Temp too high, turning the heating system off ");
    }else if(data_rec.temperature < TE_low_limit){
      data_levels.TElow = true;
      data_levels.TEhigh = false;
      Serial.println("Temp too low, turning the heating system on ");
    }else{
      Serial.print("Temp=ok");
    }

    if(data_rec.humidity > HU_high_limit){
      data_levels.HUhigh = true;
      data_levels.HUlow =false;
      Serial.println("Humidity too high, turning the dehumidifier on ");
    }else if(data_rec.humidity < HU_low_limit){
      data_levels.HUlow =true;
      data_levels.HUhigh = false;
      Serial.println("Humidity too low, turning the dehumidifier off ");
    }else{
      Serial.print("Hum=ok");
    }
    //xSemaphoreGive(xBinarySemaphore);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}


void TaskSendData(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  

  for (;;) // A Task shall never return or exit.
  {   
    //xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
    Serial.println("Sending data");

    //if (isnan(data_rec.temperature)|| isnan(data_rec.humidity)) { 
    //Serial.println(F("Eroare citire senzor!"));
    //}else {}
    radio.write( &data_levels, sizeof(data_levels));
    Serial.println("Data sent");
    //xSemaphoreGive(xBinarySemaphore);
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

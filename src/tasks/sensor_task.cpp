#include <Arduino.h>
#include <DHT.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "config/pins.h"
#include "config/data_types.h"

// 外部声明
extern QueueHandle_t sensorDataQueue;

// DHT传感器对象
DHT dht(DHT_PIN, DHT_TYPE);

void sensorTask(void *pvParameters) {
  // 初始化DHT传感器
  dht.begin();
  
  // 安全打印初始化信息
  Serial.println("[传感器任务] DHT传感器初始化完成");
  
  SensorData currentData;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 1000 / portTICK_PERIOD_MS; // 1秒间隔
  
  while(1) {
    // 精确的1秒间隔执行
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    
    // 读取温湿度
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    // 检查读取是否成功
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("[传感器任务] 读取DHT传感器失败!");
      continue;
    }
    
    // 填充数据结构
    currentData.temperature = temperature;
    currentData.humidity = humidity;
    currentData.timestamp = xTaskGetTickCount();
    
    // 发送到队列（如果队列满则等待最多100ms）
    if (xQueueSend(sensorDataQueue, &currentData, 100 / portTICK_PERIOD_MS) != pdTRUE) {
      Serial.println("[传感器任务] 传感器数据队列已满，数据丢失!");
    }
  }
}
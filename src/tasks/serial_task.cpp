#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "config/pins.h"
#include "config/data_types.h"

// 外部声明
extern QueueHandle_t sensorDataQueue;
extern QueueHandle_t irCommandQueue;
extern SemaphoreHandle_t xSerialMutex;

void serialTask(void *pvParameters) {
  SensorData sensorData;
  uint32_t irCode;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 1000 / portTICK_PERIOD_MS; // 1秒间隔
  
  uint32_t taskRunCount = 0;
  
  while(1) {
    // 精确的1秒间隔执行
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    taskRunCount++;
    
    // 获取互斥锁进行安全打印
    if (xSemaphoreTake(xSerialMutex, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
      Serial.println("\n=== 系统状态更新 ===");
      Serial.printf("任务执行计数: %lu\n", taskRunCount);
      Serial.printf("当前时间戳: %lu\n", xTaskGetTickCount());
      
      // 检查并显示传感器数据
      if (xQueueReceive(sensorDataQueue, &sensorData, 0) == pdTRUE) {
        Serial.printf("🌡️  温度: %.1f°C\n", sensorData.temperature);
        Serial.printf("💧 湿度: %.1f%%\n", sensorData.humidity);
        Serial.printf("📊 数据时间戳: %lu\n", sensorData.timestamp);
      } else {
        Serial.println("📊 暂无新的传感器数据");
      }
      
      // 检查并显示红外命令
      if (xQueueReceive(irCommandQueue, &irCode, 0) == pdTRUE) {
        Serial.printf("🔄 红外命令: 0x%08lX\n", irCode);
      }
      
      // 显示队列状态
      Serial.printf("📦 传感器队列剩余: %d\n", uxQueueMessagesWaiting(sensorDataQueue));
      Serial.printf("📦 红外队列剩余: %d\n", uxQueueMessagesWaiting(irCommandQueue));
      Serial.println("====================\n");
      
      xSemaphoreGive(xSerialMutex);
    }
  }
}
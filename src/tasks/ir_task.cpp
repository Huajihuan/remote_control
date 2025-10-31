#include <Arduino.h>
#include <IRremote.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "config/pins.h"

// 外部声明
extern QueueHandle_t irCommandQueue;
extern SemaphoreHandle_t xSerialMutex;

// 红外接收器对象
IRrecv irrecv(IR_RECEIVER_PIN);
decode_results results;

void irReceiverTask(void *pvParameters) {
  // 初始化红外接收
  irrecv.enableIRIn();
  
  if (xSemaphoreTake(xSerialMutex, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
    Serial.println("[红外任务] 红外接收器初始化完成");
    xSemaphoreGive(xSerialMutex);
  }
  
  while(1) {
    // 检查红外信号（非阻塞方式）
    if (irrecv.decode(&results)) {
      uint32_t irCode = results.value;
      
      if (xSemaphoreTake(xSerialMutex, 500 / portTICK_PERIOD_MS) == pdTRUE) {
        Serial.printf("[红外任务] 接收到红外命令: 0x%08lX\n", irCode);
        xSemaphoreGive(xSerialMutex);
      }
      
      // 发送到命令队列
      if (xQueueSend(irCommandQueue, &irCode, 0) != pdTRUE) {
        if (xSemaphoreTake(xSerialMutex, 500 / portTICK_PERIOD_MS) == pdTRUE) {
          Serial.println("[红外任务] 红外命令队列已满!");
          xSemaphoreGive(xSerialMutex);
        }
      }
      
      // 恢复接收
      irrecv.resume();
    }
    
    // 短暂延迟以避免占用太多CPU
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
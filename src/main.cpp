#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "config/pins.h"
#include "config/data_types.h"

// 前向声明
void sensorTask(void *pvParameters);
void serialTask(void *pvParameters);
void irReceiverTask(void *pvParameters);

// RTOS对象
QueueHandle_t sensorDataQueue;
QueueHandle_t irCommandQueue;
SemaphoreHandle_t xSerialMutex;

void setup()
{
  Serial.begin(115200);

  // 创建RTOS对象
  sensorDataQueue = xQueueCreate(5, sizeof(SensorData));
  irCommandQueue = xQueueCreate(10, sizeof(uint32_t));
  xSerialMutex = xSemaphoreCreateMutex();

  // 等待串口初始化
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // 安全打印启动信息
  if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE)
  {
    Serial.println("==================================");
    Serial.println("   ESP32 RTOS 红外遥控器项目");
    Serial.println("==================================");
    Serial.printf("FreeRTOS内核版本: %s\n", tskKERNEL_VERSION_NUMBER);
    Serial.printf("CPU频率: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("可用堆内存: %d bytes\n", esp_get_free_heap_size());
    xSemaphoreGive(xSerialMutex);
  }

  // 创建任务
  xTaskCreatePinnedToCore(
      sensorTask,    // 任务函数
      "Sensor Task", // 任务名称
      4096,          // 栈大小
      NULL,          // 参数
      2,             // 优先级
      NULL,          // 任务句柄
      0              // 核心0
  );

  xTaskCreatePinnedToCore(
      serialTask,
      "Serial Task",
      4096,
      NULL,
      1, // 较低优先级
      NULL,
      1 // 核心1
  );

  xTaskCreatePinnedToCore(
      irReceiverTask,
      "IR Receiver Task",
      4096,
      NULL,
      2, // 与传感器任务相同优先级
      NULL,
      0 // 核心0
  );

  Serial.println("所有任务创建完成，系统启动！");
}

void loop()
{
  // 主循环运行在优先级1的任务中
  // 这里可以放置低优先级后台任务
  vTaskDelay(10000 / portTICK_PERIOD_MS); // 10秒延迟

  // 定期显示系统状态
  if (xSemaphoreTake(xSerialMutex, 1000 / portTICK_PERIOD_MS) == pdTRUE)
  {
    Serial.printf("[系统状态] 堆内存: %dB, 最小堆内存: %dB\n",
                  esp_get_free_heap_size(),
                  esp_get_minimum_free_heap_size());
    xSemaphoreGive(xSerialMutex);
  }
}

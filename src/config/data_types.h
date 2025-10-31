#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <Arduino.h>

// 传感器数据结构
struct SensorData {
  float temperature;
  float humidity;
  uint32_t timestamp;
};

// 红外命令数据结构
struct IRCommand {
  uint32_t code;
  uint8_t protocol;
  uint32_t timestamp;
};

#endif
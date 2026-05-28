
#include <Arduino.h>
#include <HardwareSerial.h>
#include "connection_system.h"
#include "webserver_station.h"

connection_system connections;

unsigned char *packet = 0, awaiting_temp_humid = 0, red_light = 1;
unsigned long last_time_ms_temp_humid, last_time_ms_red_led, last_time_ms_flood, last_time_ms_turbine, last_time_ms_devices;
#define send_buffer_len 5
unsigned char send_buffer[send_buffer_len];

void packet_data();
void packet_flood();
float normalize_float(float number);

void setup(){
  setCpuFrequencyMhz(240);
  //Serial.begin(115200);
  connections.start();
  start_webserver_station();
  last_time_ms_temp_humid = millis();
  last_time_ms_red_led = last_time_ms_temp_humid;
  last_time_ms_flood = last_time_ms_temp_humid;
  last_time_ms_turbine = last_time_ms_temp_humid;
  last_time_ms_devices = last_time_ms_temp_humid;
  //Serial.print("\nESP32 IoT station has started\n");
}

unsigned char buz_on = 0, light_intensity = 0, turbine_on = 0;

void loop(){
  unsigned long current_time = millis();
  if(current_time - last_time_ms_temp_humid >= 5000){
    last_time_ms_temp_humid = current_time;
    if(awaiting_temp_humid == 0){
      if(connections.can_send(device_moisture_sensor, 1)){
        connections.clear_read_buffer_and_packet(device_moisture_sensor);
        send_buffer[0] = instruction_request_data;
        connections.send_packet(device_moisture_sensor, send_buffer, 1);
        connections.set_use_status(device_moisture_sensor, 1);
        awaiting_temp_humid = 1;
        delay(5);
      }
    }else{
      awaiting_temp_humid = 0;
      connections.set_use_status(device_moisture_sensor, 0);
    }
  }else if((current_time - last_time_ms_flood >= 2000) && (!awaiting_temp_humid)){
    last_time_ms_flood = current_time;
    if(connections.can_send(device_moisture_sensor, 1)){
      connections.clear_read_buffer_and_packet(device_moisture_sensor);
      send_buffer[0] = instruction_request_data | 0x10;
      connections.send_packet(device_moisture_sensor, send_buffer, 1);
      delay(1);
    }else{
      light_intensity = 0;
      buz_on = 0;
      turbine_on = 0;
      std::string str;
      str = "20";
      webserver_station_textall(str.c_str());
      delay(1);
      str = "100.000.0";
      webserver_station_textall(str.c_str());
    }
  }
  if(connections.can_send(device_rgb, 1)){
    if(current_time - last_time_ms_red_led >= 1000){
      last_time_ms_red_led = current_time;
      send_buffer[0] = instruction_send_data;
      send_buffer[1] = 2;
      send_buffer[2] = 255 - light_intensity;
      send_buffer[3] = light_intensity;
      send_buffer[4] = 0;
      connections.send_packet(device_rgb, send_buffer, 5);
      delay(1);
      if(connections.can_send(device_rgb, 1)){
        if(buz_on){
          send_buffer[0] = 0x13;
        }else{
          send_buffer[0] = 0x23;
        }
        connections.send_packet(device_rgb, send_buffer, 1);
      }
    }
  }
  if(connections.can_send(device_turbine, 1)){
    if(current_time - last_time_ms_turbine >= 1000){
      last_time_ms_turbine = current_time;
      if(turbine_on){
        send_buffer[0] = 0x13;
      }else{
        send_buffer[0] = 0x23;
      }
      connections.send_packet(device_turbine, send_buffer, 1);
    }
  }
  packet = connections.get_packet(device_moisture_sensor);
  if(packet){
    if(packet[0] == instruction_request_data){
      packet_data();
    }else{
      packet_flood();
    }
    connections.clear_read_packet(device_moisture_sensor);
  }
  if(current_time - last_time_ms_devices >= 1000){
    last_time_ms_devices = current_time;
    std::string str;
    str = "3";
    if(connections.can_send(device_moisture_sensor, 1)){
      str += "1";
    }else{
      str += "0";
    }
    if(connections.can_send(device_rgb, 1)){
      str += "1";
    }else{
      str += "0";
    }
    if(connections.can_send(device_turbine, 1)){
      str += "1";
    }else{
      str += "0";
    }
    webserver_station_textall(str.c_str());
    delay(1);
  }
  loop_webserver_station();
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void packet_data(){
  connections.set_use_status(device_moisture_sensor, 0);
  awaiting_temp_humid = 0;
  if(packet[1] != 3){
    return;
  }
  //990 - 2432
  float humidity = normalize_float((float)((((unsigned short)packet[3]) << 8) | (unsigned short)packet[2]) * 0.00416f);
  //640 - 768
  float temperature = normalize_float((float)((((unsigned short)packet[5]) << 8) | (unsigned short)packet[4]) * 0.00375f);
  light_intensity = 127;
  if(humidity > 30.0f){
    light_intensity -= 127 * ((humidity - 30.0f) / 70.0f);
  }
  light_intensity += 127;
  if(temperature > 23.0f){
    if(temperature > 30.0f){
      temperature = 30.0f;
    }
    light_intensity -= 127 * ((temperature - 23.0f) / 7.0f);
  }
  if(light_intensity < 128){
    turbine_on = 1;
  }else{
    turbine_on = 0;
  }
  std::string line = std::to_string(humidity);
  line.resize(4);
  std::string str = "1" + line;
  line = std::to_string(temperature);
  line.resize(4);
  str += line;
  webserver_station_textall(str.c_str());
  //Serial.println(str.c_str());
}

void packet_flood(){
  std::string str;
  if(packet[2] < 128){
    buz_on = 1;
    str = "21";
  }else{
    buz_on = 0;
    str = "20";
  }
  webserver_station_textall(str.c_str());
}

float normalize_float(float number){
  if(number < 0.0f){
    number = 0.0f;
  }else if(number > 99.9f){
    number = 99.9f;
  }
  return number;
}

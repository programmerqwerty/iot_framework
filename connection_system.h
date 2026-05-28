
#ifndef connection_system_h
#define connection_system_h

//esp32
//default cpu frequency 160mhz
//gpio full
//  4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
//  maybe: 13, 14, 16, 17
//gpio input only
//  34, 35, 36, 39
//dangerous flash pins
//  6, 7, 8, 9, 10, 11
//dangerous strapping pins
//  0, 2, 5, 12, 15

#include <Arduino.h>
#include <HardwareSerial.h>

#define cs_wired_ports_amount 3 //change to 2 for debug
#define cs_wireless_ports_amount 0
#define cs_byte_send_confirm_code 0x77
#define port_type_wired 1
#define port_type_wireless 2
#define device_moisture_sensor 1
#define device_rgb 2
#define device_turbine 3
#define buffer_max_index 127
#define connect_await_time_us 100000
#define connect_attempts 2
#define disconnect_check_timer_us 1000000
#define packet_max_await_us 2000
#define byte_send_confirm_await_us 1000
#define instruction_connect 1
#define instruction_request_data 2
#define instruction_command 3
#define instruction_send_data 4

class port{
public:
  unsigned char type, device_code = 0, sending_packet = 0, read_index = 0, has_packet = 0, got_header = 0, read_size = 0, is_in_use = 0;
  unsigned char read_buffer[buffer_max_index + 1];
  HardwareSerial *controller;
  unsigned long long disconnect_check_time_save;
  port(unsigned char a_type);
  void try_to_connect();
  void clear_read_packet();
};

class connection_system{
public:
  port* wired_ports[cs_wired_ports_amount], wireless_ports[cs_wireless_ports_amount];
  connection_system();
  ~connection_system();
  void start();
  unsigned char* get_packet(unsigned char a_device_code);
  unsigned char can_send(unsigned char a_device_code, unsigned char bytes_amount);
  unsigned char send_packet(unsigned char a_device_code, unsigned char *send_buffer, unsigned char bytes_amount);
  void clear_read_packet(unsigned char a_device_code);
  void clear_read_buffer_and_packet(unsigned char a_device_code);
  void set_use_status(unsigned char a_device_code, unsigned char a_is_in_use);
};

extern connection_system *cs0;

void thread_connection_system(void * pvParameters);

#endif

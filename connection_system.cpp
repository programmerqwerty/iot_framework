
#include "connection_system.h"

HardwareSerial u0(0);//5 18
HardwareSerial u1(1);//16 17
HardwareSerial u2(2);//21 22

connection_system *cs0;

connection_system::connection_system(){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    wired_ports[i0] = new port(port_type_wired);
    wired_ports[i0]->read_buffer[0] = 0;
  }
  //tbd wireless
  for(i0 = 0;i0 < cs_wireless_ports_amount;i0++){
    wired_ports[i0] = new port(port_type_wireless);
  }
}

connection_system::~connection_system(){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    delete wired_ports[i0];
  }
  for(i0 = 0;i0 < cs_wireless_ports_amount;i0++){
    delete wired_ports[i0];
  }
}

void connection_system::start(){
  u0.begin(500000, SERIAL_8N1, 5, 18);
  u1.begin(500000, SERIAL_8N1, 16, 17);
  u2.begin(500000, SERIAL_8N1, 21, 22);
  wired_ports[0]->controller = &u1;
  wired_ports[1]->controller = &u2;
  wired_ports[2]->controller = &u0;
  cs0 = this;
  TaskHandle_t thread_handler;
  xTaskCreatePinnedToCore(thread_connection_system, "cs", 4096, NULL, 1, &thread_handler, 1);
}

unsigned char* connection_system::get_packet(unsigned char a_device_code){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    if(wired_ports[i0]->device_code == a_device_code){
      if(wired_ports[i0]->has_packet){
        return wired_ports[i0]->read_buffer;
      }
    }
  }
  return 0;
}

unsigned char connection_system::can_send(unsigned char a_device_code, unsigned char bytes_amount){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    //Serial.print("device: ");
    //Serial.print(wired_ports[i0]->device_code);
    if(wired_ports[i0]->device_code == a_device_code){
      //Serial.print("; buffer: ");
      //Serial.print(wired_ports[i0]->controller->availableForWrite());
      //Serial.print("\n");
      if(wired_ports[i0]->controller->availableForWrite() >= bytes_amount){
        return 1;
      }else{
        return 0;
      }
    }
    //Serial.print("\n");
  }
  return 0;
}

unsigned char connection_system::send_packet(unsigned char a_device_code, unsigned char *send_buffer, unsigned char bytes_amount){
  unsigned char i0, i1, byte, errors_amount = 0;
  unsigned long long time_save;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    if(wired_ports[i0]->device_code == a_device_code){
      if(bytes_amount == 1){
        wired_ports[i0]->controller->write(send_buffer[0]);
        wired_ports[i0]->controller->flush();
      }else{
        for(i1 = 0;i1 < bytes_amount;i1++){
          wired_ports[i0]->controller->write(send_buffer[i1]);
          wired_ports[i0]->controller->flush();
          yield();
          time_save = esp_timer_get_time();
          while(1){
            if(wired_ports[i0]->controller->available()){
              byte = wired_ports[i0]->controller->read();
              if(byte != cs_byte_send_confirm_code){
                errors_amount++;
              }
              break;
            }
            if(esp_timer_get_time() - time_save > byte_send_confirm_await_us){
              errors_amount++;
              break;
            }
          }
        }
      }
      return errors_amount;
    }
  }
  return 0xff;
}

void connection_system::clear_read_packet(unsigned char a_device_code){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    if(wired_ports[i0]->device_code == a_device_code){
      wired_ports[i0]->clear_read_packet();
    }
  }
}

void connection_system::clear_read_buffer_and_packet(unsigned char a_device_code){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    if(wired_ports[i0]->device_code == a_device_code){
      wired_ports[i0]->clear_read_packet();
      while(wired_ports[i0]->controller->available()){
        wired_ports[i0]->controller->read();
      }
    }
  }
}

void connection_system::set_use_status(unsigned char a_device_code, unsigned char a_is_in_use){
  unsigned char i0;
  for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
    if(wired_ports[i0]->device_code == a_device_code){
      wired_ports[i0]->is_in_use = a_is_in_use;
    }
  }
}

port::port(unsigned char a_type){
  type = a_type;
  device_code = 0;
}

void port::try_to_connect(){
  unsigned char attempts = 0, byte;
  unsigned long long time_save = 0;
  while(1){
    controller->write(instruction_connect);
    controller->flush();
    time_save = esp_timer_get_time();
    yield();
    while(1){
      while(controller->available()){
        byte = controller->read();
        //Serial.print("connect byte:");
        //Serial.print(byte);
        //Serial.print("\n");
        if((byte & 0b1111) == instruction_connect){
          device_code = byte >> 4;
          return;
        }
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);
      if(esp_timer_get_time() - time_save > connect_await_time_us){
        break;
      }
    }
    attempts++;
    //Serial.print("attempt\n");
    if(attempts >= connect_attempts){
      device_code = 0;
      return;
    }
  }
}

void port::clear_read_packet(){
  got_header = 0;
  read_size = 0;
  read_buffer[0] = 0;
  has_packet = 0;
}

//connection_system *cs0;

void thread_connection_system(void *pvParameters){
  unsigned char i0, byte;
  port *p0;
  while(1){
    for(i0 = 0;i0 < cs_wired_ports_amount;i0++){
      p0 = cs0->wired_ports[i0];
      //connection
      if(!p0->device_code){
        //Serial.print("trying to connect u1\n");
        p0->try_to_connect();
        if(!p0->device_code){
          continue;
        }
        Serial.print("connected: ");
        Serial.print(p0->device_code);
        Serial.print(" at ");
        Serial.print(i0 + 1);
        Serial.print("\n");
        p0->disconnect_check_time_save = esp_timer_get_time();
      }else{
        if((p0->is_in_use == 0) && (esp_timer_get_time() - p0->disconnect_check_time_save >= disconnect_check_timer_us)){
          p0->try_to_connect();
          if(!p0->device_code){
            Serial.print("disconnected\n");
            p0->clear_read_packet();
            continue;
          }else{
            p0->disconnect_check_time_save = esp_timer_get_time();
          }
        }
      }
      //
      //read packet
      if(!p0->has_packet){
        while(p0->controller->available()){
          byte = p0->controller->read();
          if(!p0->got_header){
            p0->got_header = 1;
            p0->read_index = 0;
            p0->read_buffer[p0->read_index++] = byte;
          }else if(!p0->read_size){
            p0->read_size = byte + 1;
            p0->read_buffer[p0->read_index++] = byte;
          }else if(p0->read_index <= buffer_max_index){
            p0->read_buffer[p0->read_index++] = byte;
            p0->read_size--;
            if(!p0->read_size){
              p0->has_packet = 1;
              break;
            }
          }else{
            p0->read_size--;
            if(!p0->read_size){
              p0->has_packet = 1;
              break;
            }
          }
        }
      }
      //
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

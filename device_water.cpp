
//8mhz crystal
//  low, high, lock
//  0xff, 0xd1, 0xff

//device moisture and temperature sensor
//23c0 - debug led
//24c1 - wire to dht22

//atmega8
//8mhz crystal

#define F_CPU 8000000UL
#define DHT_DDR DDRC
#define DHT_PORT PORTC
#define DHT_PIN PINC
#define DHT_INPUTPIN 1
#define DHT_TIMEOUT 200
#define device_code 0b00010000
#define ack_code 0x77

#include <avr/io.h>
#include <util/delay.h>

unsigned char uart_byte = 0, light = 0;
unsigned char data_bytes[5];

unsigned char read_data();
unsigned char uart_read();
void uart_transmit(unsigned char byte);
void process_packets();
unsigned char adc();

int main(){
  //uart
  UBRRH = 0;
  UBRRL = 0;
  UCSRB = (1 << RXEN) | (1 << TXEN);
  UCSRC = (1 << URSEL) | (0b11 << UCSZ0);
  //adc
  ADMUX = (1 << ADLAR);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  adc();
  while(1){
    process_packets();  
  }
}

void process_packets(){
  unsigned char instruction, data;
  if(uart_read()){return;}
  if(uart_byte == 0){
    return;
  }
  instruction = uart_byte & 0b1111;
  data = uart_byte >> 4;
  if(instruction == 0x1){
    uart_transmit(device_code | 0x1);
    return;
  }else if(instruction == 0x2 && data == 0x0){
    read_data();
    uart_transmit(0x2);
    uart_transmit(0x3);
    uart_transmit(data_bytes[1]);
    uart_transmit(data_bytes[0]);
    uart_transmit(data_bytes[3]);
    uart_transmit(data_bytes[2]);
    light = !light;
    PORTC &= ~0b1;
    if(light){
      PORTC |= 0b1;
    }
  }else if(instruction == 0x2 && data == 0x1){
    data = adc();
    uart_transmit(0x12);
    uart_transmit(0x0);
    uart_transmit(data);
  }    
}

unsigned char adc(){
   ADCSRA |= (1 << ADSC);
   while (ADCSRA & (1 << ADSC));
   return ADCH;
}

void uart_transmit(unsigned char byte){
  while((UCSRA & (1 << UDRE)) == 0){}
  UDR = byte;
}

unsigned char uart_read(){
  uart_byte = 0;
  unsigned short counter = 0xffff;
  while(counter){
    if(UCSRA & (1 << RXC)){
      uart_byte = UDR;
      return 0;
    }
    counter--;
  }
  return 1;
}

unsigned char read_data(){
  DHT_DDR |= (1 << DHT_INPUTPIN); //output
  DHT_PORT |= (1 << DHT_INPUTPIN); //high
  _delay_ms(100);
  DHT_PORT &= ~(1 << DHT_INPUTPIN); //low
  _delay_us(500);
  DHT_PORT |= (1 << DHT_INPUTPIN); //high
  DHT_DDR &= ~(1 << DHT_INPUTPIN); //input
  _delay_us(40);
  if((DHT_PIN & (1 << DHT_INPUTPIN))){
    return 0;
  }
  _delay_us(80);
  if(!(DHT_PIN & (1 << DHT_INPUTPIN))){
    return 0;
  }
  _delay_us(80);
  unsigned short timeoutcounter = 0;
  unsigned char i, j;
  for(j = 0;j < 5;j++){
    unsigned char result = 0;
    for(i = 0;i < 8;i++){
      timeoutcounter = 0;
      while(!(DHT_PIN & (1 << DHT_INPUTPIN))){
        timeoutcounter++;
        if(timeoutcounter > DHT_TIMEOUT){
          return 0;
        }
      }
      _delay_us(30);
      if(DHT_PIN & (1 << DHT_INPUTPIN)){
        result |= (1 << (7 - i));
      }
      timeoutcounter = 0;
      while(DHT_PIN & (1 << DHT_INPUTPIN)){
        timeoutcounter++;
        if(timeoutcounter > DHT_TIMEOUT){
          return 0;
        }
      }
    }
    data_bytes[j] = result;
  }
  DHT_DDR |= (1 << DHT_INPUTPIN); //output
  DHT_PORT |= (1 << DHT_INPUTPIN); //low
  _delay_ms(100);
  if((unsigned char)(data_bytes[0] + data_bytes[1] + data_bytes[2] + data_bytes[3]) == data_bytes[4]){
    return 1;
  }
  return 0;
}

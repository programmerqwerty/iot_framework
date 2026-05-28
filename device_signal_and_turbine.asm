
//device signal, ouputs rgb and sound

//attiny2313
//crystal 8mhz
//fuses low, high, extended, lock
//  0xce, 0x9f, 0xff, 0xff
//06d2 - sound
//14b2 - red
//15b3 - green
//16b4 - blue

//registers
//  r0  device code
//  r16 tmp_0
//  r17 tmp_1
//  r18 tmp_2
//  r19 tmp_3
//  r20 is buzzer on

.org 0x0
rjmp init

.org 0x10
init:
  cli
  //hardware
  ldi r16, low(ramend)
  out spl, r16
  ;ldi r16, high(ramend)
  ;out sph, r16
  clr r16
  out ubrrh, r16
  out ubrrl, r16
  ldi r16, ((1 << rxen) | (1 << txen))
  out ucsrb, r16
  ldi r16, ((0b11 << ucsz0))
  out ucsrc, r16
  clr r16
  out ocr0a, r16
  out icr1h, r16
  out ocr1ah, r16
  out ocr1al, r16
  out ocr1bh, r16
  out ocr1bl, r16
  ser r16
  out icr1l, r16
  ldi r16, ((1 << com1a1) | (1 << com1b1) | (1 << wgm11))
  out tccr1a, r16
  ldi r16, ((1 << wgm13) | (1 << wgm12) | (1 << cs10))
  out tccr1b, r16
  ldi r16, ((1 << com0a1) | (1 << wgm01) | (1 << wgm00))
  out tccr0a, r16
  ldi r16, ((1 << wgm02) | (1 << cs00))
  out tccr0b, r16
  //gpio
  ldi r16, 0b100
  out ddrd, r16
  clr r16
  out portd, r16
  out portb, r16
  ldi r16, 0b11100
  out ddrb, r16
  //registers
  ldi r16, 0b110000 //0x2 rgb_led
  mov r0, r16
  clr r16
  //
  sei
main:
  //buzzer if on
  tst r20
  breq main_check_packet
  in r16, portd
  ori r16, 0b100
  out portd, r16
  ldi r16, 128
  ldi r17, 1
  rcall delay_2
  in r16, portd
  andi r16, 0b11111011
  out portd, r16
  ldi r16, 128
  ldi r17, 1
  rcall delay_2
  //
  main_check_packet:
  rcall uart_await_read
  tst r16
  breq main
  cli
  rcall received_packet
  sei
  //clr r16
  //ldi r17, 0x0f
  //rcall delay_2
rjmp main

delay_2:
  dec r16
  brne delay_2
  dec r17
  brne delay_2
ret

uart_transmit:
  sbis ucsra, udre
  rjmp uart_transmit
  out udr, r16
  clr r16
  ldi r17, 0x1
  rcall delay_2
ret

uart_await_read:
  clr r16
  ldi r17, 0x05
  uart_await_read_await:
  dec r16
  breq uart_await_read_dec_r17
  rjmp uart_await_read_check
  uart_await_read_dec_r17:
  dec r17
  breq uart_await_read_error_ret
  uart_await_read_check:
  sbis ucsra, rxc
  rjmp uart_await_read_await
  in r16, udr
  uart_await_read_error_ret:
ret

received_packet:
  mov r17, r16
  andi r17, 0b1111
  cpi r17, 0x1
  breq r_packet_connect
  cpi r17, 0x2
  breq r_packet_request_data
  cpi r17, 0x3
  breq r_packet_command
  cpi r17, 0x4
  breq r_packet_send_data_fix0
ret

r_packet_connect:
  cpi r16, 0x1
  brne r_packet_connect_regular
  //reset request //not used anymore
    //clr r0
  //
  r_packet_connect_regular:
  tst r0
  brne r_packet_connect_response
  //first connection
    andi r16, 0b11110000
    mov r0, r16
    ldi r16, 0x1
    rcall uart_transmit
    ret
  //
  r_packet_connect_response:
    mov r16, r0
    ori r16, 0x1
    rcall uart_transmit
  //
ret

//return blank 1 byte packet
r_packet_request_data:
  ldi r16, 0x2
  rcall uart_transmit
  clr r16
  rcall uart_transmit
  clr r16
  rcall uart_transmit
ret

r_packet_send_data_fix0:
rjmp r_packet_send_data

r_packet_command:
  swap r16
  andi r16, 0b1111
  cpi r16, 1
  breq r_packet_command_red_on
  cpi r16, 2
  breq r_packet_command_red_off
  cpi r16, 3
  breq r_packet_command_green_on
  cpi r16, 4
  breq r_packet_command_green_off
  cpi r16, 5
  breq r_packet_command_blue_on
  cpi r16, 6
  breq r_packet_command_blue_off
  cpi r16, 7
  breq r_packet_command_white_on
  cpi r16, 8
  breq r_packet_command_white_off
ret
r_packet_command_red_on:
  ;in r16, portd
  ;ori r16, 0b100
  ;out portd, r16
  ;ser r20
ret
r_packet_command_red_off:
  ;in r16, portd
  ;andi r16, 0b11111011
  ;out portd, r16
  ;clr r20
ret
r_packet_command_green_on:
  ;in r16, portc
  ;ori r16, 0b10
  ;out portc, r16
ret
r_packet_command_green_off:
  ;in r16, portc
  ;andi r16, 0b11111101
  ;out portc, r16
ret
r_packet_command_blue_on:
  ;in r16, portc
  ;ori r16, 0b100
  ;out portc, r16
ret
r_packet_command_blue_off:
  ;in r16, portc
  ;andi r16, 0b11111011
  ;out portc, r16
ret
r_packet_command_white_on:
  ;in r16, portc
  ;ori r16, 0b1000
  ;out portc, r16
ret
r_packet_command_white_off:
  ;in r16, portc
  ;andi r16, 0b11110111
  ;out portc, r16
ret

r_packet_send_data:
  ldi r16, 0x77
  rcall uart_transmit
  rcall uart_await_read
  ldi r16, 0x77
  rcall uart_transmit
  rcall uart_await_read
  //r16 red
  out ocr0a, r16
  ldi r16, 0x77
  rcall uart_transmit
  rcall uart_await_read
  //r16 green
  clr r17
  out ocr1ah, r17
  out ocr1al, r16
  ldi r16, 0x77
  rcall uart_transmit
  rcall uart_await_read
  //r16 blue
  clr r17
  out ocr1bh, r17
  out ocr1bl, r16
  ldi r16, 0x77
  rcall uart_transmit
  r_packet_send_data_ret:
ret

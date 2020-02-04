#include <Arduino.h>
#include <shield.h>

int read_Hex_Switch(void)
{
  int hex=0;
  
  if(digitalRead(HEX1_PIN) == LOW)
    hex = 1;
  if(digitalRead(HEX2_PIN) == LOW)
    hex += 1 << 1;
  if(digitalRead(HEX4_PIN) == LOW)
    hex += 1 << 2;
  if(digitalRead(HEX8_PIN) == LOW)
    hex += 1 << 3;

  return hex;
}
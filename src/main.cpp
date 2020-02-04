#include <Arduino.h>
#include <avr/wdt.h>
#include <ICSC.h>
#include <CLI.h>
#include <ANSI.h>
#include <shield.h>



#define ENC_PULSE_PER_ROT  280

#define ENC1_PULSE_IN  21
#define ENC2_PULSE_IN  20
#define ENC3_PULSE_IN  2

#define MOTOR1_PWM_OUT  3
#define MOTOR2_PWM_OUT  4
#define MOTOR3_PWM_OUT  5

#define MOTOR1_DIR_OUT  
#define MOTOR2_DIR_OUT  
#define MOTOR3_DIR_OUT  


#define ROBOT_LEFT  0x10
#define ROBOT_RIGHT 0x11
#define BIN_FERTIG  0x12

volatile long enc1_Value;
volatile long enc2_Value;
volatile long enc3_Value;

int rpm1 = 0;
int rpm2 = 0;
int rpm3 = 0;


int address = 0;
char buffer[50];

char cmdbuffer[50];
char cmd1[50] = "rechts\0"; 
char cmd2[50] = "links\0"; 
char cmd3[50] = "stop\0"; 
char cmd4[50] = "fahr\0"; 

int idx_buf = 0;

void test(void);
void pinger(unsigned char station, char command, unsigned char len, char *data);
void fertig(unsigned char station, char command, unsigned char len, char *data);

void isr_S4(void);
void updateEncoder1(void);
void updateEncoder2(void);
void updateEncoder3(void);

CLI_COMMAND(helpFunc);
CLI_COMMAND(connectFunc);
CLI_COMMAND(ble_send);
CLI_COMMAND(ble_status);
CLI_COMMAND(ble_addr);
CLI_COMMAND(ble_set_name);
CLI_COMMAND(ble_connect);
CLI_COMMAND(ble_renew);
CLI_COMMAND(ble_type);
CLI_COMMAND(icsc_status);
CLI_COMMAND(reset);


bool alive = true;
int counter = 0;



ANSI ansi;
byte rx_byte = 0;        // stores received byte
ICSC icsc(Serial3,1,2);

stats_ptr status;

void setup() {
  Serial3.begin(115200);
  Serial.begin(115200);
  Serial2.begin(9600);
  
  // for(int i = 0; i < WAIT_SEC_AT_START;i++)
  // {
  //     Serial.print(i+1);
  //     delay(1000);
  //     ansi.clr_line();
  // }

    ansi.cls();

    CLI.setDefaultPrompt("MASTER> ");
    CLI.onConnect(connectFunc);
    
    CLI.addCommand("help", helpFunc);
    CLI.addCommand("reset", reset);
    CLI.addCommand("ble_send", ble_send);
    CLI.addCommand("ble_status",ble_status);
    CLI.addCommand("ble_addr",ble_addr);
    CLI.addCommand("ble_connect",ble_connect);
    CLI.addCommand("ble_renew",ble_renew);
    CLI.addCommand("ble_type",ble_type);
    CLI.addCommand("icsc_status",icsc_status);
    CLI.addCommand("ble_set_name",ble_set_name);

    CLI.addClient(Serial);



  icsc.begin();
  icsc.registerCommand(ICSC_SYS_PONG, &pinger);
  icsc.registerCommand(BIN_FERTIG, &fertig);

  pinMode(ETH1_LED_RIGHT,OUTPUT);
  digitalWrite(ETH1_LED_RIGHT, LOW);
  pinMode(ETH1_LED_LEFT,OUTPUT);
  digitalWrite(ETH1_LED_LEFT, HIGH);

  pinMode(HEX1_PIN,INPUT);
  pinMode(HEX2_PIN,INPUT);
  pinMode(HEX4_PIN,INPUT);
  pinMode(HEX8_PIN,INPUT);
  address = read_Hex_Switch();

  pinMode(ENC1_PULSE_IN, INPUT_PULLUP);
  pinMode(ENC2_PULSE_IN, INPUT_PULLUP);
  pinMode(ENC3_PULSE_IN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC1_PULSE_IN), updateEncoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_PULSE_IN), updateEncoder2, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC3_PULSE_IN), updateEncoder3, RISING);





  // Serial.println("**********");
  // for(int i=0;i<10;i++)
  // {
  //   sprintf(buffer,"%2u: %i",i,cmdbuffer[i]);
  //   Serial.println(buffer);
  // }
  // Serial.println("**********");

}

void loop() {
  static unsigned long ts = millis();
  
  CLI.process();
  icsc.process();

   if (Serial2.available()) {
    rx_byte = Serial2.read();
    cmdbuffer[idx_buf] = rx_byte;
    idx_buf++;
    // send byte to serial port 3
    if(rx_byte == 10)
    { 
      cmdbuffer[idx_buf-2] = '\0';
      test();
      idx_buf = 0;      
    }



    Serial.write(rx_byte);

   }


  if (millis() - ts >= 1000) {
    counter++;
    ts = millis();

    rpm1 = (float)(enc1_Value * 60 / ENC_PULSE_PER_ROT);
    rpm2 = (float)(enc2_Value * 60 / ENC_PULSE_PER_ROT);
    rpm3 = (float)(enc3_Value * 60 / ENC_PULSE_PER_ROT);


    if(counter == 2)
    {
      // sprintf(buffer,"Address: 0x%x",address);
      // Serial.println(buffer);

      if(alive == false)
      {
        Serial.println("Gegenstation nicht erreichbar");
        digitalWrite(ETH1_LED_RIGHT, HIGH);
      }
      alive = false;
      digitalWrite(ETH1_LED_LEFT, LOW);
      //Serial.println("SEND PING");
      icsc.send(2, ICSC_SYS_PING, 5,"PING");
  //    counter = 0;
    } 

    // if(counter == 5)
    // {
    //   icsc.send(2,ROBOT_LEFT,0);
    //   Serial.println("Fahr links du Hund!");
    // }
    if(counter >= 10)
    {
      icsc.send(2,ROBOT_RIGHT,0);
      Serial.println("Fahr rechts du Hund!");
      counter = 0;
    }
  }
  


  address = read_Hex_Switch();
}

void test()
{
  
  if(strcmp(cmdbuffer,cmd1) == 0)
  {
      icsc.send(2,ROBOT_LEFT,0);
    Serial.println("fahre rechts");

  }
  else if(strcmp(cmdbuffer,cmd2) == 0)
  {
      icsc.send(2,ROBOT_LEFT,0);
    Serial.println("fahre links");

  }
  else if(strcmp(cmdbuffer,cmd3) == 0)
  {
    Serial.println("STOP!!");
  }
  else if(strcmp(cmdbuffer,cmd4) == 0)
  {
    Serial.println("START");
  }

  else
  {
    sprintf(buffer,"CMD: %s",cmdbuffer);
    Serial.println(buffer);
    Serial.println("kenne cmd nicht");
  }
  
}


void pinger(unsigned char station, char command, unsigned char len, char *data)
{
  alive = true;
  digitalWrite(ETH1_LED_RIGHT, LOW);  
  digitalWrite(ETH1_LED_LEFT, HIGH);
//  Serial.println("Pong");
}

void fertig(unsigned char station, char command, unsigned char len, char *data)
{
  Serial.println("SLAVE ist fertig");
}


void updateEncoder1(){
  enc1_Value++;
}

void updateEncoder2(){
  enc2_Value++;
}

void updateEncoder3(){
  enc3_Value++;
}


CLI_COMMAND(icsc_status)
{
    status = icsc.stats();

    sprintf(buffer,"Col: %ld",status->collision);
    Serial.println(buffer);
    sprintf(buffer,"rx_packets: %ld",status->rx_packets);
    Serial.println(buffer);
    sprintf(buffer,"rx_bytes: %ld",status->rx_bytes);
    Serial.println(buffer);
    sprintf(buffer,"tx_packets: %ld",status->tx_packets);
    Serial.println(buffer);
    sprintf(buffer,"tx_bytes: %ld",status->tx_bytes);
    Serial.println(buffer);
    sprintf(buffer,"tx_fail: %ld",status->tx_fail);
    Serial.println(buffer);
    sprintf(buffer,"cs_errors: %ld",status->cs_errors);
    Serial.println(buffer);
    sprintf(buffer,"cb_run: %ld",status->cb_run);
    Serial.println(buffer);
    sprintf(buffer,"cb_bad: %ld",status->cb_bad);
    Serial.println(buffer);
    return 0;
}

CLI_COMMAND(helpFunc) {
    dev->println("add <number 1> <number 2> - Add two numbers together");
    return 0;
}

CLI_COMMAND(reset) {
    dev->println("Reset the System - Bye Bye");
    wdt_disable();
    wdt_enable(WDTO_15MS);
    while (1) {}


   Serial.print("jump");
    return 0;
}


CLI_COMMAND(connectFunc) {
    dev->flush();
    dev->println("************* DOMA ARDUINO ************");
    dev->println("Welcome to the CLI test.");
    dev->println("Type 'help' to list commands.");
    dev->println();
}

CLI_COMMAND(ble_send) {
  dev->println("send");
  Serial2.println("Hallo du device");
  return 0;
}

CLI_COMMAND(ble_status) {
  dev->print("status: ");
  Serial2.print("AT");
  return 0;
}

CLI_COMMAND(ble_addr) {
  dev->print(" MAC: ");
  Serial2.print("AT+ADDR?");
  return 0;
}

CLI_COMMAND(ble_set_name) {
  dev->print(" Name: ");
  Serial2.print("AT+NAMEMaster");
  return 0;
}

CLI_COMMAND(ble_connect) {
  dev->print(" Name: ");
  Serial2.print("AT+CON9809CF5E548A");
  return 0;
}

CLI_COMMAND(ble_renew) {
  dev->print(" Name: ");
  Serial2.print("AT+RENEW");
  return 0;
}

CLI_COMMAND(ble_type) {
  dev->print(" Type1: ");
  Serial2.print("AT+TYPE1");
  return 0;
}
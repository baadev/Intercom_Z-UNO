#define CALL_PIN              18
#define LED_PIN               9

#define LINE_RELAY_P_1        10
#define LINE_RELAY_P_8        15

#define LGND_RELAY_P_1        8
#define LGND_RELAY_P_8        7

#define BU_PC                 22

#include "EEPROM.h"

byte unlockStatus;
byte callMode;

boolean magic;

ZUNO_SETUP_CHANNELS(ZUNO_SWITCH_BINARY(getLockStatus, setLockStatus));
ZUNO_SETUP_SLEEPING_MODE(ZUNO_SLEEPING_MODE_FREQUENTLY_AWAKE);

ZUNO_SETUP_DEBUG_MODE(DEBUG_ON);

void setup() 
{
  magic = EEPROM.read(0x0000) == 0x0A;
  if (!magic) 
  {
    EEPROM.write(0x0000, 0x0A);
    EEPROM.write(0x000F, 0x00);
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  init_relays();  
}

void loop() 
{
  digitalWrite(LED_PIN, LOW);
  delay(50);
  
  unlockStatus = EEPROM.read(0x000F);

  if ((unlockStatus != 0) && (digitalRead(CALL_PIN) == LOW))
    vass_unlock_intercom();
  else
    init_relays(); 

  digitalWrite(LED_PIN, HIGH);
  zunoSendDeviceToSleep();
}

// z-way
byte getLockStatus() 
{
  return unlockStatus;
}
void setLockStatus(byte newValue) 
{
  if (newValue > 0)
    unlockStatus = 0xFF;
  else
    unlockStatus = 0;

  EEPROM.write(0x000F, unlockStatus > 0 ? 0xFF : 0);
}

typedef enum
{
  con_to_phn = 0,
  pick_up,
  con_to_lgnd,
  break_lgnd    
} relay_cmd_t;


void push_phn_but()
{
  digitalWrite(BU_PC, LOW);  
}

void relize_phn_but()
{
  digitalWrite(BU_PC, HIGH);  
}

#define REL_T_WORK 2
void set_line_relay(byte cmd)
{
  switch(cmd)
  {
     case con_to_phn:
      digitalWrite(LINE_RELAY_P_1, HIGH);
      digitalWrite(LINE_RELAY_P_8, LOW);
      delay(REL_T_WORK); 
      digitalWrite(LINE_RELAY_P_1, LOW);
      break;
     case pick_up:
      digitalWrite(LINE_RELAY_P_1, LOW);
      digitalWrite(LINE_RELAY_P_8, HIGH);
      delay(REL_T_WORK); 
      digitalWrite(LINE_RELAY_P_8, LOW);
      break;
     case con_to_lgnd:
      digitalWrite(LGND_RELAY_P_1, HIGH);
      digitalWrite(LGND_RELAY_P_8, LOW);
      delay(REL_T_WORK); 
      digitalWrite(LGND_RELAY_P_1, LOW);
      break;
     case break_lgnd:
      digitalWrite(LGND_RELAY_P_1, LOW);
      digitalWrite(LGND_RELAY_P_8, HIGH);
      delay(REL_T_WORK); 
      digitalWrite(LGND_RELAY_P_8, LOW);
      break;
  }
}

void init_relays()
{
  pinMode(LINE_RELAY_P_1, OUTPUT);  
  pinMode(LINE_RELAY_P_8, OUTPUT);
  pinMode(LGND_RELAY_P_1, OUTPUT);
  pinMode(LGND_RELAY_P_8, OUTPUT);
  set_line_relay(con_to_phn);
  set_line_relay(con_to_lgnd);

  pinMode(BU_PC, OUTPUT);
  relize_phn_but();
}

void vass_unlock_intercom()
{
  push_phn_but();  
  set_line_relay(pick_up);
  delay(150);
  set_line_relay(break_lgnd);
  delay(1000);
  relize_phn_but();
  delay(200);
  push_phn_but(); 
  set_line_relay(con_to_lgnd); 
  set_line_relay(con_to_phn);
}

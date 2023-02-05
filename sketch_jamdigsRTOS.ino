

#include <Arduino_FreeRTOS.h>
#include <Arduino.h>
#include <TM1637Display.h>

void showHours(void *pvParameters);
void showMinutes(void *pvParameters);
void whichState(void *pvParameters);
void damn(void *pvParameters);


//Module Connection Pins
#define CLK 7
#define DIO 8

//Delay for test
#define TEST_DELAY 1000

//variable
int hours=13, minutes=1,seconds=0;
int hoursAlarm=0, minutesAlarm=0;
int pinBuzzer= 6;
int secondStopwatch = 0;
int minuteStopwatch = 0;
int secondStopwatchNow = 0;
int minuteStopwatchNow = 0;
const int pinButton1 = 4;
const int pinButton2 = 2;
const int pinButton3 = 3;
int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int lastButtonState1 = 0;
int selector=0;

TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(9600);

  // Stop interrupts
  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();

  display.setBrightness(0x0f);
  uint8_t data[] = {0x0,0x0,0x0,0x0};
  display.setSegments(data);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(pinButton1,INPUT);
  pinMode(pinButton2,INPUT);
  pinMode(pinButton3,INPUT);


  xTaskCreate(showHours, "task_1", 128, NULL,tskIDLE_PRIORITY + 1,NULL);
  xTaskCreate(showMinutes, "task_2", 128, NULL,tskIDLE_PRIORITY + 1,NULL);
  xTaskCreate(whichState, "task_3",128,NULL, tskIDLE_PRIORITY +1,NULL);
  xTaskCreate(damn, "task_4",128,NULL, tskIDLE_PRIORITY +1,NULL);

  vTaskStartScheduler();
 
}

void printNum(int hours,int minutes){
  display.showNumberDecEx(minutes, 0, true, 2, 2);
  display.showNumberDecEx(hours, 0x40, true, 2, 0);
}


ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)

    seconds = seconds + 1;
    secondStopwatch = secondStopwatch +1 ;
   
    
  if (seconds > 59) {
    seconds = 0;
    minutes = minutes + 1;
  }
  if (minutes > 59) {
    minutes = 0;
    hours = hours + 1;
  }
  if (hours > 23) {
    hours = 0;
  }

    if (secondStopwatch > 59) {
    secondStopwatch = 0;
    minuteStopwatch = minuteStopwatch + 1;
  }

}

void loop() {
}

void showHours(void *pvParameters){
  while(1){
    display.showNumberDecEx(hours, 0x40, true, 2, 0);
    vTaskDelay(500/portTICK_PERIOD_MS);
    if(selector == 1 or selector ==2){
        printNum(hoursAlarm,minutesAlarm);
         vTaskDelay(500/portTICK_PERIOD_MS);
    }
    else if (selector == 3){
     printNum(minuteStopwatch,secondStopwatch);
      vTaskDelay(500/portTICK_PERIOD_MS);
  }
   else if (selector == 4){
      printNum(minuteStopwatchNow,secondStopwatchNow);
       vTaskDelay(500/portTICK_PERIOD_MS);
     
   }
  }
}

void showMinutes(void *pvParameters){
  while(1){
    display.showNumberDecEx(minutes, 0, true, 2, 2);
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void whichState(void *pvParameters){
  while(1){
    buttonState1 = digitalRead(pinButton1);
    buttonState2 = digitalRead(pinButton2);
    buttonState3 = digitalRead(pinButton3);


    if(buttonState1 != lastButtonState1){
    if (buttonState1 == HIGH){
      selector++;
      if (selector == 5){
        selector =1;
      }
      Serial.println("on");
    }
    else{
      Serial.println("off");
    }
    delay(50);
  }

   lastButtonState1 = buttonState1;
    if (buttonState2 == HIGH and selector == 1){
        hoursAlarm ++;
        delay(100);
        if (hoursAlarm>23){
          hoursAlarm = 0;
        }
        }
     else if(buttonState2 == HIGH and selector == 2){
        minutesAlarm ++;
        delay(100);
        if (minutesAlarm>59){
          minutesAlarm = 0;
        }
      }
    else if(selector == 4){
         secondStopwatchNow = secondStopwatch;  
         minuteStopwatchNow = minuteStopwatch;
         TCNT1 = 0;
    }
  if (buttonState3 == HIGH){
    digitalWrite(pinBuzzer, HIGH);
    selector = 0;
    minuteStopwatch = 0;
    secondStopwatch =0;
  }
  else{
    digitalWrite(pinBuzzer, LOW);
    selector = selector;
    }

   vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void damn(void *pvParameters){
  while(1){
    if(selector == 1 or selector ==2){
    secondStopwatch = 0;
    minuteStopwatch = 0;
    if (hours == hoursAlarm and minutes == minutesAlarm){
       digitalWrite(pinBuzzer, LOW);
      }
    }
  else{
       if (hours == hoursAlarm and minutes == minutesAlarm){
      digitalWrite(pinBuzzer, HIGH);
      digitalWrite(12, HIGH); 
      delay(1000);
      digitalWrite(pinBuzzer, LOW);
      digitalWrite(12, LOW);
      delay(1000);
  }
  else {
    digitalWrite(pinBuzzer, LOW);
  }
  }
  vTaskDelay(500/portTICK_PERIOD_MS);
  
}
}

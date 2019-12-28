#include "freertos/FreeRTOS.h"
#include "driver/pcnt.h"
#include "LocationStatus.h"
#include <FastLED.h>

#define SPEED_SENSOR_PIN    4
#define SPEED_LED_PIN      15
#define SPEED_MAX_MMINS  8000
#define PWM_CHANNEL         4
#define PCNT_UNIT        PCNT_UNIT_5
#define PCNT_CHAN        PCNT_CHANNEL_1

#define LED_PIN         14
#define LEDS_PER_SIDE   29
#define LEDS_COUNT      (1+2*LEDS_PER_SIDE)
#define LED_DISTANCE_IN_MM  16.6

#define SPEED_CALC_SECTORS          1
#define SPEED_CALC_RADIUS_IN_MM    40
#define SPEED_CALC_MILLISECONDS  1000
#define SPEED_CONSIDERED_PARKED_MILLISECONDS 15000

#define COLOR_PRIMARY      CRGB::Orange
#define COLOR_SECONDARY    CRGB::Black
#define COLOR_TERTIARY     CRGB::Black
#define COLOR_BRIGHTNESS_PARKED 3
#define COLOR_BRIGHTNESS_MOVING 45

int sensorValue = 0;  // variable to store the value coming from the sensor
double dSpeedKMH = 0;
double dSpeedmmS = 0;
pcnt_config_t pcntConfig;

CRGB leds[LEDS_COUNT];
CRGB* ledsLeft;
CRGB* ledsRight;

enum ScooterState{
  PARKED,
  MOVING
};

ScooterState scsThis;
  
void recalcSpeed(void *) {
  static int16_t iBuf[3];
  double dDistance;
  double dPulseAvg;
  int16_t iCur;
  unsigned uLastMove=0;
 
  while(true) {
    pcnt_get_counter_value(PCNT_UNIT, iBuf);
    iBuf[2]=iBuf[1];
    iBuf[1]=iBuf[0];
    pcnt_counter_clear(PCNT_UNIT);

    if(iBuf[0]>SPEED_MAX_MMINS) {
      iBuf[0]=iBuf[1]; /*Keep old speed if measurement exceeeds max speed.*/
    }
    dPulseAvg=(2.0*iBuf[0]+iBuf[1]+0.5*iBuf[2])/3.5;
    
    dDistance=SPEED_CALC_RADIUS_IN_MM * PI * 2.0 * dPulseAvg / SPEED_CALC_SECTORS / (SPEED_CALC_MILLISECONDS/1000.);
    if(dDistance) {
      scsThis=MOVING;
      uLastMove=millis();
    } else if (uLastMove + SPEED_CONSIDERED_PARKED_MILLISECONDS < millis()) {
      scsThis=PARKED;
    }
    dSpeedmmS = dDistance;
    dSpeedKMH = dDistance / 277.778; /*mm/s to km/h*/
    delay(SPEED_CALC_MILLISECONDS);
  }
}

void renderLEDController(void *);
void setupLEDController() {
   Serial.begin(115200);
  // declare the ledPin as an OUTPUT:
  pinMode(SPEED_SENSOR_PIN, INPUT);
  pinMode(23, OUTPUT);
  
  ledcSetup(PWM_CHANNEL, 38*1000, 8);
  ledcAttachPin(SPEED_LED_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 128);

  pcntConfig.pos_mode       = PCNT_COUNT_INC;
  pcntConfig.neg_mode       = PCNT_COUNT_DIS;
  pcntConfig.pulse_gpio_num = SPEED_SENSOR_PIN;
  pcntConfig.ctrl_gpio_num  = PCNT_PIN_NOT_USED;
  pcntConfig.unit           = PCNT_UNIT;
  pcntConfig.channel        = PCNT_CHAN;

  pcntConfig.counter_h_lim =  32767;
  pcntConfig.counter_l_lim = -1;

  pcntConfig.lctrl_mode = PCNT_MODE_KEEP;
  pcntConfig.hctrl_mode = PCNT_MODE_KEEP;

  Serial.println(pcnt_unit_config(&pcntConfig));

  pcnt_set_filter_value(PCNT_UNIT, 1);
  pcnt_filter_enable(PCNT_UNIT);
  
  pcnt_intr_disable(PCNT_UNIT);
  pcnt_event_disable(PCNT_UNIT, PCNT_EVT_L_LIM);
  pcnt_event_disable(PCNT_UNIT, PCNT_EVT_H_LIM);
  pcnt_event_disable(PCNT_UNIT, PCNT_EVT_THRES_0);
  pcnt_event_disable(PCNT_UNIT, PCNT_EVT_THRES_1);
  pcnt_event_disable(PCNT_UNIT, PCNT_EVT_ZERO);


  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  pcnt_intr_enable(PCNT_UNIT);
  pcnt_counter_resume(PCNT_UNIT);

  xTaskCreatePinnedToCore( recalcSpeed, /* Task function. */
                           "Speed calculation",  /* String with name of task. */
                           1000,            /* Stack size in words. */
                           NULL,             /* Parameter passed as input of the task */
                           1,                /* Priority of the task. */
                           NULL,             /* Task handle. */
                           1);               /*core id*/
  ledsLeft=leds+1;;
  ledsRight=leds+1+LEDS_PER_SIDE;
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LEDS_COUNT).setCorrection( TypicalLEDStrip );
  scsThis=PARKED;
  delay(500);
  xTaskCreatePinnedToCore( renderLEDController, /* Task function. */
                           "LED rendering",  /* String with name of task. */
                           10000,            /* Stack size in words. */
                           NULL,             /* Parameter passed as input of the task */
                           2,                /* Priority of the task. */
                           NULL,             /* Task handle. */
                           1);               /*core id*/
}

void renderLEDsParked() {
  int i=0;
  switch(locationStatus) {
    case SCANNING:
      leds[0]=CRGB::White;
      break;
    case CONNECT:
      leds[0]=CRGB::Blue;
      leds[0].fadeLightBy(250);
      break;
    case REQUESTING:
      leds[0]=CRGB::Yellow;
      leds[0].fadeLightBy(230);
      break;
    case UPDATING:
      leds[0]=CRGB::Red;
      break;
    case UPDATED:
      leds[0]=CRGB::Red;
      leds[0].fadeLightBy(210);
      break;
    case INVALID:
    default:
      leds[0]=CRGB::Green;
      leds[0].fadeLightBy(210);
  }

    /*Background pattern*/
    for(i = 0; i < LEDS_PER_SIDE; i++) {
      switch (i%3) {
        case 0:
          ledsRight[LEDS_PER_SIDE-i-1] = COLOR_PRIMARY;
          break;
        case 1:
          ledsRight[LEDS_PER_SIDE-i-1] = COLOR_SECONDARY;
          break;
        case 2:
          ledsRight[LEDS_PER_SIDE-i-1] = COLOR_TERTIARY;
          break;
      }
      ledsRight[LEDS_PER_SIDE-i-1].fadeLightBy(255-COLOR_BRIGHTNESS_PARKED);
      ledsLeft[LEDS_PER_SIDE-i-1] = ledsRight[LEDS_PER_SIDE-i-1];
    }
}

void renderLEDsMoving() {
  int i=0;
  static double dDistanceInmm;
  dDistanceInmm += dSpeedmmS/20; /*this is called 20 times per second*/
  leds[0]=CRGB::White;
  
  /*Background pattern*/
  for(i = 0; i < LEDS_PER_SIDE; i++) {
    switch (i%3) {
      case 0:
        ledsRight[LEDS_PER_SIDE-i-1] = COLOR_PRIMARY;
        break;
      case 1:
        ledsRight[LEDS_PER_SIDE-i-1] = COLOR_SECONDARY;
        break;
      case 2:
        ledsRight[LEDS_PER_SIDE-i-1] = COLOR_TERTIARY;
        break;
    }
    ledsRight[LEDS_PER_SIDE-i-1].fadeLightBy(255-COLOR_BRIGHTNESS_MOVING);

    /*Moving animation*/
    if(int(dDistanceInmm+LED_DISTANCE_IN_MM*i)%800<50){
      ledsLeft[LEDS_PER_SIDE-i-1] = ledsRight[LEDS_PER_SIDE-i-1] = CRGB::Green;
    }
    ledsLeft[LEDS_PER_SIDE-i-1] = ledsRight[LEDS_PER_SIDE-i-1];
  }

  /*Frontlight*/
  for(i = 0; i < _min(2,LEDS_PER_SIDE); i++) {
    ledsRight[i] = CRGB::White;
    ledsRight[i].fadeLightBy(255-COLOR_BRIGHTNESS_MOVING);
    ledsLeft[i] = ledsRight[i];
  }
  /*Backlight*/
  for(i = _min(LEDS_PER_SIDE-2,LEDS_PER_SIDE); i < LEDS_PER_SIDE; i++) {
    ledsRight[i] = CRGB::Red; 
    ledsRight[i].fadeLightBy(255-COLOR_BRIGHTNESS_MOVING);
    ledsLeft[i] = ledsRight[i];
  }
}

void renderLEDController(void *) {
  while(true) {
    if(scsThis==PARKED) {
      renderLEDsParked();
    } else {
      renderLEDsMoving();
    }
    FastLED.show();
    delay(50);
  }
}

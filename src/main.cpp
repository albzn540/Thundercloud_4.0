/*
* This sketch is for the rooflamp "ThunderCloud".
* Creates and immersive effect from the cloud.
* Connects over WiFi for controlling the lamp and a physical pushbutton.
*
* Author: Albin Winkelmann
* Version: 4.2
* See date below
*/

const char* lastEdited = "16.01.2017";

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#include <Arduino.h>
#include <OneButton.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LEDFader.h>             //some files are in the local lib

//Assigning the pin values for each LED
const int red1 = D5;
const int red2 = D6;
const int blue1 = D7;
const int blue2 = D8;
const int purple = D9;

const int blueBoardLight = D4;
const int redBoardLight = D0;

//for the _thunder method
#define BETWEEN 4579
#define DURATION 43
#define TIMES 7
unsigned long lastTime1 = 0;
int waitTime1 = 0;
unsigned long lastTime2 = 0;
int waitTime2 = 0;

//for the _fade method
#define LED_NUM 5
LEDFader leds[LED_NUM] = {
  LEDFader(red1),
  LEDFader(red2),
  LEDFader(blue1),
  LEDFader(blue2),
  LEDFader(purple)
};
int nextFade;
unsigned long fadeDuration[LED_NUM];
unsigned long lastFade = 0;
unsigned long fadeDurationVariable = 0;
unsigned long lastVariable = 0;
int fadingLEDS = 0;

//Physical button
const int buttonPin = D2;
uint8_t numberOfModes = 4;        //including "off" mode

//Blynk widgets
int lightButton = 0;
int thunderButton = 0;
int fadeButton = 0;
int offButton = 0;
int fadeSlider = 0;
int blueButton = 0;
int redButton = 0;
int purpleButton = 0;

//unnamed
int mode = 0;
int brightness = 255; //goes from 0 - 255, starting brightness
bool manualButtonsPressedCache = false;

//Setting up OneButton
OneButton button(buttonPin, true);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "4c559cfcd319444881546f7e62650874";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Brexit";
char pass[] = "dinm@mm@";

//widgets for Blynk
WidgetTerminal terminal(V3);
WidgetLCD lcd(V5);

//____________________________________________________________
//             END OF DECLARATIONS AND SHITT

void _light(int value) {
  if (value == 255) {
    digitalWrite(red1, HIGH);
    digitalWrite(red2, HIGH);
    digitalWrite(blue1, HIGH);
    digitalWrite(blue2, HIGH);
    digitalWrite(purple, HIGH);
  }
  else if (value == 0) {
    digitalWrite(red1, LOW);
    digitalWrite(red2, LOW);
    digitalWrite(blue1, LOW);
    digitalWrite(blue2, LOW);
    digitalWrite(purple, LOW);
  }
  else {
    analogWrite(red1, value);
    analogWrite(red2, value);
    analogWrite(blue1, value);
    analogWrite(blue2, value);
    analogWrite(purple, value);
  }
}

void _thunder() {
  if (millis() - waitTime1 > lastTime1)  // time for a new flash
  {
    // adjust timing params
    lastTime1 += waitTime1;
    waitTime1 = random(BETWEEN);

    // Red 1
    for (int i=0; i< random(TIMES); i++)
    {
      //Serial.println(millis());
      digitalWrite(red1, HIGH);
      delay(20 + random(DURATION));
      digitalWrite(red1, LOW);
      delay(10);
    }

   // Blue 2
    for (int k=0; k< random(TIMES); k++)
    {
      //Serial.println(millis());
      digitalWrite(blue2, HIGH);
      delay(20 + random(DURATION));
      digitalWrite(blue2, LOW);
      delay(10);
    }
  }

  if (millis() - waitTime2 > lastTime2)  // time for a new flash
  {
    // adjust timing params
    lastTime2 += waitTime2;
    waitTime2 = random(BETWEEN);

    // Blue 1
    for (int k=0; k< random(TIMES); k++)
    {
      //Serial.println(millis());
      digitalWrite(blue1, HIGH);
      delay(20 + random(DURATION));
      digitalWrite(blue1, LOW);
      delay(10);
    }

     // Red 2
    for (int k=0; k< random(TIMES); k++)
    {
      //Serial.println(millis());
      digitalWrite(red2, HIGH);
      delay(20 + random(DURATION));
      digitalWrite(red2, LOW);
      delay(10);
    }

     // Purple 1
    for (int i=0; i< random(TIMES); i++)
    {
      //Serial.println(millis());
      digitalWrite(purple, HIGH);
      delay(20 + random(DURATION));
      digitalWrite(purple, LOW);
      delay(10);
    }

  }

}

void _fade() {
  fadingLEDS = 0;

  //checks if any lEDs are fading
  for (byte i = 0; i < LED_NUM; i++){
    LEDFader *led = &leds[i];
    led->update();

    if(led->is_fading() == true) {
      fadingLEDS++;
    }

    else {
      //start fade down the already fading LED
      if(led->get_value() == 255) {
        // Fade from 0 - 255
        led->fade(0, fadeDuration[i]);
        fadingLEDS++;
      }
    }
  }

  for (byte i = 0; i < LED_NUM; i++){
    LEDFader *led = &leds[i];

    //start new fade?
    if(fadingLEDS < 2 && random(1,10) == 1) {
      fadeDuration[i] = fadeDurationVariable;

      // Fade from 255 - 0
      if (led->get_value() == 0) {
        led->fade(255, fadeDuration[i]);
      }

      fadingLEDS++;
    }
  }
}

void manualMode () {

  if(blueButton) {
    digitalWrite(blue1, HIGH);
    digitalWrite(blue2, HIGH);
  }
  else {
    digitalWrite(blue1, LOW);
    digitalWrite(blue2, LOW);
  }

  if(redButton) {
    digitalWrite(red1, HIGH);
    digitalWrite(red2, HIGH);
  }
  else {
    digitalWrite(red1, LOW);
    digitalWrite(red2, LOW);
  }

  if(purpleButton) {
    digitalWrite(purple, HIGH);
  }
  else {
    digitalWrite(purple, LOW);
  }
}

void blinkLEDS(int blinks) {
  for(byte i = 0; i < blinks; i++) {
    _light(LOW);
    delay(300);
    _light(HIGH);
    delay(300);
  }
  //set mode to light
  mode = 0;
}

void resetLEDs() {
  for (byte i = 0; i < LED_NUM; i++) {
    LEDFader *led = &leds[i];
    led->stop_fade();
    led->set_value(0);
  }

  //clear leds
  _light(LOW);
}

void switchMode() {

  resetLEDs();
  lcd.clear();

  //clear manual buttons
  Blynk.virtualWrite(V6, LOW);
  Blynk.virtualWrite(V7, LOW);
  Blynk.virtualWrite(V8, LOW);
  manualButtonsPressedCache = false;


  switch (mode) {
    case 0:
    brightness = 255;
    Serial.println("Light");
    terminal.println("Light");
    lcd.print(2, 0, "Let there be");
    lcd.print(6, 1, "Light");
    break;

    case 1:
    Serial.println("Thunder");
    terminal.println("Thunder");
    lcd.print(1, 0, "Current weather");
    lcd.print(5, 1, "Thunder");
    break;

    case 2:
    Serial.println("Fading");
    terminal.println("Fading");
    lcd.print(3, 0, "Moody mode:");
    lcd.print(5, 1, "Fading");
    break;

    case 3:
    Serial.println("Off");
    terminal.println("Off");
    lcd.print(1, 0, "Current weather");
    lcd.print(5, 1, "Sunny");
    break;

    case 4:
    manualButtonsPressedCache = true;
    lcd.clear();
    Serial.println("Hacking mode");
    terminal.println("Hacking mode");
    lcd.print(3, 0, "Hacking mode");
    lcd.print(5, 1, "Enabled");
    break;
  }
  terminal.flush();
}

void selectedMode() {
  switch (mode) {
    case 0:
    _light(brightness);
    break;

    case 1:
    _thunder();
    break;

    case 2:
    _fade();
    break;

    case 3:
    break;

    case 4:
    manualMode();
    break;
  }

}

void buttonClicked() {
  if(mode < 3) {
  mode++;
  }
  else {
    mode = 0;
  }

  switchMode();
}

void checkSliders() {
  //fadingslider
  // automatic fading if fadeSlider = 0
  if (fadeSlider == 0) {
    fadeDurationVariable = random(1500, 4000);
  }
  else {
    fadeDurationVariable = fadeSlider * 50;
    for (byte i = 0; i < LED_NUM; i++){
      LEDFader *led = &leds[i];

      if(lastVariable != fadeDurationVariable) {
        if(led->is_fading() == true) {
          led->change_duration(fadeDurationVariable);
        }
      }

      lastVariable = fadeDurationVariable;
    }
  }

}

bool manualButtonsPressed() {
  if(manualButtonsPressedCache != true) {
    if(redButton || blueButton || purpleButton) {
      manualButtonsPressedCache = true;
      return true;
    }
  }
  return false;
}

void checkButton() {
  //physical button
  button.tick();

  //blynk button
  if(lightButton) {
    mode = 0;
    switchMode();

  }
  else if(thunderButton) {
    mode = 1;
    switchMode();
  }

  else if(fadeButton) {
    mode = 2;
    switchMode();
  }

  else if(offButton) {
    mode = 3;
    switchMode();
  }

  else if(manualButtonsPressed()) {
    mode = 4;

    //switchmode clears the manualButtonsPressedCache so this has to be here

  }
}

void reconnectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    if(Blynk.connect(5000) == true) {
      blinkLEDS(2);
    }
  }
}

void blink(uint8_t led, uint8_t times, uint8_t groundState) {
  for(uint8_t i = 0; i < times; i++) {
    digitalWrite(led, !groundState);
    delay(100);
    digitalWrite(led, groundState);
    delay(100);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Lightningcloud 4.2");
  Serial.println("The cloud source code by Albin Winkelmann");
  Serial.print("Created in 2016         ");
  Serial.print("Last edited: ");
  Serial.println(lastEdited);
  Serial.println("Initializing...");
  Serial.print("");

  pinMode(red1, OUTPUT);
  pinMode(red2, OUTPUT);
  pinMode(blue1, OUTPUT);
  pinMode(blue2, OUTPUT);
  pinMode(purple, OUTPUT);
  pinMode(redBoardLight, OUTPUT);
  pinMode(blueBoardLight, OUTPUT);

  digitalWrite(redBoardLight, HIGH);
  digitalWrite(blueBoardLight, HIGH);

  button.attachClick(buttonClicked);
  button.setClickTicks(200);

  button.attachLongPressStart(reconnectWiFi);
  button.setPressTicks(600);

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(redBoardLight, LOW);
  }
  digitalWrite(redBoardLight, HIGH);

  //blink if connection succeeded
  if (WiFi.status() == WL_CONNECTED) {
    blink(blueBoardLight, 2, HIGH);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Blynk.config(auth);
  if (Blynk.connect(5000) == true) {
    Serial.println("Connected to Blynk servers");
  }
  else {
    Serial.println("Who the fuck need the Bylnk servers anyway?");
  }

  lcd.clear();
  Blynk.virtualWrite(V4, 255);
  Blynk.virtualWrite(V6, 0);
  Blynk.virtualWrite(V7, 0);
  Blynk.virtualWrite(V8, 0);


  Serial.println("Done!");
  switchMode();
}

BLYNK_WRITE(V0) {
  lightButton = param.asInt();
}
BLYNK_WRITE(V1) {
  thunderButton = param.asInt();
}
BLYNK_WRITE(V2) {
  fadeButton = param.asInt();
}
BLYNK_WRITE(V3) {
  fadeSlider = param.asInt();
}
BLYNK_WRITE(V4) {
  brightness = param.asInt();
}
BLYNK_WRITE(V6) {
  blueButton = param.asInt();
}
BLYNK_WRITE(V7) {
  redButton = param.asInt();
}
BLYNK_WRITE(V8) {
  purpleButton = param.asInt();
}
BLYNK_WRITE(V9) {
  offButton = param.asInt();
}


//for the lux sensor on the phone
BLYNK_WRITE(V5) {
  int a = param.asInt();
  brightness = round(25*a);
}

void loop()
{

  //Blynk will only run if connected to the internet
  if(WiFi.status() == WL_CONNECTED) {
    if(Blynk.connected() == true) {
      Blynk.run();
    }
  }

  checkSliders();
  checkButton();
  selectedMode();
}

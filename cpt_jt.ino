#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <SimpleTimer.h>

#include "Adafruit_GFX.h"
#include "Max72xxPanel.h"
#include "RTClib.h"


// Constante

const long period_refresh_date = 10l * 1000l; // 10 s
const long period_affichage_date_ref = 30l * 60l * 1000l; // 30m
const long period_affichage_encouragement = 7l * 60l * 1000ll; // 7m



RTC_DS1307 RTC;

int pinCS = 10; // Attach CS to this pin, DIN to MOSI (11) and CLK to SCK (13) (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

const int buttonPin = 2; 
int lastButtonState = LOW;

String tape = "200";
int wait = 200; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

SimpleTimer timer;

void print_text_complete(String const& texte, int const& wait = 100){
  
  matrix.fillScreen(LOW);
  
  int text_size_px = width * texte.length() - 1;
  
  for(int pixel_position = 0; pixel_position < text_size_px ; pixel_position++){
    if( text_size_px < matrix.width() - spacer){
      // Pas de defilement
      pixel_position = text_size_px;
    }
    
    int letter = pixel_position / width;
    int x = (matrix.width() - 1) - pixel_position % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
  
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < texte.length() ) {
        matrix.drawChar(x, y, texte[letter], HIGH, LOW, 1);
      } else {
        matrix.drawChar(x, y, ' ', HIGH, LOW, 1);
      }
  
      letter--;
      x -= width;
    }
    pixel_position++;
    
    matrix.write(); // Send bitmap to display
    delay(wait);
   
  }
  delay(wait*2);
}

uint16_t get_duree(){
  uint16_t nbDayRef;
  EEPROM.get(0, nbDayRef);
  
  DateTime now = RTC.now(); 
  uint16_t nbDayNow = now.daystime();
  
  Serial.println(nbDayNow);
  Serial.println(nbDayRef);
  Serial.println("Nb jours " + String(nbDayNow - nbDayRef));
  return nbDayNow - nbDayRef;
}


void save_date(){
  DateTime now = RTC.now(); 
  uint16_t nbDayTotal = now.daystime();
  
  EEPROM.put(0, nbDayTotal);
  Serial.println("SAVE data " + String(nbDayTotal));
  
  EEPROM.put(8, now);
  
  print_text_complete( String("Remise a zero du compteur") );
}

void print_duree(){
  uint16_t nbDayTotal = get_duree();
  print_text_complete( String(nbDayTotal), 100 );
}

void print_reference(){
  DateTime now;
  EEPROM.get(8, now);
  char  msg[256];
  sprintf(msg, "Date du dernier AT : %d-%02d-%02d", now.year(), now.month(), now.day());
  
  print_text_complete( msg );
  delay(500);
  print_duree();
}

void print_encouragement(){

  uint16_t nbDayTotal = get_duree();
  if ( nbDayTotal < 10 ) {
    print_text_complete( "Aller ça ne fait que commencer" );
  } else if ( nbDayTotal < 20 ) {
    print_text_complete( "Aller ca ne fait que commencer" );
  } else if ( nbDayTotal < 200 ) {
    print_text_complete( "Vous êtes des machines !" );
  }
  
  delay(500);
  print_duree();
}

void setup() {

  Serial.begin(9600);
  
  matrix.setIntensity(15); // Use a value between 0 and 15 for brightness

  // Adjust to your own needs
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
  Serial.println("Matrix initiated");

  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
    Serial.print("RTC adjusted at ");
    Serial.print(__DATE__);
    Serial.print("-");
    Serial.println(__TIME__);
  }

  pinMode(buttonPin, INPUT);

  print_duree();

  timer.setInterval(period_affichage_encouragement, print_encouragement);
  timer.setInterval(period_affichage_date_ref, print_reference);
  timer.setInterval(period_refresh_date, print_duree);
  
}


void loop() {
  
  int buttonState = digitalRead(buttonPin);

  if( buttonState == HIGH && lastButtonState == LOW){
    save_date();
    
    print_duree();
  }

  timer.run();

  lastButtonState = buttonState;
}

#include <Time.h>
#include <Wire.h>  
#include <Encoder.h>
#include <TinyGPS.h>
#include <DS3232RTC.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

// Defines / settings
#define RGBLEDPIN     6
#define RotaryAPIN    2
#define RotaryBPIN    3
#define TimeChangePIN 4
#define GPSclockRX   10
#define GPSClockTx   11
#define N_LEDS       117          // 13 wide x 9 high grid
#define colorAmount  (4*(sizeof(colors)/sizeof(uint32_t)))



SoftwareSerial SerialGPS = SoftwareSerial(GPSclockRX, GPSClockTx);
TinyGPS gps; 

Encoder myEnc(RotaryAPIN, RotaryBPIN);
long oldPosition  = -999;
long old2Position = -999;
boolean encoderPushed = false;

time_t prevDisplay = 0; // when the digital clock was displayed

Adafruit_NeoPixel grid = Adafruit_NeoPixel(N_LEDS, RGBLEDPIN, NEO_GRB + NEO_KHZ400);

int intBrightness = 255; // the brightness of the clock (0 = off and 255 = 100%)
int offset = 1; //(GMT+1, add another 1 for summertime)
String strTime = ""; // used to detect if word time has changed
time_t prevTime = 0;

//Clock Colours
uint32_t colorWhite = grid.Color(255, 255, 255);
uint32_t colorBlack = grid.Color(0, 0, 0);
uint32_t colorRed = grid.Color(255, 30, 30);
uint32_t colorGreen = grid.Color(30, 255, 30);
uint32_t colorBlue = grid.Color(30, 30, 255);
uint32_t colorPurple = grid.Color(255, 0, 255);
uint32_t colorYellow = grid.Color(125, 255, 0);
uint32_t colorWhatever = grid.Color(102,51,204);
uint32_t colorMore = grid.Color(255,153,0);
uint32_t colorVariable; //current colour
uint32_t colors[] = {colorWhite, colorRed, colorGreen, colorBlue, colorPurple, colorYellow, colorWhatever, colorMore}; //cycle through with encoder

//Words
int arrIT[] = {116,115,-1};
int arrIS[] = {113,112,-1};
int arrHALF[] = {110,109,108,107,-1};
int arrA[] = {109,-1}; //part of HALF
int arrMTEN[] = {106,105,104,-1};

int arrQUARTER[] = {91,92,93,94,95,96,97,-1};
int arrTWENTY[] = {98,99,100,101,102,103,-1};

int arrMFIVE[] = {90,89,88,87,-1};
int arrMINUTES[] = {85,84,83,82,81,80,79,-1};

int arrPAST[] = {65,66,67,68,-1};
int arrPAVLA[] = {70,71,72,73,74,-1};
int arrTO[] = {76,77,-1};

int arrONE[] = {64,63,62,-1};
int arrTWO[] = {60,59,58,-1};
int arrTHREE[] = {56,55,54,53,52,-1};

int arrFOUR[] = {39,40,41,42,-1};
int arrFIVE[] = {43,44,45,46,-1};
int arrSEVEN[] = {47,48,49,50,51,-1};

int arrSIX[] = {38,37,36,-1};
int arrEIGHT[] = {35,34,33,32,31,-1};
int arrNINE[] = {29,28,27,26,-1};

int arrTEN[] = {14,15,16,-1};
int arrELEVEN[] = {19,20,21,22,23,24,-1};

int arrTWELVE[] = {12,11,10,9,8,7,-1};
int arrOCLOCK[] = {5,4,3,2,1,0,-1};


void setup()
{
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  SerialGPS.begin(9600);
  Serial.println("Booting ... ");
  
  delay(200);
  
   // setup the LED strip  
  grid.begin();
  grid.show();


  // initialize the buttons
  pinMode(TimeChangePIN, INPUT_PULLUP); //temp pulled up to not have the low on the button..
  pinMode(RotaryAPIN, INPUT_PULLUP);
  pinMode(RotaryBPIN, INPUT_PULLUP);

  // set the brightness of the strip
  grid.setBrightness(intBrightness);
  
  //startup sequence
  colorWipe(colorBlack, 0);
  paintWord(arrPAVLA,colorPurple);
  delay(1000);
  fadeOut(10);
  colorWipe(colorBlack, 0);
  
  
}

void loop() {
  while (SerialGPS.available()) {
    if (gps.encode(SerialGPS.read())) { // process gps messages
      // when TinyGPS reports new data...
      unsigned long age;
      int Year;
      byte Month, Day, Hour, Minute, Second;
      gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age);
      if (age < 500) {
        //Serial.println("GPS time available with good age - updated RTC");
        // set the Time to the latest GPS reading
        setTime(Hour, Minute, Second, Day, Month, Year);
        RTC.set(now());
      }
    }
  }
  setTime(RTC.get()+(offset * SECS_PER_HOUR)); 
  
  
  if(digitalRead(TimeChangePIN) == HIGH){
    if (encoderPushed!=digitalRead(TimeChangePIN)) {
      myEnc.write(0);
      colorWipe(colorBlack,0);
    }
    encoderPushed = digitalRead(TimeChangePIN);
    
    // encoder rotation for colours
    long newPosition = myEnc.read();
    if (newPosition > colorAmount) { newPosition = 0; myEnc.write(0);}
    if (newPosition < 0) { newPosition = colorAmount; myEnc.write(colorAmount);}
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
    } 
    colorVariable = colors[(oldPosition/4)];
  }
  
  // test to see if a forward button is being held down for time setting
  if(digitalRead(TimeChangePIN) == LOW){
        if (encoderPushed!=digitalRead(TimeChangePIN)) {
            myEnc.write(0);
            colorWipe(colorBlack,0);
    }
    encoderPushed = digitalRead(TimeChangePIN);
    
    long timezonePosition = myEnc.read();
      //Serial.print("Encoder: ");
      //Serial.println(timezonePosition);
        if (timezonePosition > 48) { timezonePosition = 0; myEnc.write(0);}
        if (timezonePosition < 0) { timezonePosition = 48; myEnc.write(48);}
        if (timezonePosition != old2Position) {
        old2Position = timezonePosition;
      } 
      offset = old2Position/4;
      Serial.print("  Timezone: ");
      Serial.println(offset);


    }

  grid.setBrightness(intBrightness);  

  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if the time has changed
      prevDisplay = now();
      digitalClockDisplay();  
      displayTime();
    }
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println();
  
  //Serial.print("RTC time: ");
  //Serial.println(RTC.get()); 
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color, uint8_t wait) {
  for(uint16_t i=0; i<grid.numPixels(); i++) {
      grid.setPixelColor(i, color);
  }
  grid.show();
  delay(wait);
}

void paintWord(int arrWord[], uint32_t intColor){
  for(int i = 0; i < grid.numPixels() + 1; i++){
    if(arrWord[i] == -1){
      grid.show();
      break;
    }else{
      grid.setPixelColor(arrWord[i],intColor);
    }
  }
}

void fadeOut(int time){
  for (int i = intBrightness; i > 0; --i){
    grid.setBrightness(i);
    grid.show();
    delay(time);
  }
}

void displayTime(){
  String strCurrentTime; // build the current time
  // Now, turn on the "It is" leds
  
  paintWord(arrIT, colorVariable);
  paintWord(arrIS, colorVariable);
  
  // now we display the appropriate minute counter
  if((minute()>4) && (minute()<10)){
    // FIVE MINUTES
    strCurrentTime = "five ";
    paintWord(arrMFIVE, colorVariable);
    paintWord(arrMINUTES, colorVariable);
  } 
  if((minute()>9) && (minute()<15)) { 
    //TEN MINUTES;
    strCurrentTime = "ten ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorVariable);
  }
  if((minute()>14) && (minute()<20)) {
    // QUARTER
    strCurrentTime = "quarter ";
    paintWord(arrMTEN, colorBlack);
    paintWord(arrA, colorVariable);
    paintWord(arrQUARTER, colorVariable);
    paintWord(arrMINUTES, colorBlack);
  }
  if((minute()>19) && (minute()<25)) { 
    //TWENTY MINUTES
    strCurrentTime = "twenty ";
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrA, colorBlack);
    paintWord(arrTWENTY, colorVariable);
    paintWord(arrMINUTES, colorVariable);
  }
  if((minute()>24) && (minute()<30)) { 
    //TWENTY FIVE
    strCurrentTime = "twenty five ";
    paintWord(arrMFIVE, colorVariable);
  }  
  if((minute()>29) && (minute()<35)) {
    strCurrentTime = "half ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrHALF, colorVariable);
    paintWord(arrTWENTY, colorBlack);
    paintWord(arrMINUTES, colorBlack);
  }
  if((minute()>34) && (minute()<40)) { 
    //TWENTY FIVE
    strCurrentTime = "twenty five ";
    paintWord(arrMFIVE, colorVariable);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorVariable);
    paintWord(arrMINUTES, colorVariable);
  }  
  if((minute()>39) && (minute()<45)) {
    
    strCurrentTime = "twenty ";
    paintWord(arrMFIVE, colorBlack);
  }
  if((minute()>44) && (minute()<50)) {
    strCurrentTime = "quarter ";
    paintWord(arrQUARTER, colorVariable);
    paintWord(arrA, colorVariable);
    paintWord(arrTWENTY, colorBlack);
    paintWord(arrMINUTES, colorBlack);
  }
  if((minute()>49) && (minute()<55)){
    strCurrentTime = "ten ";
    paintWord(arrMTEN, colorVariable);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrA, colorBlack);
    paintWord(arrMINUTES, colorVariable);
  } 
  if(minute()>54){
    strCurrentTime = "five ";
    paintWord(arrMFIVE, colorVariable);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrMINUTES, colorVariable);
  }
  if((minute() <5)){
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMINUTES, colorBlack);
    switch(hour()){
      case 1:
      case 13:
      strCurrentTime = strCurrentTime + "one ";
        paintWord(arrONE, colorVariable);
        paintWord(arrTWELVE, colorBlack);
      break;
    case 2:
    case 14:
      strCurrentTime = strCurrentTime + "two ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorVariable);
      break;
    case 3: 
    case 15:
      strCurrentTime = strCurrentTime + "three ";
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorVariable);
      break;
    case 4: 
    case 16:
      strCurrentTime = strCurrentTime + "four ";
        paintWord(arrTHREE, colorBlack);
      break;
    case 5: 
    case 17:
      strCurrentTime = strCurrentTime + "five ";
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorVariable);
      break;
    case 6: 
    case 18:
      strCurrentTime = strCurrentTime + "six ";
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorVariable);
      break;
    case 7: 
    case 19:
      strCurrentTime = strCurrentTime + "seven ";
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorVariable);
      break;
    case 8: 
    case 20:
      strCurrentTime = strCurrentTime + "eight ";
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorVariable);
      break;
    case 9: 
    case 21:
      strCurrentTime = strCurrentTime + "nine ";
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorVariable);
      break;
    case 10:
    case 22:
      strCurrentTime = strCurrentTime + "ten ";
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorVariable);
      break;
    case 11:
    case 23:
      strCurrentTime = strCurrentTime + "eleven ";
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorVariable);
      break;
    case 0:
    case 12: 
    case 24:
      strCurrentTime = strCurrentTime + "twelve ";
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorVariable);
      break;
    }
    strCurrentTime = strCurrentTime + "oclock ";
    paintWord(arrPAST, colorBlack);
    paintWord(arrOCLOCK, colorVariable);
    paintWord(arrTO, colorBlack);
  }else if((minute() < 35) && (minute() >4)){
    strCurrentTime = strCurrentTime + "past ";
    paintWord(arrPAST, colorVariable);
    paintWord(arrOCLOCK, colorBlack);
    paintWord(arrTO, colorBlack);
    switch (hour()) {
      case 1:
      case 13:
        strCurrentTime = strCurrentTime + "one ";
        paintWord(arrONE, colorVariable);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 2: 
      case 14:
        strCurrentTime = strCurrentTime + "two ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorVariable);
        break;
      case 3: 
      case 15:
        strCurrentTime = strCurrentTime + "three ";
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorVariable);
        break;
      case 4: 
      case 16:
        strCurrentTime = strCurrentTime + "four ";
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorVariable);
        break;
      case 5: 
      case 17:
        strCurrentTime = strCurrentTime + "five ";
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorVariable);
        break;
      case 6: 
      case 18:
        strCurrentTime = strCurrentTime + "six ";
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorVariable);
        break;
      case 7: 
      case 19:
        strCurrentTime = strCurrentTime + "seven ";
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorVariable);
        break;
      case 8: 
      case 20:
        strCurrentTime = strCurrentTime + "eight ";
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorVariable);
        break;
      case 9: 
      case 21:
        strCurrentTime = strCurrentTime + "nine ";
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorVariable);
        break;
      case 10:
      case 22:
        strCurrentTime = strCurrentTime + "ten ";
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorVariable);
        break;
      case 11:
      case 23:
        strCurrentTime = strCurrentTime + "eleven ";
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorVariable);
        break;
      case 0:
      case 12:
      case 24:
        strCurrentTime = strCurrentTime + "twelve ";
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorVariable);
        break;
      }
    }else{
      // if we are greater than 34 minutes past the hour then display
      // the next hour, as we will be displaying a 'to' sign
      strCurrentTime = strCurrentTime + "to ";
      paintWord(arrPAST, colorBlack);
      paintWord(arrOCLOCK, colorBlack);
      paintWord(arrTO, colorVariable);
      switch (hour()) {
        case 1: 
        case 13:
        strCurrentTime = strCurrentTime + "two ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorVariable);
        break;
      case 14:
      case 2:
        strCurrentTime = strCurrentTime + "three ";
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorVariable);
        break;
      case 15:
      case 3:
        strCurrentTime = strCurrentTime + "four ";
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorVariable);
        break;
      case 4: 
      case 16:
        strCurrentTime = strCurrentTime + "five ";
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorVariable);
        break;
      case 5: 
      case 17:
        strCurrentTime = strCurrentTime + "six ";
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorVariable);
        break;
      case 6: 
      case 18:
        strCurrentTime = strCurrentTime + "seven ";
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorVariable);
        break;
      case 7: 
      case 19:
        strCurrentTime = strCurrentTime + "eight ";
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorVariable);
        break;
      case 8: 
      case 20:
        strCurrentTime = strCurrentTime + "nine ";
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorVariable);
        break;
      case 9: 
      case 21:
        strCurrentTime = strCurrentTime + "ten ";
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorVariable);
        break;
      case 10: 
      case 22:
        strCurrentTime = strCurrentTime + "eleven ";
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorVariable);
        break;
      case 11: 
      case 23:
        strCurrentTime = strCurrentTime + "twelve ";
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorVariable);
        break;
      case 0:
      case 12: 
      case 24:
        strCurrentTime = strCurrentTime + "one ";
        paintWord(arrONE, colorVariable);
        paintWord(arrTWELVE, colorBlack);
        break;
    }
  }

  if(strCurrentTime != strTime){
    digitalClockDisplay();
    strTime = strCurrentTime;
    if(strTime == ""){  //seems to run too often?
      delay(20); // was fadeIN
    }
  }
}


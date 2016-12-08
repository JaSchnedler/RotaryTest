#import <neopixel.h>
#import <math.h>


unsigned long currentTime;
unsigned long loopTime;
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev=0;

const int pin_A = 5;
const int pin_B = 6;
const int INTENSITY = 0;
const int COLOR = 1;
const int FIDDLE = 2;
const int AREA = 3;
const int TOLERANCE = 4;
const int PRESET = 5;
const int PRESETINTENSITY = 5;
const int PRESETCOLOR = 6;
const int RED = 0;
const int GREEN = 1;
const int BLUE = 2;


const int stripRingPin = 1;
const int NUMPIXRing = 15;
const int NUMPIXSTRIPUP = 24;
const int stripUpPin = 0;
const int buttonPin = 3;

int menuCounter = 0;

bool intensityBool = false;
bool colorBool = false;
bool fiddleBool = false;
bool areaBool = false;
bool toleranceBool = false;
bool presetBool = false;
bool menu = true;
bool submenu = false;
bool INCREMENT = false;
bool DECREMENT = false;
bool menuCalculation = false;
bool growingToleranceFlag = true;
bool longpress = false;
bool shortpress = false;
bool fromSleep = false;

int SUBMENUMINVAL = 1;
int SUBMENUMAXVAL = 48;
int menuLength = 6;
int intensity = 30;
int color = 30;
int fiddle = 20;
int area = 3;
int tolerance = 3;
int preset = 0;
int presetIntensity = 30;
int presetColor = 30;

int listOfSubmenuBoolsLength = 6;
int subMenuValsLength = 7;
int areaMenuCounter = 0;
float lastAreaMenuUpdate = 0;
int toleranceMenuCounter = 0;
float tolerenceMenuTimer = 0;
int numberOfLamps = 7;

float latestButtonPress = 0;
float latestInteraction = 0;

//lists of values
int rgbVals[3];
int colorMenuVals[3];
String menuNames[6] = {"Intensity", "Color", "Fiddle", "Area", "Tolerance", "Preset"};
int submenuVals[7] = {intensity, color, fiddle, area, tolerance, presetIntensity,presetColor}; // 0 = intensity, 1 = colourTemp 2 = fiddling val ...etc...
bool listOfSubmenuBools[6] = {intensityBool, colorBool, fiddleBool, areaBool, toleranceBool, presetBool}; // 0=intensityBool 1= colorBool ... etc...
//Pixels
Adafruit_NeoPixel stripRing = Adafruit_NeoPixel(NUMPIXRing, stripRingPin);
Adafruit_NeoPixel stripUp = Adafruit_NeoPixel(NUMPIXSTRIPUP, stripUpPin);
//Colours
uint32_t magentaRing = stripRing.Color(255, 0, 255);
uint32_t smileyGreen = stripRing.Color(30,255,30);
uint32_t noColor = stripRing.Color(0,0,0);


unsigned int localPort = 8888;
UDP Udp;
IPAddress remoteIPPacketSender(192,168,1,14);
IPAddress remoteIPServer(192,168,1,151);
int port = 11000;
int packetSenderPort = 55056;
int processingPort = 12000;



void setup()  {
  Udp.begin(localPort);
  pinMode(9, OUTPUT);
  pinMode(stripRingPin, OUTPUT);
  pinMode(stripUpPin, OUTPUT);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);

  currentTime = millis();
  loopTime = currentTime;

  Serial.begin (9600);
  Serial.println("start");
  stripRing.begin();
  stripUp.begin();
  checkHardware();
  stripUp.show();
  updateBasedOnMenu(menuCounter);
  updateColor(submenuVals[COLOR]);
  updateIntensity(submenuVals[INTENSITY]);
  for(int i = 0; i<menuLength;i++){
    Serial.println(submenuVals[i]);
  }
  latestInteraction = millis();
}

void loop()  {
  updateRotary(); //either increments or decreanses overall counter.
  handleButtonPress();
  if(millis()-latestInteraction < 30000){
    updateCurrentValue();  //address the current value by spinning.
  }else{
    resetStrip(stripUpPin);
    fromSleep = true;
    resetStrip(stripRingPin);
    Serial.print("formsleep: ");
    Serial.println(fromSleep);
  }
  //printStatus();
}
//Loop functions, called in the loop.
/*DESCRIPTION
This function takes care of updating the rotary encoder. This is done as the
first thing in every cycle of the code. The basic principle is that if one pin
is triggered before another the rotary encoder has been turned that way. Thus,
we can determine the rotation direction by looking for the first activated pin.
*/
void updateRotary(){
  INCREMENT = false;
  DECREMENT = false;
  currentTime = millis();
  if(currentTime >= (loopTime + 5)){
    // 5ms since last check of encoder = 200Hz
    encoder_A = digitalRead(pin_A);    // Read encoder pins
    encoder_B = digitalRead(pin_B);
    if((!encoder_A) && (encoder_A_prev)){
      // A has gone from high to low
      if(encoder_B) {
        // B is high so clockwise
        // increase the brightness, dont go over 255
        Serial.print("up, menuCounter: ");
        Serial.println(menuCounter);
        //printStatus();
        INCREMENT = true;

      }
      else {
        // B is low so counter-clockwise
        // decrease the brightness, dont go below 0
        Serial.print("down, menuCounter: ");
        Serial.println(menuCounter);
        DECREMENT = true;
      }

    }
    encoder_A_prev = encoder_A;     // Store value of A for next time

    // set the brightness of pin 9:

    loopTime = currentTime;  // Updates loopTime
    }
  }
/*DESCRIPTION
If the button is pressed and it hasnt been a very short while since the last
press (avoid double readings), we the detemine if we are currently in the
upper menu or a submenu. If in the outermenu, we enter the specific menu using
enterSubmenu() and then we se flags for the following cycles to run correctly.
If we are not in the menu we must be a submenu, which we leave using
leaveSubmenu() and setting the flags back.
*/
void handleButtonPress(){
  for(int i = 0;i < 80;i++){
    if(!digitalRead(buttonPin)){
      longpress = true;
      shortpress = true;
      delay(10);
    }else{
      longpress = false;
      break;
    }
  }

  if(longpress){
      Serial.print("Long press");
      updateBasedOnLongPress();
      longpress = false;
      latestButtonPress = millis();
  }

  bool lastPressBool = (millis() - latestButtonPress) > 500;
  if(shortpress && lastPressBool && !longpress){
    latestButtonPress = millis();
    Serial.println("Short press");

    if(menu & !fromSleep){
      //for(int i = 0; i < menuLength; i++){
        //if(menuCounter == i){
          enterSubmenu(menuCounter);
          updateBasedOnSubmenuVal(menuCounter);
          menu = false;
          submenu = true;
        //}
      //}
      }else if(!menu){
        //for(int i = 0; i < menuLength; i++){
          //if(menuCounter == i){
            leaveSubmenu(menuCounter);
            menu = true;
            submenu = false;
          //}
        //}
      }
      latestInteraction = millis();
      shortpress = false;
      if(fromSleep){
        fromSleep = false;
        updateBasedOnMenu(menuCounter);
        updateColor(submenuVals[COLOR]);
      }
    }

  }
/*DESCRIPTION
If in the menu we ask if the rotary changed, if so we update the small pixelring
to reflect that change. If in the submenu and the rotary encoder changed we
determine what submenu we are in and use the function updateBasedOnSubmenuVal()
to branch to the correct update function.
*/
void updateCurrentValue(){
  if(menu){
    if(INCREMENT || DECREMENT){
      if(INCREMENT){if(menuCounter == 5){menuCounter = 0;}else{menuCounter++;}}
      if(DECREMENT){if(menuCounter == 0){menuCounter = 5;}else{menuCounter--;}}
        setPixelColor(stripRingPin,0,0,0);
      for(int i = 0 ;i < menuLength; i++){
        if(menuCounter == i){
          updateBasedOnMenu(i);
          Serial.println(menuNames[i]);
        }
      }
    }
    if(menuCounter == AREA){displayAreaMenu();}
    if(menuCounter == TOLERANCE){displayToleranceMenu();}

  }
  if(submenu){
    if(INCREMENT || DECREMENT){
      updateBasedOnMenu(menuCounter);
      for(int i = 0; i < subMenuValsLength;i++){
        if(listOfSubmenuBools[i] == true){
          if(INCREMENT && submenuVals[i] != SUBMENUMAXVAL){
            submenuVals[i] = submenuVals[i]+1;
            updateBasedOnSubmenuVal(i);
          }
          if(DECREMENT && submenuVals[i] != SUBMENUMINVAL){
            //Serial.print("Value before decrement  ");
            //Serial.println(submenuVals[i]);
            submenuVals[i] = submenuVals[i]-1;
            updateBasedOnSubmenuVal(i);
            //Serial.print("Value after decrement  ");
            //Serial.println(submenuVals[i]);
          }
        }
      }
    }
  }
 }

 //Entering and leaving submenus
 /*DESCRIPTION
 Enters submenu by setting the flag back to false, and if it was the fiddle
 submenu we set the preview back to the lights as they were.
 */
 void enterSubmenu(int menuItem){
         listOfSubmenuBools[menuItem] = true;
         updateBasedOnMenu(menuItem);
         Serial.print("Entered submenu: ");
         Serial.println(menuItem);
         Serial.print("The active boolean is: ");
         Serial.println(listOfSubmenuBools[menuItem]);
         if(menuItem == AREA || menuItem == TOLERANCE){
           updateIntensity(SUBMENUMAXVAL);
         }
 }
 void leaveSubmenu(int menuItem){

       listOfSubmenuBools[menuItem] = false;
       Serial.print("Left submenu: ");
       Serial.println(menuItem);
       updateBasedOnMenu(menuItem);
       //Serial.print("The boolean is: ");
       //Serial.println(listOfSubmenuBools[i]);
       if(menuItem == FIDDLE || menuItem == AREA || menuItem == TOLERANCE){
         updateColor(submenuVals[COLOR]);
         updateIntensity(submenuVals[INTENSITY]);
       }
       latestInteraction = millis();

 }

//Behavior branching functions:
/*DESCIPTION
Takes a submenu which it is supposed to update. This function determines what
update function to call based on which submenu is currently being altered.
*/
void updateBasedOnSubmenuVal(int submenuItem){
  //Serial.println("about to update chosen value");
  if(submenuItem == INTENSITY){
    updateIntensity(submenuVals[INTENSITY]);
    displayIntensityMenu();
  }
  if(submenuItem == COLOR){updateColor(submenuVals[COLOR]);}
  if(submenuItem == FIDDLE){fiddleFunction(submenuVals[FIDDLE]);}
  if(submenuItem == AREA){
    updateArea(submenuVals[AREA]);
    displayAreaMenu();
  }
  if(submenuItem == TOLERANCE){
    updateTolerance(submenuVals[TOLERANCE]);
    displayToleranceMenu();
  }
  if(submenuItem == PRESET){updatePreset();}

  }

void updateBasedOnMenu(int menuItem){
  //Serial.println("Branch into the desired display");
  if(menuItem != AREA){areaMenuCounter = 0;}
  if(menuItem != TOLERANCE){
    toleranceMenuCounter = 0;
    growingToleranceFlag = true;
  }
  if(menuItem == INTENSITY){displayIntensityMenu();}
  if(menuItem == COLOR){displayColorMenu();}
  if(menuItem == FIDDLE){displayFiddleMenu();}
  if(menuItem == AREA){displayAreaMenu();}
  if(menuItem == TOLERANCE){displayToleranceMenu();}
  if(menuItem == PRESET){
    displayPresetMenu();
    updatePreset();
  }else{
    updateColor(submenuVals[COLOR]);
  }
}
void updateBasedOnLongPress(){
  if(menuCounter == INTENSITY){sendIntensity();}
  if(menuCounter == COLOR){sendTemperature();}
  //if(menuCounter == FIDDLE){displayFiddleMenu();}
  if(menuCounter == AREA){sendAdjustedArea();}
  //if(menuCounter == TOLERANCE){displayToleranceMenu();}
  if(menuCounter == PRESET){
    savePreset();

  }
}
//menu updating functions:
void displayIntensityMenu(){
  if(listOfSubmenuBools[INTENSITY]){
    Serial.println("should have low intensity on the menu now");
    for(int i = 0; i < NUMPIXRing ; i++){
      int intens = map(i,0,NUMPIXRing,1,60);
      Serial.println(intens);
      stripRing.setPixelColor(i,stripRing.Color(intens,intens,intens));
    }
  }else{
    Serial.println("should have HIGH intensity on the menu now");
    for(int i = 0; i < NUMPIXRing ; i++){
      int temp = map(i,0,NUMPIXRing,1,200);
      Serial.println(temp);
      stripRing.setPixelColor(i,stripRing.Color(temp,temp,temp));
      }
    }
  stripRing.show();
}
void displayColorMenu(){
  if(listOfSubmenuBools[COLOR]){
    for(int i = 0; i < NUMPIXRing ; i++){
      int temp = map(i,0,NUMPIXRing,0,100);
      //Serial.println(temp);
      updateMenuColor(temp);
      int r = (float(colorMenuVals[RED])/100) * 40;
      int g = (float(colorMenuVals[GREEN])/100) * 40;
      int b = (float(colorMenuVals[BLUE]/100)) * 40;
      stripRing.setPixelColor(i,stripRing.Color(r,g,b));
    }
  }else{
    for(int i = 0; i < NUMPIXRing ; i++){
      int temp = map(i,0,NUMPIXRing,0,100);
      //Serial.println(temp);
      updateMenuColor(temp);
      stripRing.setPixelColor(i,stripRing.Color(colorMenuVals[RED],colorMenuVals[GREEN],colorMenuVals[BLUE]));
    }
  }
  stripRing.show();
}

void displayFiddleMenu(){
  for(int i=0; i< stripRing.numPixels(); i++) {
    //int value = map(i,0,100,0,stripRing.numPixels());
    stripRing.setPixelColor(i, Wheel(((i * 256 / stripRing.numPixels()) + i) & 255));
  }
  stripRing.show();
}
void displayAreaMenu(){
  bool areaAnimationTimer = (currentTime - lastAreaMenuUpdate) > 200;
  int midPixel = NUMPIXRing/2;
  if(areaAnimationTimer){
    if((midPixel - areaMenuCounter) >= 0 && (midPixel + areaMenuCounter) <= NUMPIXRing){
      stripRing.setPixelColor(midPixel + areaMenuCounter , stripRing.Color(0,150,0));
      //Serial.print("Set pixel:");
      //Serial.println(midPixel+areaMenuCounter);
      stripRing.setPixelColor(midPixel - areaMenuCounter , stripRing.Color(0,150,0));
      //Serial.print("Set pixel:");
      //Serial.println(midPixel-areaMenuCounter);
      lastAreaMenuUpdate = millis();
      areaMenuCounter ++;
      }
      if(midPixel-areaMenuCounter == - 1){
        areaMenuCounter = 0;
        //Serial.println("stripRing and counter should be reset");
      }
    }
  stripRing.show();
}
void displayToleranceMenu(){
  bool toleranceMenuTimerBool = currentTime - tolerenceMenuTimer > 200;
  //Serial.println(currentTime - tolerenceMenuTimer);
  if(toleranceMenuTimerBool){
    resetStrip(stripRingPin);
    if(toleranceMenuCounter < (NUMPIXRing/2) && growingToleranceFlag ){
      stripRing.setPixelColor(toleranceMenuCounter,magentaRing);
      stripRing.setPixelColor((NUMPIXRing-1) - toleranceMenuCounter,magentaRing);
      toleranceMenuCounter ++;
    } else{
      stripRing.setPixelColor(toleranceMenuCounter,magentaRing);
      stripRing.setPixelColor((NUMPIXRing-1) - toleranceMenuCounter,magentaRing);
      toleranceMenuCounter --;
      growingToleranceFlag = false;
    }
    if(!growingToleranceFlag && toleranceMenuCounter == 0){
      growingToleranceFlag = true;
    }
    tolerenceMenuTimer = millis();
    stripRing.show();

  }
}
void displayPresetMenu(){
      for(int i = 0; i < NUMPIXRing;i++){
        if(i % 2 == 0){
          stripRing.setPixelColor(i,smileyGreen);
        }
      }
    stripRing.show();
}
//Feedback in the form of animations to indicate a reaction
void acceptPresetBlinks(){
  setPixelColor(stripUpPin,0,200,0);
  stripUp.show();
  delay(300);
  resetStrip(stripUpPin);
  delay(300);
  setPixelColor(stripUpPin,0,200,0);
  stripUp.show();
  delay(300);
  resetStrip(stripUpPin);
  delay(300);
  setPixelColor(stripUpPin,0,200,0);
  stripUp.show();
  delay(300);
  resetStrip(stripUpPin);
  updateColor(submenuVals[PRESETCOLOR]);
  updateIntensity(submenuVals[PRESETINTENSITY]);
}
void sendToCeilingBlinks(){
  /*
  for(int i = 0; i < NUMPIXSTRIPUP; i++){
    stripUp.setPixelColor(i,smileyGreen);
    stripUp.setPixelColor(i-1,noColor);
    stripUp.show();
    delay(50);
  }
  */
}

//Update functions called to update the pixel strips
/*DESCRIPTION
This function takes a value from 0 - 100 and updates the intensity of the light
based on that value. Multiplying by 2 instead of 2.55 keeps the intensity of the
light away from the maximum, which takes a large energytoll.
*/
void updateIntensity(int intens){
  int r = intens * (rgbVals[RED]/SUBMENUMAXVAL);
  int g = intens * (rgbVals[GREEN]/SUBMENUMAXVAL);
  int b = intens * (rgbVals[BLUE]/SUBMENUMAXVAL);
  setPixelColor(stripUpPin,r,g,b);
  //Serial.print("Updated Intensity: ");
  //Serial.println(intens);
}
/*DESCRIPTION
This function is very math heavy, and there is no need to understand it.
It takes an argument from 0 - 100 and calculates a temperature based on that
value. In the end the colour is applied using the function setPixelColor()
*/
void updateColor(int tempPercentage){
  int readyVal = 9000 - (map(tempPercentage,SUBMENUMINVAL,SUBMENUMAXVAL,1,100) * ((9000-1000)/100));
  //Serial.print("readyVal: ");
  //Serial.println(readyVal);
  int temp = readyVal / 100;
  //Serial.print("TemperatureBeforeCalc: ");
  //Serial.println(temp);
  float r, g, b;

  if ( temp <= 66 ) {
    r = 255;
    g = temp;
    g = 99.4708025861 * log(g) - 161.1195681661;
    if ( temp <= 19) {
      b = 0;
      } else {
        b = temp - 10;
        b = 138.5177312231 * log(b) - 305.0447927307;
      }
      } else {
        r = temp - 60;
        r = 329.698727446 * pow(r, -0.1332047592);
        g = temp - 60;
        g = 288.1221695283 * pow(g, -0.0755148492 );
        b = 255;
      }
        rgbVals[RED] = int(r);
        rgbVals[GREEN] = int(g);
        rgbVals[BLUE] = int(b);
        updateIntensity(submenuVals[INTENSITY]);
        //setPixelColor(stripUpPin,rgbVals[0],rgbVals[1],rgbVals[2]);
        Serial.print("Updated Color: ");
        Serial.println(rgbVals[RED]);
        Serial.println(rgbVals[GREEN]);
        Serial.println(rgbVals[BLUE]);
}
void updateMenuColor(int tempPercentage){
  int readyVal = 3000 - tempPercentage * ((2500)/100);
  //Serial.print("MenureadyVal: ");
  //Serial.println(readyVal);
  int temp = readyVal / 100;
  //Serial.print("TemperatureBeforeCalc: ");
  //Serial.println(temp);
  float r, g, b;

  if ( temp <= 66 ) {
    r = 255;
    g = temp;
    g = 99.4708025861 * log(g) - 161.1195681661;
    if ( temp <= 19) {
      b = 0;
      } else {
        b = temp - 10;
        b = 138.5177312231 * log(b) - 305.0447927307;
      }
      } else {
        r = temp - 60;
        r = 329.698727446 * pow(r, -0.1332047592);
        g = temp - 60;
        g = 288.1221695283 * pow(g, -0.0755148492 );
        b = 255;
      }

        colorMenuVals[0] = int(r);
        colorMenuVals[1] = int(g);
        colorMenuVals[2] = int(b);
}
/*DESCRIPTION
This function takes a value, and uses that to determine where on a rainbow
the color should be.
*/
void fiddleFunction(int val) {
        int value = map(val,0,100,0,stripUp.numPixels());
        //Serial.print("Value for fiddleFunction: ");
        //Serial.println(value);
        for(int i=0; i< stripUp.numPixels(); i++) {
          stripUp.setPixelColor(i, Wheel(((value * 256 / stripUp.numPixels()) + value) & 255));
        }
        stripUp.show();

        Serial.print("Fiddle value: ");
        Serial.println(submenuVals[FIDDLE]);

}
/*DESCRIPTION
The area is indicated in the device as a growing amount of light shining in
the device, thus this function lights up the LED ring as much as the tolerance
has been set.
*/
void updateArea(int areaVal){
  setPixelColor(stripUpPin,0,0,0);
  for(int i = 0; i < areaVal; i++){
    stripUp.setPixelColor(NUMPIXSTRIPUP - i, stripUp.Color(0,200,0));
  }
  stripUp.show();
}
/*DESCRIPTION
The tolerance follows the same principle as Area, but with yellow light
*/
void updateTolerance(int tolerance){
  setPixelColor(stripUpPin,0,0,0);
  for(int i = 0; i < tolerance; i++){
    stripUp.setPixelColor(NUMPIXSTRIPUP - i, stripUp.Color(200,0,200));
  }
  stripUp.show();
}
//updates the look of the dial during presetmenu
void updatePreset(){
  updateColor(submenuVals[PRESETCOLOR]);
  updateIntensity(submenuVals[PRESETINTENSITY]);
  if(listOfSubmenuBools[PRESET]){
    submenuVals[INTENSITY] = submenuVals[PRESETINTENSITY];
    submenuVals[COLOR] = submenuVals[PRESETCOLOR];
    //Serial.println("Changes the values since the preset was entered");
  }
  //Serial.println("should update stripUp");
}
//Saves the current light to the preset for future application.
void savePreset(){
  submenuVals[PRESETINTENSITY] = submenuVals[INTENSITY];
  submenuVals[PRESETCOLOR] = submenuVals[COLOR];
  Serial.print("Values of preset are: ");
  Serial.print(submenuVals[PRESETINTENSITY]);
  Serial.print("   ,   ");
  Serial.println(submenuVals[PRESETCOLOR]);
  acceptPresetBlinks();
}

//Utility functions called by the other functions on demand
/*DESCRIPTION
This function gets the pixelring to change and the colous to apply, then
it applies it to the correct one
*/
void setPixelColor(int stripNr, int r, int g, int b){
      if(stripNr == stripRingPin){
        for(int i = 0; i<NUMPIXRing;i++){
          stripRing.setPixelColor(i,stripRing.Color(r,g,b));
        }
        stripRing.show();

        }else if(stripNr == stripUpPin){
          for(int i = 0; i < stripUp.numPixels();i++){
            stripUp.setPixelColor(i,stripUp.Color(r,g,b));
          }
          stripUp.show();

        }
}
void resetStrip(int stripNr){
  if(stripNr == stripRingPin){
    for(int i = 0; i<NUMPIXRing;i++){
      stripRing.setPixelColor(i,stripRing.Color(0,0,0));
    }
    stripRing.show();

  }else if(stripNr == stripUpPin){
    for(int i = 0; i < stripUp.numPixels();i++){
      stripUp.setPixelColor(i,stripUp.Color(0,0,0));
    }
  stripUp.show();

  }
}

/*DESCRIPTION
This function quickly checks if the pixelrings are responsive and working.
*/
void checkHardware(){
  setPixelColor(stripRingPin,0,150,0);
  stripRing.show();
  delay(1000);
  setPixelColor(stripRingPin,0,0,0);
  stripRing.show();
  setPixelColor(stripUpPin,0,150,0);
  stripUp.show();
  delay(1000);
  setPixelColor(stripUpPin,0,0,0);
  stripUp.show();
}
/*DESCRIPTION
Prints out a lot of values, use with care as it makes the rotary encoder
bad, since it has a delay.
*/
void printStatus(){
  Serial.print("MenuBool: ");
  Serial.println(menu);
  Serial.print(" IntensityBool: ");
  Serial.println(listOfSubmenuBools[INTENSITY]);
  Serial.print(" ColorBool: ");
  Serial.println(listOfSubmenuBools[COLOR]);
  Serial.print(" FIDDLEbool: ");
  Serial.println(listOfSubmenuBools[FIDDLE]);
  delay(100);
}
/*DESCRIPTION
Based on the given value the function is a specific integer value itself.
*/
uint32_t Wheel(byte WheelPos) {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85) {
          return stripUp.Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        if(WheelPos < 170) {
          WheelPos -= 85;
          return stripUp.Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        WheelPos -= 170;
        return stripUp.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

/*DESCRIPTION
This function applies the light to the ceiling using UDP and the following:
IP: 192.168.1.151
PORT: 11000


This needs to be developed - most likely a need to branch into a colour, a temp
and an intensity function.
*/

void sendTemperature(){
    Serial.println("temperature sent to ceiling");

    sendToCeilingBlinks();
    char identifier[] = "setCT&";
    Udp.beginPacket(remoteIPPacketSender,processingPort);
    Udp.write(identifier);
    int tempKelvin = map(submenuVals[1],0,100,9000,2700);
    Udp.write(String(tempKelvin));
    Udp.endPacket();
    delay(2000);
}
void sendIntensity(){
  Serial.println("intensity sent to ceiling");

  sendToCeilingBlinks();
  char identifier[] = "setDimLevel&";
  Udp.beginPacket(remoteIPPacketSender,processingPort);
  Udp.write(identifier);
  Udp.write(String(map(submenuVals[0],0,100,0,65535)));
  Udp.endPacket();
  delay(2000);
}
void sendAdjustedArea(){
  Serial.println("area sent to ceiling");

  sendToCeilingBlinks();
  char identifier[] = "areaSize&";
  Udp.beginPacket(remoteIPPacketSender,processingPort);
  Udp.write(identifier);
  Udp.write(String(submenuVals[3]));
  Udp.endPacket();
  delay(2000);
}
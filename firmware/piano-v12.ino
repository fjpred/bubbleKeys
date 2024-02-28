// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define LED_PIN    6
#define LED_COUNT 50


// OLED SSD1306 Screen
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Fonts/Picopixel.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);





// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)


// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4
#define PB 5

// Counters and button states
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;
unsigned long lastpButtonPress = 0;
int notesIndex[7] = {0, 2, 3, 5, 7, 8, 10};
int acc_count = 0;
int sc_count = 0;
int outVal = 2;

// Notes and scale names
/*String notes[7] = {"A", "B", "C", "D", "E", "F", "G"};
String accidental[3] = {"b", " ", "#"};*/
String scale_type[2][5] = {
                            {"Major", "Natural", "Harmonic", "Melodic", "Major"},
                            {"", "Minor", "Minor", "Minor", "Pentatonic"}
                           };


// setup() function -- runs once at startup --------------------------------

void setup() {
 
  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);
  pinMode(PB, INPUT_PULLUP);
  
  strip.begin();              // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();               // Turn OFF all pixels ASAP
  strip.setBrightness(225);    // Set BRIGHTNESS to about 1/2 (max = 255)
  Serial.begin(9600);         //initialize serial monitor

  lastStateCLK = digitalRead(CLK);    // Read the initial state of CLK

  displayLED(0,0);              // Turn on first scale


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  
  oledText(counter, acc_count, sc_count);

}


// loop() function -- runs repeatedly as long as board is on ---------------

void loop() {
  currentStateCLK = digitalRead(CLK);       // Read the current state of CLK

  // If last and current state of CLK are different, then pulse occurred. React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter --;
      outVal = notesIndex[counter] + acc_count;
      if (outVal == -1){
        outVal = 11;  //restart at end, no negative values
      }
      
      currentDir ="CCW";
      displayLED(outVal,sc_count);
    } else {       // Encoder is rotating CW so increment
      counter ++;

      outVal = notesIndex[counter] + acc_count;
      if (outVal == -1){
        outVal = 11;
      }
      
      currentDir ="CW";
      displayLED(outVal,sc_count);
    }

    // restrict note counter to 0 to 6
    if (counter > 6){
      counter = 0;
    } else if (counter <0){
      counter = 6;
    }

    // print to OLED display
    oledText(counter, acc_count, sc_count);

    // print to serial window
    Serial.print("Direction: ");
    Serial.print(currentDir);
    Serial.print(" | Counter: ");
    Serial.println(outVal);
  }


  lastStateCLK = currentStateCLK;        // Remember last CLK state
  int btnState = digitalRead(SW);        // Read the rot enc button state

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      acc_count++;
      if (acc_count > 1){
        acc_count = -1;
      }
      outVal = notesIndex[counter] + acc_count;
      if (outVal == -1){
        outVal = 11;
      }

      displayLED(outVal,sc_count);
      Serial.print("Accidental - ");
      Serial.println(acc_count);


      oledText(counter, acc_count, sc_count);
    }


    lastButtonPress = millis();     // Remember last button press event

  }


  int pbtnState = digitalRead(PB);   // Read the push button state

  //If we detect LOW signal, button is pressed
  if (pbtnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastpButtonPress > 50) {
      sc_count++;
      if (sc_count > 4){
        sc_count = 0;
      }
      
      displayLED(outVal,sc_count);          //update lights, turn on keyboard
      Serial.println("print at button - " + scale_type[0][sc_count] + " " + scale_type[1][sc_count]);
      
      oledText(counter, acc_count, sc_count);
    }

    // Remember last button press event
    lastpButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  delay(1);

}


int displayLED(int begin, int scale){
    
    int startSpace = 0;
    int octaves = 2;      //restrict for LPK25
    int totalNotes = (octaves * 12) + 1;

    int keys[totalNotes] = {0};
    int keyboard[totalNotes] = {0};
    int start = begin + 9;  // correct for keyboard starting at c

    // scales ___________________________________________________
    int scaleCall[5][12] = {
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},   /* 1. major scale */
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0},   /* 2. minor scale */
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1},   /* 3. harmonic minor scale */
        {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},   /* 4. melodic minor scale* */
        {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0},   /* 5. major pentatonic scale */
        /*{1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0},   6. minor pentatonic scale
        {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0},    7. dorian mode
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0}   8. mixolydian mode */
        
    };

    int majorScale [12];
    memcpy(majorScale, scaleCall[0], sizeof(majorScale));
    // __________________________________________________________
    
    
    // set notes for on ___________________________________________________
    int x = 0;

    strip.clear();       //clear strip
    
    for (int i = 0; i < totalNotes; i++){
      if(i == 35 or i == 43 or i == 49) {
          continue;
        } else{
           if(x < start){
              if (scaleCall[scale][(12 -((start - x) % 12))] == 1){
                  strip.setPixelColor(i + startSpace, random(0, 255), random(0, 255), random(0, 255));
              }else{
                  strip.setPixelColor(i + startSpace, 0, 0, 0);
              }
          }else{
              if (scaleCall[scale][(x - start) % 12] == 1){
                strip.setPixelColor(i + startSpace, random(0, 255), random(0, 255), random(0, 255));
            } else{
                strip.setPixelColor(i + startSpace, 0, 0, 0);
            }
      }
      x++;
      }
    }

    strip.show();

}

int oledText(int notee, int acci, int scalee){
// Notes and scale names
char onotes[7] = {'A', 'B', 'C', 'D', 'E', 'F', 'G'};
String oacc[3] = {"b", " ", "#"};
String oscl[2][5] = {
                            {"Major", "Natural", "Harmonic", "Melodic", "Major"},
                            {"", "Minor", "Minor", "Minor", "Pentatonic"}
                           };


String sc1 = oscl[0][scalee];
String sc2 = oscl[1][scalee];


acci = acci + 1;
  
  display.clearDisplay();    //clear display
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  
  display.setFont(&FreeSansBold12pt7b);
  display.setCursor(0, 32);
  display.println(onotes[notee]);
  Serial.println("disp print 1 - " + onotes[notee]);

  display.setFont(&FreeMono9pt7b);
  display.setCursor(35, 30);
  display.println(oacc[acci]);
  Serial.println("disp print 2 - " /*+ oacc[acci]*/);
 
  display.setFont(&Picopixel);
  display.setCursor(64, 12);
  display.println(sc1);
  display.setCursor(64, 26);
  display.println(sc2);
  Serial.println("disp print 3 - " + sc1 + " " + sc2);  
  
  display.display();      // Show initial text
}

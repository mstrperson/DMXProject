#include <Adafruit_NeoPixel.h>
#include <Thread.h>
#include <DMXSerial.h>

// Comment/Uncomment this line for DEBUG mode
#define DEBUG 1

// Change this to match the DMX Address Number printed on the side of the Arduino box.
#define DMX_ADDRESS 501

// 0-255 the value from the Lightboard which triggers the WHOOSH state
#define WHOOSH_THRESHOLD 128

// NeoPixels are wired to Pin 6.
#define PIN 6

// Number of Neopixels on the chain.  (currently 3px * 3 candles)
#define NUMPIXELS 9

// define the NeoPixel object.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);


// Some constants you can use to tweak the hue of the candles.
#define RED_MIN 180
#define RED_MAX 255
#define YELLOW_MAX 30

// Loop count (affects the timing of the random flickers)
int loopCount = 0;

// the current state of the system.  Default is Steady State, flickering candles.
int currentState = 0;

// Define the Constant values for each of the system States.
#define STEADY_STATE 0
#define WHOOSH_STATE 1
#define RESET_STATE 2
// 0 : Steady
// 1 : WHOOSH
// 2 : Reset

// keep track of the last Value read on the DMX Serial connection.
int lastReadValue = 0;

// Random flickers in the candle, random occurrance in the steady state.
void doFlicker()
{
  for(int i = 0; i < 20; i++)
  {
    pixels.setBrightness(random(50, 75));
    for(int j = 0; j < NUMPIXELS; j++)
    {
      pixels.setPixelColor(j, pixels.Color(random(RED_MIN, RED_MAX), 20, 0));
    }
    pixels.show();
    delay(20);
  }
}

// This is the steady state candle setting.  Each third LED has a particular behavior pattern.
void candle(int n)
{
  if(n%3==0)
  {
    pixels.setPixelColor(n, pixels.Color(255, 20, 0));
  }
  else if(n%3==1)
  {
    int yellow = random(YELLOW_MAX);
    pixels.setPixelColor(n, pixels.Color(255, yellow, 0));
  }
  else
  {
    int red = random(RED_MIN, RED_MAX);
    int yellow = random(YELLOW_MAX);
  
    pixels.setPixelColor(n, pixels.Color(red, yellow, 0));
  }
}

void setup() {
  // put your setup code here, to run once:
  pixels.begin();

  Serial.begin(115200);
  DMXSerial.init(DMXReceiver);
  
  DMXSerial.write(1, 80);
  DMXSerial.write(2, 0);
  DMXSerial.write(3, 0);
  
}

// Steady state for the system.
void steadyState()
{
  if(loopCount++ % 5 == 0 && random(100) > 50)
  {
    pixels.setBrightness(random(128, 255));
  }
  
  // put your main code here, to run repeatedly:
  for(int i=0; i<NUMPIXELS; i++) 
  {
    candle(i);  
  }
  pixels.show();

  if(random(100) > 5)
  {
    doFlicker();
  }
}

// When the DMX instructions go HIGH, sweep candles off from first to last.
void candleWhooshState()
{
  pixels.setBrightness(255);
  pixels.show();
  delay(250);  
  for(int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();

// ****** CHANGE THIS DELAY TO ALTER THE TIMING OF THE WHOOSH *****
    delay(100);
  }
}

// Going back from the WHOOSH to steady state.  Re-light the LEDS from first to last
void resetState()
{
  pixels.setBrightness(255);
  for(int i = 0; i < NUMPIXELS; i++)
  {
    candle(i);
    pixels.show();
// ****** CHANGE THIS DELAY TO ALTER THE TIMING OF THE WHOOSH *****
    delay(100);
  }
  currentState = STEADY_STATE;
}

void loop() 
{
  // Check to make sure the DMX is connected...
  if(DMXSerial.noDataSince() < 5000)
  {
    
#ifdef DEBUG
    // Debug, show a blue LED if we are debugging.
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();
    delay(100);
#endif

    // read the latest value from the DMX Serial connection
    int newVal = DMXSerial.read(DMX_ADDRESS);

    // If we have transitioned from LOW to HIGH...
    if(newVal > WHOOSH_THRESHOLD && lastReadValue < WHOOSH_THRESHOLD)
    {
      // go to the WHOOSH state.
      currentState = WHOOSH_STATE;
    }
    // Otherwise, if we have gone from HIGH to LOW...
    else if(newVal < WHOOSH_THRESHOLD && lastReadValue > WHOOSH_THRESHOLD)
    {
      // go to the RESET state.
      currentState = RESET_STATE;
    }

    // keep track of where we are!~
    lastReadValue = newVal;
  }
#ifdef DEBUG
  // if we are debugging, flash the first LED Green if there is no connection.
  else
  {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    delay(100);
  }
#endif

  // Execute the current state of the system
  if(currentState == WHOOSH_STATE)
  {
    candleWhooshState();
  }
  else if(currentState == RESET_STATE)
  {
    resetState();
  }
  else
  {
    steadyState();
  }
  
  delay(50);
}

#include <Adafruit_NeoPixel.h>
#include <Thread.h>
#include <DMXSerial.h>

#define DMX_ADDRESS 501

#define WHOOSH_THRESHOLD 128

#define PIN 6
#define NUMPIXELS 9

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

#define RED_MIN 180
#define RED_MAX 255
#define YELLOW_MAX 30

int loopCount = 0;

bool whooshState = false;

int lastReadValue = 0;
//bool connectionEstablished = false;

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

void candleWhooshState()
{
  pixels.setBrightness(255);
  pixels.show();
  delay(250);  
  for(int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
    delay(100);
  }
}

void loop() 
{
  if(DMXSerial.receive())
  {
    int newVal = DMXSerial.read(DMX_ADDRESS);
    if(newVal > WHOOSH_THRESHOLD && lastReadValue < WHOOSH_THRESHOLD)
    {
      whooshState = true;
    }
    else
    {
      whooshState = false;
    }
  }
  if(whooshState)
  {
    candleWhooshState();
  }
  else
  {
    steadyState();
  }
  
  delay(50);
}

//////////////////////////////////////////////////
//                                              //
// FHT Led Matrix Audio Visualisation Project   //
// by Ville Saarinen                            //
//                                              //
// The used platform is Arduino Nano            //
// Microchip: AtMega328P                        //
//                                              //
// This project is about RGB led vizulizer      //
// It uses FHT to obtain audio data from AUX    //
// This is supposed to be integrated to         //
// a larger audio system                        //
//                                              //
//////////////////////////////////////////////////

// BUGS: High frequency voices show also in lower bands 
// (duplicates to there)

#include <FHT.h>
#include <Adafruit_NeoPixel.h>

#define LOG_OUT 1 // Use the log output function
#define FHT_N 256 // Set to 256 point fht

#define WIDTH 8 // Columns
#define HEIGHT 8 // Rows
#define SKIP_BANDS 15 // Skips first frequencies
#define GREEN_SPEED 0.03 // What rate does greens go down

#define LED_PIN 10 // Led matrix control pin

int magnitudes[WIDTH] = {}; // Magnitude for each band
int red_level[WIDTH] = {}; // Levels for reds
double green_level[WIDTH] = {}; // Levels for greens
double last_green[WIDTH] = {}; // Last rounds green leds

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(WIDTH * HEIGHT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  
  TIMSK0 = 0; // Turn off timer0 for lower jitter

  ADCSRA |= (1 << ADEN); // Enable adc

  ADCSRA &= ~(1 << ADIE); // Disable adc conversion complete interrupt

  ADCSRA &= ~(1 << ADATE); // Set single conversation mode

  // Set adc prescaler to 16
  ADCSRA |= (1 << ADPS2); // Sets ADPS2
  ADCSRA &= ~(B00000011); // Clears ADPS1 and ADPS0
  
  ADMUX |= (1 << REFS0); // Set the Vref for adc
  
  DIDR0 |= (1 << ADC0D); // Turn off the digital input for adc0

  pinMode(LED_PIN, OUTPUT); // Led matrix

  strip.begin();

  strip.setBrightness(20); // Sets the brightness
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  while(1) { // reduces jitter
    
    cli();  // UDRE interrupt slows this way down on arduino1.0
    
    ADMUX &= ~(B00001111); // Sets adc0
    
    for (int i = 0 ; i < FHT_N ; ++i) { // Save 256 samples
      while(ADCSRA & B01000000);
      
      ADCSRA |= (1 << ADSC); // Start conversation
      
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    
    sei();

    strip.clear();
    
    countMagnitudes();

    for(int i = 0; i < WIDTH; ++i) {
      Serial.print( magnitudes[ i ] );
      Serial.print(" ");
    }
    Serial.println(" ");

    countRed();

    countGreen();

    setLeds();
    
    strip.show();
  }
}

void countMagnitudes() {
  int band_width = int((79 - SKIP_BANDS)/WIDTH); // Calculate band width
  int result = 0;
  for(int i = 0; i < WIDTH; ++i) {
    for(int j = 0; j < band_width; ++j) {
      result = result + fht_log_out[SKIP_BANDS + i * band_width + j];
    }
    result = result/band_width;
    result = map(result, 0, 120, 0, 8);
    result = constrain(result, 0, 8);
    magnitudes[i] = result;
    result = 0;
  }
}

void countRed() {
  for(int i = 0; i < WIDTH; ++i) {
    red_level[i] = magnitudes[i] - 1;
  }
}

void countGreen() {
  for(int i = 0; i < WIDTH; ++i) {
    if(magnitudes[i] < last_green[i]) {
      green_level[i] = last_green[i] - GREEN_SPEED; 
    } else {
      green_level[i] = magnitudes[i];
    }
    green_level[i] = constrain(green_level[i], 1, 8);
    last_green[i] = green_level[i];
  }
}

void setLeds() {

  // Setting red ones
  for(int i = 0; i < WIDTH; ++i) {
    for(int j = 0; j < red_level[i]; ++j) {
      if((i % 2) == 0) {
        strip.setPixelColor(HEIGHT * i + j, 255, 0, 0); // Red
      } else {
        strip.setPixelColor(HEIGHT * i + HEIGHT - j - 1, 255, 0, 0); // Red
      }
    }

    if((i % 2) == 0) {
        strip.setPixelColor(int(HEIGHT * i + green_level[i] - 1), 0, 255, 0); // Green
      } else {
        strip.setPixelColor(int(HEIGHT * i + HEIGHT - green_level[i]), 0, 255, 0); // Green
      }
  }
}

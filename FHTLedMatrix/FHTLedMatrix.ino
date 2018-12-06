#define LOG_OUT 1 // Use the log output function
#define FHT_N 256 // Set to 256 point fht
#define WIDTH 8 // Columns
#define HEIGHT 8 // Rows

#define LED_PIN 10 // Led matrix

int magnitudes[8] = [];

#include <FHT.h>
#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(WIDTH * HEIGHT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
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

    drawBase();
    strip.show();
  }
}

void drawBase() {
  for(int i = 0; i < WIDTH; ++i) {
    if((i % 2) == 0) {
      strip.setPixelColor(HEIGHT * i, 0, 255, 0); // Red
    } else {
      strip.setPixelColor(HEIGHT * i + HEIGHT - 1, 0, 255, 0); // Red
    }
  }
}

void countMagnitudes() {
  int band = int(fht_log_out.size()/WIDTH); // Calculate band width
  for(int i = 0; i < WIDTH; ++i) {
    // TODO Calculate
  }
}

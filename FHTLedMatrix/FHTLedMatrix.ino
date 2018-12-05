#define LOG_OUT 1 // Use the log output function
#define FHT_N 256 // Set to 256 point fht

#include <FHT.h> // Include the library

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
}

void loop() {
  while(1) { // reduces jitter
    
    cli();  // UDRE interrupt slows this way down on arduino1.0

    ADMUX &=  ~(B00001111); // Sets adc0
    
    for (int i = 0 ; i < FHT_N ; ++i) { // Save 256 samples
      
      while(!(ADCSRA & B00010000)); // Wait for adc to be ready
      
      ADCSRA |= (1 << ADSC); // Start covnersation
      
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
  }
}

#ifndef SIMONIIR_INO_
#define SIMONIIR_INO_
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// a goes to DEN
// b goes to NUM

#include "blue_high_348hz.h"
#include "blue_low_90hz.h"
#include "green_high_422hz.h"
#include "green_low_279hz.h"
#include "red_329hz.h"
#include "yellow_210hz.h"
#include "lowpass1_5k.h"
#include "highpass1_5k.h"

#include "red_reallylow.h"

#define CHECK_COLOR
//#define OUTPUT_SAMPLES
#define CONTINUOUS_SAMPLING

#define NO_PRESCALING 0x01
#define PRESCALE_8    0x02
#define PRESCALE_64   0x03

#define SAMPLE_OFFSET 510
#define CTC_MATCH 500 //2000//*should* run the interrupt at 1kHz
//37 works when speaker is aimed at mic
#define BUCKET_OFFSET 25
#define BUCKET_NORMALIZE 3

#define NUM_SAMPLES 512
#define SAMPLE_PIN  A0
#define SAMPLE_MOD 4

#define AVG_SAMPLES 10

const float AVG_CONST = 1.0/AVG_SAMPLES;
const int nb = 9;

enum colors_t { RED, BLUE, YELLOW, GREEN };

unsigned long int last_time;

int sampled_signal[NUM_SAMPLES] = {0};
volatile bool  sample_flag = false;
volatile long delta_t = 0;
volatile int current_sample = 0;

float x[9] = {0};
float y_blue_low[9] = {0};
float y_blue_high[9] = {0};
float y_green_low[9] = {0};
float y_green_high[9] = {0};
float y_red[9] = {0};
float y_yellow[9] = {0};

unsigned long int blue_bucket = 0;
unsigned long int blue_bucket_high = 0;
unsigned long int green_bucket = 0;
unsigned long int green_bucket_high = 0;
unsigned long int red_bucket = 0;
unsigned long int yellow_bucket = 0;

float in = 0;

//b is numerator a is denominator.
void IIR(float *x, float *y, const float *b, short nb, const float *a, short na);
colors_t sort(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d);

//interrupt handler for the timer compare
ISR(TIMER2_COMPA_vect) {
  static unsigned int index = 0; //circular buffer index
  char count = 0; //count to get every xth sample
  //holds both high and low byte of adc, adcl must be read first and adch next
  unsigned char high = 0, low = 0;

  float low_filter[4] = {0};
  float high_filter[4] = {0}; //output of low and high
  float sample_in[4] = {0};
  float low_bucket = 0;
  float high_bucket = 0;

  static long start_time = 0;

  start_time = micros();

  if (!sample_flag) { //don't do it when we have a 'hit'
    // read the adc
    ADMUX = 0x40;
    sbi(ADCSRA, ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    low = ADCL;
    high = ADCH;
    current_sample = ((high << 8) | low) - SAMPLE_OFFSET;

    for(int i = 3; i>0; i--) //shift in the sampled data
      sample_in[i] = sample_in[i-1];

    sample_in[0] = current_sample;

    //run the filters
    IIR(sample_in, high_filter, HIGH_NUM, HIGH_NL, HIGH_DEN, HIGH_DL);
    IIR(sample_in, low_filter, LOW_NUM, LOW_NL, LOW_DEN, LOW_DL);

    high_bucket = AVG_CONST*high_filter[0]*high_filter[0] + (1 - AVG_CONST)*high_bucket;
    low_bucket = AVG_CONST*low_filter[0]*low_filter[0] + (1 - AVG_CONST)*low_bucket;

    if (low_bucket/BUCKET_NORMALIZE > high_bucket + BUCKET_OFFSET) {
      sample_flag = true;
    }
    if (count == 0) { //take every xth sample
      sampled_signal[index] = current_sample;
      if (++index == NUM_SAMPLES)
        index = 0;
    }
  }

  if (++count == 4)
    count = 0;
  delta_t = micros() - start_time;
} //end interrupt handler

void sense_color_init(){
  noInterrupts();

  //configure the timer interrupt
  TCCR2A = 0;
  TCCR2B = PRESCALE_8; //sets the prescaler
  TCNT2  = 0;             //resets the timer

  OCR2A = CTC_MATCH;
  TCCR2B |= (0x01 << WGM21);  //enables CTC mode
  TIMSK2 |= (0x01 << OCIE2A); //enables the interrupt CTC interrupt

  ADCSRA = 0x87;
  Serial.print("sense_color_init");
  interrupts();
}

colors_t sense_color(){
  colors_t color;
  blue_bucket   = 0;
  green_bucket  = 0;
  red_bucket    = 0;
  yellow_bucket = 0;

#ifndef CONTINUOUS_SAMPLING
  while(!Serial.available());
#endif

  sample_flag = true;
  while(sample_flag); //pause execution while sampling

#ifdef OUTPUT_SAMPLES
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    Serial.print(i);
    Serial.print("\t");
    Serial.print(sampled_signal[i]);
    Serial.print("\n");
  }

  Serial.print("----------------\n");
#endif

  int j = 0;
  while(j < NUM_SAMPLES){
    in = (float)sampled_signal[j];
    for(int i = nb-1;i>0;i--) //shift in the sampled data
      x[i] = x[i-1];

    x[0] = in;

    //run the data through the filters
    //IIR(x, y_blue_low, BLUE_LOW_NUM, BLUE_LOW_NL, BLUE_LOW_DEN, BLUE_LOW_DL);
    IIR(x, y_blue_high, BLUE_HIGH_NUM, BLUE_HIGH_NL, BLUE_HIGH_DEN, BLUE_HIGH_DL);
    IIR(x, y_green_high, GREEN_HIGH_NUM, GREEN_HIGH_NL, GREEN_HIGH_DEN, GREEN_HIGH_DL);
    IIR(x, y_red, RED_LOW_NUM, RED_LOW_NL, RED_LOW_DEN, RED_LOW_DL);
    IIR(x, y_yellow, YELLOW_NUM, YELLOW_NL, YELLOW_DEN, YELLOW_DL);

    //get the energy out of each of the filters
    //blue_bucket   += y_blue_low[0] * y_blue_low[0];
    blue_bucket   += y_blue_high[0] * y_blue_high[0];
    red_bucket    += y_red[0] * y_red[0];
    yellow_bucket += y_yellow[0] * y_yellow[0];
    green_bucket  += y_green_high[0] * y_green_high[0];

#ifdef OUTPUT_SAMPLES
    Serial.print(j);
    Serial.print("\t");
    Serial.print(y_red[0]);
    Serial.print("\t");
    Serial.print(y_yellow[0]);
    Serial.print("\t");
    Serial.print(y_blue_low[0]);
    Serial.print("\t");
    Serial.print(y_blue_high[0]);
    Serial.print("\t");
    Serial.print(y_green_low[0]);
    Serial.print("\t");
    Serial.print(y_green_high[0]);
    Serial.print("\n");
#endif

    j++;
  }

  //see which energy is highest
  color = sort(red_bucket,blue_bucket,yellow_bucket,green_bucket);

#ifdef CHECK_COLOR
  Serial.print("Red: ");
  Serial.println(red_bucket);
  Serial.print("Blue: ");
  Serial.println(blue_bucket);
  Serial.print("Green: ");
  Serial.println(green_bucket);
  Serial.print("Yellow: ");
  Serial.println(yellow_bucket);

  switch(color) {
    case RED:
      Serial.println("Red");
      break;
    case BLUE:
      Serial.println("Blue");
      break;
    case YELLOW:
      Serial.println("Yellow");
      break;
    case GREEN:
      Serial.println("Green");
      break;
  }
#endif


#ifndef CONTINUOUS_SAMPLING
  while(Serial.available()) Serial.read();
#endif
  return color;
} //end sense_color()

//b is numerator a is denominator.
void IIR(float *x, float *y, const float *b, short nb, const float *a, short na)
{
  double z1,z2;
  short i;

  for(z1=0,i=0;i<nb;i++)
    z1+=x[i]*b[i];

  for(i=na-1;i>0;i--)
    y[i]=y[i-1];

  for(z2=0,i=1;i<na;i++)
    z2+=y[i]*a[i];

  y[0]=(z1-z2);
}

colors_t sort(unsigned long int a,unsigned long int b,unsigned long int c,unsigned long int d){
  colors_t highest = RED;
  unsigned long int highest_num = 0;

  highest_num = a;
  if(b > highest_num){
    highest_num = b;
    highest = BLUE;
  }
  if(c > highest_num){
    highest_num = c;
    highest = YELLOW;
  }
  if(d > highest_num){
    highest_num = d;
    highest = GREEN;
  }

  return highest;
}
#endif

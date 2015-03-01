// a goes to DEN
// b goes to NUM

#define NO_PRESCALING 0x01
#define PRESCALE_8    0x02
#define PRESCALE_64   0x03

#define CTC_MATCH 2000 //*should* run the interrupt at 1kHz

#define NUM_SAMPLES 512
#define SAMPLE_PIN  A0

unsigned long int last_time;

const short na = 9;
const short nb = 9;

float x[9] = {0};
float y_blue[9] = {0};
float y_yellow[9] = {0};
float y_green[9] = {0};
float y_red[9] = {0};

unsigned int sampled_signal[NUM_SAMPLES] = {0};
volatile bool  sample_flag = false;

unsigned long int blue_bucket = 0;
unsigned long int red_bucket = 0;
unsigned long int yellow_bucket = 0;
unsigned long int green_bucket = 0;

float in = 0;
int color;

const float b_blue[9] = { //x coeff
  0.0005291156704,-0.0008445017156, 0.002166110324,-0.002284691436,  0.00326248887,
  -0.002284691436, 0.002166110324,-0.0008445017156,0.0005291156704
};
const float a_blue[9] = { //y coeffs
                1,   -1.767378688,    5.037234783,   -5.476717949,    7.929422379,
     -5.315847874,    4.745763302,   -1.616050601,   0.8875268102
};

const float b_yellow[9] = {
  0.0005034227506,0.0002466906153, 0.001598455361,0.0006247343263, 0.002243153052,
  0.0006247343263, 0.001598455361,0.0002466906153,0.0005034227506
};

const float a_yellow[9] = {
                1,    0.544231236,    3.976155758,     1.59006381,    5.834795952,
      1.543359637,    3.746161699,   0.4976285994,    0.887517333
};

const float b_red[9] = {
  0.0005793659366, 0.001973848324, 0.004431678448,  0.00662337523, 0.007720588706,
    0.00662337523, 0.004431678448, 0.001973848324,0.0005793659366
};

const float a_red[9] = {
                1,    3.753026247,     9.15087986,    14.20672417,    16.64303207,
      13.78934002,    8.621077538,     3.43173337,    0.887544632
};

const float b_green[9] = {
  0.0007984918775, 0.005124222022,  0.01531646121,  0.02786091156,  0.03374225274,
    0.02786091156,  0.01531646121, 0.005124222022,0.0007984918775
};

const float a_green[9] = {
                1,    6.952979088,    22.00747871,    41.24068069,    49.95503998,
       40.0291214,    20.73347855,    6.358115673,   0.8876139522
};

void IIR(float in, float *x, float *y, const float *b, short nb, const float *a, short na);
int sort(unsigned long int a, unsigned long int b, unsigned long int c, unsigned long int d);

//interrupt handler for the timer compare
ISR(TIMER1_COMPA_vect) {
  static unsigned int index = 0;
  if (sample_flag) {
    //Serial.println(index);
    sampled_signal[index++] = analogRead(SAMPLE_PIN);
    if (index >= NUM_SAMPLES) {
      index = 0;
      sample_flag = false;
    }
  }
} //end interrupt handler

void setup(){
  noInterrupts();
  Serial.begin(9600);

  //configure the timer interrupt
  TCCR1A = 0;
  TCCR1B = PRESCALE_8; //sets the prescaler to 8
  TCNT1  = 0;             //resets the timer

  OCR1A = CTC_MATCH;
  TCCR1B |= (0x01 << WGM12);  //enables CTC mode
  TIMSK1 |= (0x01 << OCIE1A); //enables the interrupt CTC interrupt
  
  interrupts();
}

void loop(){
  blue_bucket   =0;
  red_bucket    =0;
  yellow_bucket =0;
  green_bucket  =0;
  
  while(!Serial.available());
  sample_flag = true;
  while(sample_flag){
    //Serial.println(sample_flag);
  }
  //Serial.println("R");
  
  
  int j = 0;
  while(j < NUM_SAMPLES){
    in = (float)sampled_signal[j];
    for(int i=nb-1;i>0;i--)
      x[i]=x[i-1];
    
    x[0]=in;
    
    IIR(x,y_blue,b_blue,nb,a_blue,na);
    IIR(x,y_yellow,b_yellow,nb,a_yellow,na);
    IIR(x,y_green,b_green,nb,a_green,na);
    IIR(x,y_red,b_red,nb,a_red,na);
    
    blue_bucket   += y_blue[0] * y_blue[0];
    red_bucket    += y_red[0] * y_red[0];
    yellow_bucket += y_yellow[0] * y_yellow[0];
    green_bucket  += y_green[0] * y_green[0];
    
    //Serial.println(j);
    j++;
  }
  
  color = sort(blue_bucket,red_bucket,yellow_bucket,green_bucket);
  //Serial.print("Color: ");
  //Serial.println(color);
  
  Serial.print("Blue: ");
  Serial.println(blue_bucket);
  Serial.print("Red: ");
  Serial.println(red_bucket);
  Serial.print("Green: ");
  Serial.println(green_bucket);
  Serial.print("Yellow: ");
  Serial.println(yellow_bucket);
  
  if(color == 0){
    Serial.println("Blue");
  }
  else if(color == 1){
    Serial.println("Red");
  }
  else if(color == 2){
    Serial.println("Yellow");
  }
  else if(color == 3){
    Serial.println("Green");
  }
  
  //while(Serial.available()) Serial.read();
}

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

int sort(unsigned long int a,unsigned long int b,unsigned long int c,unsigned long int d){
  int highest = 0;
  int highest_num = 0;
  
  highest_num = a;
  if(b > highest_num){
    highest_num = b;
    highest = 1;
  }
  if(c > highest_num){
    highest_num = c;
    highest = 2;
  }
  if(d > highest_num){
    highest_num = d;
    highest = 3;
  }
  
  return highest;
}

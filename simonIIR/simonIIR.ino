// a goes to DEN
// b goes to NUM

unsigned long int last_time;

const short na = 9;
const short nb = 9;

float x[9] = {0};
float y_blue[9] = {0};
float y_yellow[9] = {0};
float y_green[9] = {0};
float y_red[9] = {0};

unsigned long long int blue_bucket = 0;
unsigned long long int red_bucket = 0;
unsigned long long int yellow_bucket = 0;
unsigned long long int green_bucket = 0;

float in = 5;

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

void setup(){
  Serial.begin(9600);
}

void loop(){  
  for(int i=nb-1;i>0;i--)
    x[i]=x[i-1];
  
  x[0]=in;
  
  IIR(in,x,y_blue,b_blue,nb,a_blue,na);
  IIR(in,x,y_yellow,b_yellow,nb,a_yellow,na);
  IIR(in,x,y_green,b_green,nb,a_green,na);
  IIR(in,x,y_red,b_red,nb,a_red,na);
  
  blue_bucket   += y_blue[0] * y_blue[0];
  red_bucket    += y_red[0] * y_red[0];
  yellow_bucket += y_yellow[0] * y_yellow[0];
  green_bucket  += y_green[0] * y_green[0];
  
  Serial.println(micros()-last_time);
  last_time = micros();
}

void IIR(float in, float *x, float *y, const float *b, short nb, const float *a, short na)
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

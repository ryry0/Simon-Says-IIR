

void setup(){
  Serial.begin(9600);
}

void loop(){
  
}

void IIR(short in, short *x, short *y, short *b, short nb, short *a, short na)
{
  long z1,z2,temp;
  short i;
  
  for(i=nb-1;i>0;i--)
    x[i]=x[i-1];
  
  x[0]=in;
  
  for(z1=0,i=0;i<nb;i++)
    z1+=(long)x[i]*b[i];
    
  for(i=na-1;i>0;i--)
    y[i]=y[i-1];
  
  for(z2=0,i=1;i<na;i++)
    z2+=(long)y[i]*a[i]
    
  z1=z1-z2;
  z1+=0x400;
  y[0]=(short)(z1-z2);
}
           

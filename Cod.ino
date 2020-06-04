void setup() {
  // put your setup code here, to run once:
  Timer0();
  Timer1();
  PiniMotoare();
  PiniPWM();
  PiniSonar();
  PiniSenzorLinie();
}
////////////////////////////////////////////////////////////////////////
//Variabile globale

int overflow=0;
double distance;
long count;
int viteza_nominala=200;
int kp=20;

////////////////////////////////////////////////////////////////////////
//TIMER0 pentru utilizarea motoarelor

void Timer0(){
  cli();
  TCCR0A=0;
  TCCR0B=0;
  TCCR0A|=(1<<WGM01)|(1<<WGM00)|(1<<COM0B1)|(1<<COM0A1); //setare fast pwm
  TCCR0B|=(1<<CS00); //fara prescaler
  sei();  
}
////////////////////////////////////////////////////////////////////////
//TIMER1 pentru calculul distantei de la sonar

void Timer1(){
  cli();
  TCCR1A=0;
  TIMSK1|=(1<<TOIE1); //activeaza intreruperea overflow
  sei();
}
////////////////////////////////////////////////////////////////////////
//Intreruperea de tip overflow

ISR(TIMER1_OVF_vect){
  overflow++;  /* Increment Timer Overflow count */
}

////////////////////////////////////////////////////////////////////////

void Timer2(){
  cli();
  TCCR2A=0;
  TCCR2B=0;
  TCCR2B|=(0 << CS22)|(1 << CS21)|(0 << CS20); //prescaler 8
  sei();
}
////////////////////////////////////////////////////////////////////////
//Setarea serialului pentru transmitere date + functie de transmitere seriala

void USART_Transmit(unsigned char data)
{
   while(!( UCSR0A & (1<<UDRE0))); 
   UDR0=data;
}

void SetareSerial(){
   //Setare serial
  UCSR0C=(1<<UCSZ00)|(1<<UCSZ01); //Numarul bitilor de date
  UCSR0B=(1<<RXEN0)|(1<<TXEN0); //Activez receptia si transmisia
  UCSR0A=(1<<UDRE0);
  UBRR0=103; //Baud rate 9600
  }

////////////////////////////////////////////////////////////////////////
//Functii delay

void Delay3s(){
  int durata=0;
  TCNT2=0;
  while(durata<=23529)//16MHz/8=2MHz; 3S/(0.5us*255)=23529
  { 
    if(TCNT2==255)
    {
      TCNT2=0;
      durata++;
    }
    TCNT2++;
  }
}

void Delay1s(){
  int durata=0;
  TCNT2=0;
  while(durata<=7843)//16MHz/8=2MHz; 1S/(0.5us*255)=7843
  { 
    if(TCNT2==255)
    {
      TCNT2=0;
      durata++;
    }
    TCNT2++;
  }
}

void Delay10ms(){
  int durata=0;
  TCNT2=0;
  while(durata<=392)//16MHz/8=2MHz; 10ms/(0.5us*255)=78
  { 
    if(TCNT2==255)
  {
    TCNT2=0;
    durata++;
    }
    TCNT2++;
  }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//Functie pentru trimiterea distantei pe serial
void TrimiteDist(int var)
{
  char buf[50];
  memset(buf,0,50);
  sprintf(buf,"Distanta este: %d cm\n", var);
  for(int i=0;i<strlen(buf);i++)
    USART_Transmit(buf[i]);
}

////////////////////////////////////////////////////////////////////////
//Setare pini

void PiniPWM(){ //D5,D6 -> ENB,ENA
  DDRD|=0x60;  
}

////////////////////////////////////////////////////////////////////////

void PiniMotoare(){ //IN1,IN2,IN3,IN4 -> B1,B2,B3,B4 | ENA,ENB -> D3,D4
  DDRB|=0x1E;
}

////////////////////////////////////////////////////////////////////////

void PiniSenzorLinie(){//A0,A1,A2,A3,A4
  DDRC|=0x1F;
}

////////////////////////////////////////////////////////////////////////

void PiniSonar(){ //D7,B0
  DDRD|=0x80;
  DDRB|=0x01;
}

////////////////////////////////////////////////////////////////////////
int SenzorLinie(){  
  int eroare=0;
  if(PINC&0X1F==0x01)
  { Dreapta();
    eroare=-2;
  }
  else if(PINC&0X1F==0x02)
  {
    Fata();
    eroare=-1;
  }
  else if(PINC&0X1F==0x04)
  {
    Fata();
    eroare=0;
  }
  else if(PINC&0X1F==0x08)
  {
    Fata();
    eroare=1;
  }
  else if(PINC&0X1F==0x10)
  {
    Stanga();
    eroare=2;
  }
  return eroare;
}
////////////////////////////////////////////////////////////////////////

void CalculPID()
{
  int eroare=0;
  int v1,v2;
  eroare=SenzorLinie();
  v1=viteza_nominala-kp*eroare;
  v2=viteza_nominala+kp*eroare;
}

////////////////////////////////////////////////////////////////////////
//Functii pentru motoare

void fata(){
  OCR0A=150;
  OCR0B=150;
  PORTB&=~0x1E;
  PORTB|=0x14;
}

/*void Stop(){
  OCR0A=0;
  OCR0B=0;
  PORTB=&~0x1E;
  PORTD=&~0x18;
}*/

void spate(){
  OCR0A=120;
  OCR0B=120;
  PORTB&=~0x1E;
  PORTB|=0x0A;
}

void stanga(){
  OCR0A=10;
  OCR0B=120;
  PORTB&=~0x1E;
  PORTB|=0x10;
}

void dreapta(){
  OCR0A=120;
  OCR0B=10;
  PORTB&=~0x1E;
  PORTB|=0x04;
}
////////////////////////////////////////////////////////////////////////

void loop() {

while(1){
  PORTD&=~0x80; //Trigger pe D7
  Delay10ms();
  PORTD|=0x80;
  Delay10ms();
  PORTD&=~0x80;
  
  TCNT1=0;
  TCCR1B=(1<<ICES1)|(1<<CS10);
  TIFR1=1<<ICF1;
  TIFR1=1<<TOV1;
  
  while ((TIFR1 & (1 << ICF1)) == 1);// Wait for rising edge 
    {
      TCNT1 = 0;  /* Clear Timer counter */
      TCCR1B = 0x01;  /* Capture on falling edge, No prescaler */
      TIFR1 = 1<<ICF1; /* Clear ICP flag (Input Capture flag) */
      TIFR1 = 1<<TOV1; /* Clear Timer Overflow flag */
      TimerOverflow = 0;/* Clear Timer overflow count */
    }

  while ((TIFR1 & ~(1 << ICF1)) == 0);/* Wait for falling edge */
    {
      count = ICR1 + (65535 * overflow); /* Take count */
      /* 16MHz Timer freq, sound speed =343 m/s */
      distance = ((double)count / 932.94);
    }
    if(distance<40)
    {
      stanga();
      Delay1s();
      fata();
      Delay10ms();
      dreapta();
      Delay1s();
      fata();
    }
 }
}

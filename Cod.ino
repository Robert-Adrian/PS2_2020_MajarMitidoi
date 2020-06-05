#define VN 170
#define KP 15

int i=0;
int n = 0;                //contor overflow sonar
int distanta=0;
int oprire=0;

void timer1(){
  cli();                    //dezactiveaza toate intreruperile
  DDRD|=0x80;               //pin 7 output trig
  DDRB&=~0x01;              //pin 8 input echo  
  TCCR1A = 0;                             //curatare registrii
  TCCR1B = 0;                 
  TCCR1B |= (1 << CS10);                    //no prescaling
  sei();
}

void initializare_CAN()
{
  ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));//factor de divizare 128
  ADMUX |= (1 << REFS0);//setare tensiune de referinta
  ADCSRA |= (1 << ADEN);//Activare convertor
  ADCSRA |= (1 << ADSC);//start conversie CAN
}

void setare_color(){
  // pini 2 3 4 (pd2 pd3 pd4) pentru culoare rgb
  //2- red 
  //3-green
  //4 -blue
  DDRD|=0x1C; //setare pini output RGB
  // analog 5 (A5) PC5 pin input fotorezistor
  DDRC&=~0x20;
}

void setare_PID(){
  DDRC &= ~0X1F; // PINII A0-A4 INPUT BANDA LEDURI
}

void pwm() {
 cli();    //dezactiveaza toate intreruperile
  DDRB|=0x1E;            //pin 9,10,11,12 iesire pentru in1,2,3,4 directie motor
  DDRD|=0X60;            //pin 5,6 pwm output 
  TCCR0A=0;
  TCCR0B=0;                                        //curatare registrii
  TCCR0A |= (1 <<WGM01) | (1<<WGM00)|(1<<COM0B1) |(1<<COM0A1);  //setare fast pwm
  TCCR0B|=(1<<CS00);                               //selectare no prescaling
 // OCR0B=114;
  sei();
}

void timer2(){
  cli();
  TCCR2A=0;
  TCCR2B=0;
  TCCR2B |= (1<< CS20) | (1<<CS21)|(1<<CS22);     //PRESCALER 1024
  TCCR2A|=(1<<WGM21);       //MOD CTC
  TIMSK2 |=(1<<OCIE2A);         //ACTIVARE INTRERUPERE LA COMPARARE A;
  OCR2A=158;          //INTRERUPERE LA 10MS
  sei();
}


ISR(TIMER2_COMPA_vect){
  i++;
}

void fata(){
  PORTB &=~0X1E;
  PORTB |=0X0A;
  OCR0A=200;
  OCR0B=200;  
}

void spate(){
  PORTB &=~0X1E;
  PORTB |=0X14;
  OCR0A=200;
  OCR0B=200;
}

void stanga(){
  PORTB &=~0X1E;
  PORTB |=0X12;
  OCR0A=200;
  OCR0B=200;
}

void dreapta(){
  PORTB &=~0X1E;
  PORTB |=0X0C;
  OCR0A=200;
  OCR0B=200;
}

uint16_t citire_CAN()
{
  uint32_t Ncan=0;
  ADMUX |= 0x20;// citire de la pinul A5
  ADCSRA |= (1 << ADSC);//start conversie
  while(ADCSRA & (1 << ADSC));//urmarim si cand devine 0 => sfarsitul conversiei
  Ncan=ADC;
  return Ncan;
}

int track_sensor() {
  int eroare=0;
  if((PINC & 0x1B)==0){
    oprire =1;
  }
  if((PINC & 0x08)==0)       //S2(st->dr)
    eroare=1;
  else if((PINC & 0x02)==0)   //S4
          eroare=-1;
  else if((PINC & 0x10)==0)   //S1
          eroare=2;
  else if((PINC & 0x01)==0)   //S5
          eroare=-2;
  else if((PINC & 0x04)==0)   //S3
          eroare=0;
          
  return eroare;
}

void PID(){
  int e=track_sensor();
  if((PINC & 0x10)==0){  // senzor 5 activ viraj la stanga
     PORTB &=~0X1E;
     PORTB |=0X0A;
    OCR0A=VN + e*KP;
    OCR0B=VN - e*KP;
  }
  if((PINC & 0x08)==0) {  // senzor 4 activ viraj la stanga
      PORTB &=~0X1E;
      PORTB |=0X0A;
    OCR0A=VN + e*KP;
    OCR0B=VN - e*KP;
  }
  if((PINC & 0x04)==0) {   // senzor 3 activ in fata
    OCR0A=VN + e*KP;
    OCR0B=VN - e*KP;
  }
  if((PINC & 0x02)==0) {  // senzor 2 activ viraj la dreapta
     PORTB &=~0X1E;
     PORTB |=0X0A;
    OCR0A=VN + e*KP;
    OCR0B=VN - e*KP;
  }
  if((PINC & 0x01)==0) {  // senzor 1 activ viraj la dreapta
     PORTB &=~0X1E;
     PORTB |=0X0A;
    OCR0A=VN + e*KP;
    OCR0B=VN - e*KP;
  }
  if((PINC & 0x1B)==0){
    PORTB &=~0X1E;
  }
}

int culoare(){
  int cul_r=0,cul_y=0,cul_b=0;
  int w=0;
  
  PORTD|=0X04;
  delay(10000);
  
  cul_r=analogRead(A5);
  // cul_r=citire_CAN();
  //i=0;
  //while(i<5);//cat timp i=0 adica asteptam 10 ms
  
  PORTD&=~0X04;
  PORTD|=0X0C;
  delay(10000);
  
  cul_y=analogRead(A5);
 // cul_y=citire_CAN();
  //i=0;
  //while(i<10);//cat timp i=0 adica asteptam 10 ms
  
  PORTD&=~0X0C;
  PORTD|=0X10;
  delay(10000);
  cul_r=analogRead(A5);
  //cul_b=citire_CAN();
  //i=0;
  //while(i<10);//cat timp i=0 adica asteptam 10 ms

  
  if((cul_r>cul_y) && (cul_r > cul_b)){
    //red
    w=1;
  }
  else
    if((cul_b>cul_y) && (cul_b > cul_r)){
    //blue
    w=2;
    }
    else{
      //yellow
      w=3;
    }
  
  PORTD&=~0X10;
  return w;
}

void sonar()
{
  float timp;
                
  TCNT1 = 0;
  PORTD &= ~0x80;
  while(TCNT1 <= 160);                             // asteapta 10us   adica 160 tcnt1
  PORTD |= 0x80;
  while(TCNT1 <= 320);                            // asteapta 10us   adica 160 tcnt1
  PORTD &= ~0x80;
  TCNT1 = 0;
  
  while(PINB & 0x01) {
    if(TCNT1 == 65535) {
      n++;
      TCNT1 = 0;
    }
  }

  timp = (((n*65535) + TCNT1)*0.0625)/1000000;      //timpul  in secunde 
  distanta = timp*17150;

  if ((distanta > 1) ){
    if(distanta<20){ // daca e obstacol
    //aici se va implementa depasirea obstacolului 
    //deplasare la stanga
    //apoi in fata pt un anumit timp
    //apoi revenire pana se va semnala vectorul de senzori de linie
    //si apoi se va regla pozitia cu PID
    // ocolire();
    }
    else{//daca nu e obstacol
      if(oprire == 1){ //daca sa gatat linia
        int cul ;
        cul=culoare();
        if(cul==1){
           delay(3000);
           fata();
           delay(1000);
           //pana cand vede linie
        }
        else if(cul==2){
          //oprire 
          dreapta();
          delay(1000);
          //inapoi pe traseu
        }
        else if(cul==3){
          fata();
          delay(1000);
           //pana cand vede linie
        }
      }
      else { //altfel inseamna ca suntem pe traseu si continuam sa urmarim linia 
        PID();
      }
      
    }
    n=0;
  }
}

void setup() {
  DDRD=0x00;
  DDRB=0x00;
  pwm();
  timer2(); 
  setare_PID();
  setare_color();
  initializare_CAN();
  timer1();
  //Serial.begin(9600);
}
void loop(){
 // PID();
  sonar();
  /*
  int cul=0 ;
  cul=culoare();
        if(cul==1){
           Serial.println("rosu");
           //pana cand vede linie
        }
        else if(cul==2){
           Serial.println("galben");
          //inapoi pe traseu
        }
        else if(cul==3){
           Serial.println("albastru");
           //pana cand vede linie
        }
  */
  /*
  int cul_r=0,cul_y=0,cul_b=0;
  PORTD|=0X04;
  delay(10000);
  
  cul_r=analogRead(A5);

  PORTD&=~0X04;
  PORTD|=0X0C;
  delay(10000);
  
  cul_y=analogRead(A5);

  PORTD&=~0X0C;
  PORTD|=0X10;
  delay(10000);
  cul_b=analogRead(A5);
  Serial.println("rosu");
  Serial.println(cul_r);
  Serial.println("galben");
  Serial.println(cul_y);
  Serial.println("albastru");
  Serial.println(cul_b);
  PORTD&=~0X10;
  */
}

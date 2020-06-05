#define VN 170
#define KP 15

int contor_overflow_timer2 = 0;
int overflow_senzor_dist = 0;                //contor overflow sonar
int distanta = 0;
int oprire = 0;

void Timer1(){
  cli();                    //dezactiveaza toate intreruperile
  DDRD |= 0x80;               //pin 7 output trig
  DDRB &= ~0x01;              //pin 8 input echo  
  TCCR1A = 0;                             //curatare registrii
  TCCR1B = 0;                 
  TCCR1B |= (1 << CS10);                    //no prescaling
  sei();
}

void init_CAN()
{
  ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));//factor de divizare 128
  ADMUX |= (1 << REFS0);//setare tensiune de referinta
  ADCSRA |= (1 << ADEN);//Activare convertor
  ADCSRA |= (1 << ADSC);//start conversie CAN
}

void setare_culoare(){
  // pini 2 3 4 (pd2 pd3 pd4) pentru culoare rgb
  //2- red 
  //3-green
  //4 -blue
  DDRD |= 0x1C; //setare pini output RGB
  // analog 5 (A5) PC5 pin input fotorezistor
  DDRC &= ~0x20;
}

void init_PID(){
  DDRC &= ~0X1F; // PINII A0-A4 INPUT BANDA LEDURI
}

void PWM() {
 cli();    //dezactiveaza toate intreruperile
  DDRB |= 0x1E;            //pin 9,10,11,12 iesire pentru in1,2,3,4 directie motor
  DDRD |= 0X60;            //pin 5,6 pwm output 
  TCCR0A = 0;
  TCCR0B = 0;                                        //curatare registrii
  TCCR0A |= (1 <<WGM01) | (1<<WGM00)|(1<<COM0B1) | (1<<COM0A1);  //setare fast pwm
  TCCR0B |= (1<<CS00);                               //selectare no prescaling
 // OCR0B=114;
  sei();
}

void Timer2(){
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2B |= (1<< CS20) | (1<<CS21) | (1<<CS22);     //PRESCALER 1024
  TCCR2A |= (1<<WGM21);       //MOD CTC
  TIMSK2 |=(1<<OCIE2A);         //ACTIVARE INTRERUPERE LA COMPARARE A;
  OCR2A = 158;          //INTRERUPERE LA 10MS
  sei();
}


ISR(TIMER2_COMPA_vect){
  contor_overflow_timer2++;
}

void fata(){
  PORTB &= ~0X1E;
  PORTB |=0X0A;
  OCR0A = 200;
  OCR0B = 200;  
}

void spate(){
  PORTB &= ~0X1E;
  PORTB |= 0X14;
  OCR0A = 200;
  OCR0B = 200;
}

void stanga(){
  PORTB &= ~0X1E;
  PORTB |= 0X12;
  OCR0A = 200;
  OCR0B = 200;
}

void dreapta(){
  PORTB &= ~0X1E;
  PORTB |= 0X0C;
  OCR0A = 200;
  OCR0B = 200;
}

uint16_t read_CAN()
{
  uint32_t Ncan=0;
  ADMUX |= 0x20; // citire de la pinul A5
  ADCSRA |= (1 << ADSC); //start conversie
  while(ADCSRA & (1 << ADSC)); //urmarim si cand devine 0 => sfarsitul conversiei
  Ncan = ADC;
  return Ncan;
}

int track_sensor() {
  int eroare = 0;
  if((PINC & 0x1B) == 0){
    oprire = 1;
  }
  if((PINC & 0x08) == 0)       //S2(st->dr)
    eroare = 1;
  else if((PINC & 0x02) == 0)   //S4
          eroare = -1;
  else if((PINC & 0x10) == 0)   //S1
          eroare = 2;
  else if((PINC & 0x01) == 0)   //S5
          eroare = -2;
  else if((PINC & 0x04) == 0)   //S3
          eroare = 0;
          
  return eroare;
}

void PID(){
  int eroare = track_sensor();
  if((PINC & 0x10) == 0){  // senzor 5 activ viraj la stanga
     PORTB &= ~0X1E;
     PORTB |= 0X0A;
    OCR0A = VN + eroare * KP;
    OCR0B = VN - eroare * KP;
  }
  if((PINC & 0x08) == 0) {  // senzor 4 activ viraj la stanga
      PORTB &= ~0X1E;
      PORTB |= 0X0A;
    OCR0A = VN + eroare * KP;
    OCR0B = VN - eroare * KP;
  }
  if((PINC & 0x04) == 0) {   // senzor 3 activ in fata
    OCR0A = VN + eroare * KP;
    OCR0B = VN - eroare * KP;
  }
  if((PINC & 0x02) == 0) {  // senzor 2 activ viraj la dreapta
     PORTB &= ~0X1E;
     PORTB |= 0X0A;
    OCR0A = VN + eroare * KP;
    OCR0B = VN - eroare * KP;
  }
  if((PINC & 0x01) == 0) {  // senzor 1 activ viraj la dreapta
     PORTB &= ~0X1E;
     PORTB |= 0X0A;
    OCR0A = VN + eroare * KP;
    OCR0B = VN - eroare * KP;
  }
  if((PINC & 0x1B) == 0){
    PORTB &= ~0X1E;
  }
}

int culoare(){
  int cul_rosie = 0, cul_galben = 0, cul_albastru = 0;
  int flag_culoare = 0;
  
  PORTD |= 0X04;
  delay(10000);
  
  cul_rosie = analogRead(A5);
  
  PORTD &= ~0X04;
  PORTD |= 0X0C;
  delay(10000);
  
  cul_galben = analogRead(A5);
  
  PORTD &= ~0X0C;
  PORTD |= 0X10;
  delay(10000);
  
  cul_albastru = analogRead(A5);
  
  if((cul_rosie > cul_galben) && (cul_rosie > cul_albastru)){
    //rosie
    flag_culoare = 1;
  }
  else
    if((cul_albastru > cul_galben) && (cul_albastru > cul_rosie)){
      //albastru
      flag_culoare = 2;
    }
    else{
      //galben
      flag_culoare = 3;
    }
  
  PORTD &= ~0X10;
  return flag_culoare;
}

void ocolire() {
  stanga();
  TCNT1 = 0;
  TCNT1++;
  if (TCNT1 > 45000) {
    TCNT1=0;
  }
  dreapta();
  TCNT1 = 0;
  TCNT1++;
  if (TCNT1 > 1000) {
    TCNT1=0;
  }
  fata();
  if (TCNT1 > 50000) {
    TCNT1=0;
  }
  dreapta();
  if (TCNT1 > 45000) {
    TCNT1=0;
  }
  stanga();
  TCNT1 = 0;
  TCNT1++;
  if(TCNT1 > 1000) {
    TCNT1 = 0;
  }
  fata();     
}

void senzor_distanta()
{
  float timp;
                
  TCNT1 = 0;
  PORTD &= ~0x80;
  while (TCNT1 <= 160);                             // asteapta 10us   adica 160 tcnt1
  PORTD |= 0x80;
  while (TCNT1 <= 320);                            // asteapta 10us   adica 160 tcnt1
  PORTD &= ~0x80;
  TCNT1 = 0;
  
  while (PINB & 0x01) {
    if(TCNT1 == 65535) {
      overflow_senzor_dist++;
      TCNT1 = 0;
    }
  }

  timp = (((overflow_senzor_dist * 65535) + TCNT1) * 0.0625) / 1000000;      //timpul  in secunde 
  distanta = timp * 17150;

  if (distanta > 1){
    if(distanta < 20){
     // daca e obstacol
    //aici se va implementa depasirea obstacolului 
    //deplasare la stanga
    //apoi in fata pt un anumit timp
    //apoi revenire pana se va semnala vectorul de senzori de linie
    //si apoi se va regla pozitia cu PID
     ocolire();
    } else {
      //daca nu e obstacol
      int flag_culoare = culoare();
      if(oprire == 1){ 
        //daca s-a terminat linia
        
        if(flag_culoare == 1){
           delay(3000);
           fata();
           delay(1000);
           //pana cand vede linie
        }
        else if (flag_culoare == 2) {
          //oprire 
          dreapta();
          delay(1000);
          //inapoi pe traseu
        }
        else if (flag_culoare == 3) {
          fata();
          delay(1000);
           //pana cand vede linie
        }
      }
      else { //altfel inseamna ca suntem pe traseu si continuam sa urmarim linia 
        PID();
      }
      
    }
    overflow_senzor_dist = 0;
  }
}

void setup() {
  DDRD = 0x00;
  DDRB = 0x00;
  PWM();
  Timer2(); 
  Timer1();
  init_PID();
  setare_culoare();
  init_CAN();
}
void loop(){
  senzor_distanta();
}

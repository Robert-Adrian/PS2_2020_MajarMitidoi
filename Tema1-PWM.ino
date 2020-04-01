void setup() {
  // put your setup code here, to run once:

   cli();    //dezactiveaza toate intreruperile
  DDRB|=0x1E;            //pin 9,10,11,12 iesire pentru in1,2,3,4 directie motor
  DDRD|=0X60;            //pin 5,6 pwm output 
  TCCR0A=0;
  TCCR0B=0;                                        //curatare registrii
  TCCR0A |= (1 <<WGM01) | (1<<WGM00)|(1<<COM0B1) |(1<<COM0A1);  //setare fast pwm
  TCCR0B|=(1<<CS00);                               //selectare no prescaling
  sei();
  
}

void fata(){
  PORTB &=~0X1E;//oprim rotile
  PORTB |=0X0A; // pornim ambele roti
  OCR0A=250; //semnal pwm pentru viteza motorului / de miscare a rotii
  OCR0B=250; //semnal pwm pentru viteza motorului / de miscare a rotii
}

void spate(){
  PORTB &=~0X1E;
  PORTB |=0X14; // ambele roti se misca in spate
  OCR0A=250;
  OCR0B=250;
}

void stanga(){
  PORTB &=~0X1E;
  PORTB |=0X12; // roata stanga se misca in fata , iar cea din dreapta se misca in spate
  OCR0A=250;
  OCR0B=250;
}

void dreapta(){
  PORTB &=~0X1E;
  PORTB |=0X0C; // roata dreapta se misca in fata, iar cea din stanga se misca in spate
  OCR0A=250;
  OCR0B=250;
}


void loop() {
  // put your main code here, to run repeatedly:

}

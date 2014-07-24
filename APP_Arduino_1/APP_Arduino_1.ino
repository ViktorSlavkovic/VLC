/*      //////////////////////////////////////////////////////////////////////////////////////

        autor:     Viktor Slavkovic
        projekat:  Prenosenje podataka putem svetla
                   Petnica :: Racunarstvo :: 2014 :: Letnji Seminar
        
        verzija:   1.0
        
        opis:      Program za Arduino Leonardo koji dobijene
                   bajtove sa USB-a emituje LE Diodom, a istovremeno,
                   preko timer prekida (owerflow; 16bit; 16MHz - na 
                   svaki clock cycle), emitovani signal prima fotootpornikom
                   (svetlo - 1k; tama - 10k; maxV - 150V). Leonardov
                   AD konverotr (digitalRead()) moze da gura do 10^4
                   puta u sekundi.
                   Mozda bi brze bilo sa Timer4 (64Mhz ??):
                     http://provideyourown.com/2012/arduino-leonardo-versus-uno-whats-new/
                   Svakako, jako je sporo slati analogRead preko USB-a na svaki interrupt,
                   ali to je ok za testiranja, a u sustini, na arduinu bi trebo da bude
                   i dekoderski deo. Dakle, arduino treba da bude kompletan uredjaj
                   za komunikaciju i da vraca samo ono sto je primljeno.
                   Tutorial za prekide:
                     http://www.engblaze.com/we-interrupt-this-program-to-bring-you-a-tutorial-on-arduino-interrupts/
                     http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
                   Processor (ATmega16u4) Datasheet:
                     http://www.atmel.com/Images/Atmel-7766-8-bit-AVR-ATmega16U4-32U4_%20Datasheet.pdf
                   AVR Lib (Low level nacisto):
                     http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
                   
                   
//////////////////////////////////////////////////////////////////////////////////////      */    

#include <avr/interrupt.h>


////////////////////////////////////////////////////////
//              CONST && CONFIG
////////////////////////////////////////////////////////

const int H_Period = 20;         //  0.020 sec
const int L_Period = 10;         //  0.010 sec   
const int Delay_Period = 10;     //  0.010 sec
const int PR_Read_INT_Step = 1;  // PR_Read_INT_Step/16MHz*2^16 ~ 0.02048 sec (za 5)

const int ledPin = 3;
const int prPin = A0;

volatile int int_count = 0;      // usprava analogRead int_count puta
                                 // (brojac)

////////////////////////////////////////////////////////
//              SETUP && INIT
////////////////////////////////////////////////////////

void initPhotoResInt() {
  noInterrupts();   
  
  TCCR1A = 0;         // Set TCCR1A register to 0
  TCCR1B = 0;
  
  TIMSK1 = (1 << TOIE1);  // Enable Timer1 overflow interrupt  
  TCCR1B |= (1 << CS10);  // Set CS10 bit so timer runs at clock speed
  
  interrupts();       
}

void setup() {                
  
  Serial.begin(9600);       // Serial
  
  pinMode(ledPin, OUTPUT);  // Pin Init
  
  initPhotoResInt();        // Photoresistor Iterrupt Init
}

////////////////////////////////////////////////////////
//              IO
////////////////////////////////////////////////////////

void blinkBit(boolean state) {
  digitalWrite(ledPin, HIGH);
  delay((state)? H_Period : L_Period);       
  digitalWrite(ledPin, LOW);
  delay(Delay_Period);
}

void blinkByte(char c) {
  for (int i = 7; i >= 0; i--) {
    boolean b = bitRead(c, i);
    blinkBit(b);
  }
}

void handleIn() {
  if (Serial.available() > 0) {
      char c = Serial.read();
      blinkByte(c);
  }
}

void handleOut() {
  Serial.println(analogRead(prPin)*0.0049);
}

////////////////////////////////////////////////////////
//              ACTION
////////////////////////////////////////////////////////

ISR(TIMER1_OVF_vect)    //  Prekidna rutina
{
    int_count++;
    if (int_count == PR_Read_INT_Step) {
      handleOut();
      int_count = 0;
    }
}

void loop() {
    handleIn();
}

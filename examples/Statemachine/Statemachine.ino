/*
  Statemachine
  Einfaches Beispiel einer Statemachine, die eine LED blinken lässt
 
  This example code is in the public domain.
 */

#include <rslx.h>

const int LED = LED_BUILTIN;

// Statemachine
STATES (X, sStart, sAN, sAUS);

void sm()
{
  switch (X)
  {
  case sStart:
    // sStart setzt den pinMode richtig und wechselt dann zu sAN
    pinMode(LED, OUTPUT);
    Serial.println("Jetzt geht es los!");
    X = sAN;
    break;
  case sAN:
    // sAN schaltet die Lampe an und wechselt nach 0,5 Sekunden zu sAUS
    digitalWrite(LED, HIGH);
    if (X.age() >= 500)
    {
      Serial.println("sAN wechselt zu sAUS");
      X = sAUS;
    }
    break;  
  case sAUS:
    // sAUS schaltet die Lampe aus und wechselt nach 0,5 Sekunden zu sAN
    digitalWrite(LED, LOW);
    if (X.age() >= 500)
    {
      Serial.println("sAUS wechselt zu sAN");
      X = sAN;
    }
    break;  
  }
}


void setup() 
{
  // Serielle Kommunikation anknipsen (115200 Baud)
  Serial.begin(115200); while (!Serial) ;
  // rslx einrichten: Ein-/Ausgabe über Serial
  rslx.setup(Serial);
  // void sm() soll alle 50ms aufgerufen werden
  rslx.registerTask(sm, 50);
}

void loop() 
{
  // rslx update muss so oft wie möglich aufgerufen werden!
  // Damit wird der Parser und der Scheduler aufgerufen:
  // - Parser: liest von der seriellen Schnittstelle ein 
  //   und reagiert auf Kommandos
  // - Scheduler: ruft registrierte Tasks auf (wie z.B. void sm()...)
  rslx.update();
}

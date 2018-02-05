/*
  Statemachine
  Einfaches Beispiel einer Statemachine, die eine LED blinken lässt
  Die Blinkzeit lässt sich dabei über die Serielle Konsole einstellen:
  
  v, vl - Listet alle Variablen auf
  vw <name> <wert> setzt Variable <name> auf den neuen <wert>
  vw <index> <wert> setzt Variable Nummer <index> auf neuen <wert>
  vd - Setzt alle Variablen auf ihren <default> zurück
  vs - Speichert den aktuellen Wert als neuen <default> ab
 
  This example code is in the public domain.
 */

#include <rslx.h>

const int LED = LED_BUILTIN;

// Statemachine
STATES (X, sStart, sAN, sAUS);

// Variablen
// VAR ( <name>, <typ>, <default-wert> );
VAR(duration, int, 500);   // gleich wie int duration = 500; aber über Console schreib-/lesbar 

void sm()
{
  if (sStart == X)
  {
    // sStart setzt den pinMode richtig und wechselt dann zu sAN
    pinMode(LED, OUTPUT);
    Serial.println("Jetzt geht es los!");
    X = sAN;
  }
  else if (sAN == X)
  {
    // sAN schaltet die Lampe an und wechselt nach duration Millisekunden zu sAUS
    digitalWrite(LED, HIGH);
    if (X.age() >= duration)
    {
      X = sAUS;
    }
  }
  else if (sAUS == X)
  {
    // sAUS schaltet die Lampe aus und wechselt nach duration Millisekunden zu sAN
    digitalWrite(LED, LOW);
    if (X.age() >= duration)
    {
      X = sAN;
    }
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
  // - Parser: liest von der seriellen Schnittstelle ein und 
  //   reagiert auf Kommandos
  // - Scheduler: ruft registrierte Tasks auf (wie z.B. void sm()...)
  rslx.update();
}

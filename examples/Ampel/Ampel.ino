/*
  Ampel
  Ampelsteuerung für eine Bedarfsfußgängerampel zur Demonstration 
  einer Statemachine-Steuerung
 
  This example code is in the public domain.
 */

#include <rslx.h>

// Pins, an denen die Ampel Lichter und Taster hängen
const int ledAR = 8;          // Auto Rot
const int ledAGe = 7;         // Auto Gelb
const int ledAGr = 6;         // Auto Grün
const int pinTaster = 5;      // Fußgänger-Taster
const int ledFR = 4;          // Fußgänger Rot
const int ledFGr = 3;         // Fußgänger Grün

// Steuerung Ist-Werte und Soll-Werte
VAR(x, int, 0);   // aktueller Zustand des Tasters 
VAR(iw_tast, int, 0);   // aktueller Zustand des Tasters 
VAR(sw_AR, int, 0);     // Soll-Wert der Autoampel Rot
VAR(sw_AGe, int, 0);    // Soll-Wert der Autoampel Gelb
VAR(sw_AGr, int, 0);    // Soll-Wert der Autoampel Grün
VAR(sw_FR, int, 0);     // Soll-Wert der Fußgängerampel Gelb
VAR(sw_FGr, int, 0);    // Soll-Wert der Fußgängerampel Grün
PARAM(sw_T, int, 100);  // Entprell-Wert des Tasters (muss mind. bw_T ms gedrückt sein)
PARAM(sw_U, int, 2000); // Dauer einer Ampelumschaltzeit
PARAM(sw_U2, int, 10000); // Dauer einer der Fußgängergrünphase

// Zustände der Steuerung
STATES (X, sInit, sAus, sTaster, sAGr, sAGe, sAR, sFGr, sFR, sARGe, sAGr2);

// Fehler der Steuerung
ERRORS (eTASTER, eINTERNAL);

void steuerung()
{
  // alle Ist-Werte einlesen
  if (X > sInit)
  {
    iw_tast = 1 - digitalRead(pinTaster); // Pullup, also ist 1 die 0 und andersrum...
  }
  
  // Steuerung
  if (sInit == X)
  {
    // Zu Beginn: alle Pins auf den passenden Modus setzen
    pinMode(ledAR, OUTPUT);
    pinMode(ledAGe, OUTPUT);
    pinMode(ledAGr, OUTPUT);
    pinMode(ledFR, OUTPUT);
    pinMode(ledFGr, OUTPUT);
    pinMode(pinTaster, INPUT_PULLUP);
    X = sAus;
  }
  else if (sAus == X)
  {
    // Normalerweise ist die Bedarfsampel aus. 
    sw_AR = sw_AGe = sw_AGr = sw_FR = sw_FGr = 0;
    // Wir warten auf die Betätigung des Tasters!
    if (iw_tast)
    {
      X = sTaster;
    }
  }
  else if (sTaster == X)
  {
    // Der Taster soll eine bestimmte Zeit gedrückt sein! Flackernde 
    // Taster-Bits interpretieren wir als defekten Taster.
    if ((iw_tast == 0) && (X.age() < sw_T))
    {
      // Fehler merken
      Errors.signal(E_ERROR, eTASTER);
      // sinnvollerweise ist die Ampel dann aus
      X = sAus;
    }
    else if (X.age() >= sw_T)
    {
      // jemand hat gedrückt, machen wir die Ampel an !
      // den Fehler löschen wir auch, denn jetzt hat der Taster ja funktioniert
      Errors.clear(eTASTER);
      X = sAGr;
    }
  }
  else if (sAGr == X)
  {
    // A-Ampel grün, F-Ampel rot, Rest aus
    sw_FR = sw_AGr = 1;
    sw_AR = sw_AGe = sw_FGr = 0;
    // nach der Umschaltzeit sw_U zu Gelb wechseln
    if (X.age() > sw_U)
    {
      X = sAGe;
    }
  }
  else if (sAGe == X)
  {
    // A-Ampel gelb, F-Ampel rot, Rest aus
    sw_FR = sw_AGe = 1; 
    sw_AR = sw_AGr = sw_FGr = 0;
    // nach der Umschaltzeit sw_U zu Rot wechseln
    if (X.age() > sw_U)
    {
      X = sAR;
    }
  }
  else if (sAR == X)
  {
    // A-Ampel rot, F-Ampel rot
    sw_FR = sw_AR = 1;
    sw_AGe = sw_AGr = sw_FGr = 0;
    // nach der Umschaltzeit sw_U zu Fußgänger grün wechseln
    if (X.age() > sw_U)
    {
      X = sFGr;
    }
  }
  else if (sFGr == X)
  {
    sw_FGr = sw_AR = 1; sw_FR = 0;
    sw_AGe = sw_AGr = 0;
    // nach der Umschaltzeit sw_U2 zu Fußgänger rot wechseln
    if (X.age() > sw_U2)
    {
      X = sFR;
    }
  } 
  else if (sFR == X)
  {
    sw_FR = sw_AR = 1; 
    sw_FGr = sw_AGe = sw_AGr = 0;
    // nach der Umschaltzeit sw_U zu rot/gelb wechseln
    if (X.age() > sw_U)
    {
      X = sARGe;
    }
  } 
  else if (sARGe == X)
  {
    sw_FR = sw_AGe = sw_AR = 1;
    sw_AGr = sw_FGr = 0;
    // nach der Umschaltzeit sw_U zu grün2 wechseln
    if (X.age() > sw_U)
    {
      X = sAGr2;
    }
  }
  else if (sAGr2 == X)
  {
    sw_FR = sw_AGr = 1; 
    sw_AGe = sw_AR = sw_FGr = 0;
    // nach der Umschaltzeit 5*sw_U machen wir die Ampel wieder ganz aus, 
    // es seie denn jemand drückt davor wieder den Taster...
    if (iw_tast)
    {
      X = sTaster;
    }
    else if (X.age() > 5*sw_U)
    {
      X = sAus;
    }
  } 
  else
  {
    Errors.set(E_FATAL, eINTERNAL);
  }
  
  // alle Soll-Werte schreiben
  if (X > sInit)
  {
    digitalWrite(ledAR, sw_AR);
    digitalWrite(ledAGe, sw_AGe);
    digitalWrite(ledAGr, sw_AGr);
    digitalWrite(ledFR, sw_FR);
    digitalWrite(ledFGr, sw_FGr);
  } 
}


void setup() 
{
  // Serielle Kommunikation anknipsen (115200 Baud)
  Serial.begin(115200);
  // rslx einrichten
  rslx.setup(Serial);
  // void steuerung() soll alle 25ms aufgerufen werden
  rslx.registerTask(steuerung, 25);
}

void loop() 
{
  // rslx update muss so oft wie möglich aufgerufen werden!
  // Damit wird der Parser und der Scheduler aufgerufen:
  // - Parser: liest von der seriellen Schnittstelle ein 
  //   und reagiert auf Kommandos
  // - Scheduler: ruft registrierte Tasks auf (wie z.B. void steuerung()...)
  rslx.update();
}

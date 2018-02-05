/*
  Statemachine Dimmer Beispiel
  Einfaches Beispiel einer Statemachine, die eine LED ein-/ausdimmt

  This example code is in the public domain.
*/

#include <rslx.h>

const int LED = 11;
const int TASTER = 3;

// Statemachine
STATES (X, sStart, sWarten, sTaster, sDimmen, sSchalten);

// Variablen
VAR(hell, int, 0);
VAR(richtung, int, 1);
VAR(taster, int, 0);

void sm()
{
  // immer den Taster auslesen
  taster = digitalRead(TASTER);

  if (sStart == X)
  {
    // sStart nix zu tun
    X = sWarten;
  }
  else if (sWarten == X)
  {
    // Taster auswerten
    if (LOW == taster)
    {
      X = sTaster;
    }
  }
  else if (sTaster == X)
  {
    // wurde der Taster innerhalb von 500ms losgelassen?
    if ((HIGH == taster) && (X.age() <= 500))
    {
      X = sSchalten;
    }

    // ist der Taster schon länger als 500ms gedrückt?
    else if (X.age() >= 500)
    {
      X = sDimmen;
    }
  }
  else if (sSchalten == X)
  {
    if (hell != 0)
    {
      hell = 0;
      richtung = 1;
    }
    else
    {
      hell = 255;
      richtung = -1;
    }
    X = sWarten;
  }
  else  if (sDimmen == X)
  {
    // solange die Helligkeit verändern,
    hell += richtung;
    if (hell > 255)
    {
      richtung = -1;
      hell = 255;
    }

    if (hell < 0)
    {
      richtung = 1;
      hell = 0;
    }

    // bis der Taster nicht mehr gedrückt ist
    if (HIGH == taster)
    {
      X = sWarten;
      // Dimmrichtung umdrehen
      richtung *= -1;
    }
  }

  // immer die Helligkeit rausschreiben
  analogWrite(LED, hell);
}


void setup()
{
  // Serielle Kommunikation anknipsen (115200 Baud)
  Serial.begin(115200); while (!Serial) ;
  // rslx einrichten: Ein-/Ausgabe über Serial
  rslx.setup(Serial);
  // void sm() soll alle 10ms aufgerufen werden
  rslx.registerTask(sm, 10);
  // PinModes
  pinMode(LED, OUTPUT);
  pinMode(TASTER, INPUT_PULLUP);
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


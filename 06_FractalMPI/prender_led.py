#include <Servo.h>

Servo servoSolar;

// -------------------- Pines --------------------
const int LDR_IZQ_PIN = A0;
const int LDR_DER_PIN = A1;
const int POT_PIN     = A2;

const int SERVO_PIN = 9;
const int LED_PIN   = 3;

// -------------------- Variables --------------------
int angulo = 90;
bool modoManual = false;

unsigned long tiempoLED = 0;
bool estadoLED = false;

void setup() {

  Serial.begin(9600);

  servoSolar.attach(SERVO_PIN);
  servoSolar.write(angulo);

  pinMode(LED_PIN, OUTPUT);

  Serial.println("Sistema iniciado");
  Serial.println("M = Manual");
  Serial.println("A = Automatico");
  Serial.println("I = Izquierda");
  Serial.println("D = Derecha");
  Serial.println("S = Stop");
}

void loop() {

  //=================================================
  // COMANDOS SERIAL
  //=================================================

  if (Serial.available()) {

    char comando = toupper(Serial.read());

    if (comando == 'M') {

      modoManual = true;
      Serial.println("Modo MANUAL");
    }

    else if (comando == 'A') {

      modoManual = false;
      Serial.println("Modo AUTOMATICO");
    }

    else if (modoManual) {

      if (comando == 'I') {

        angulo--;

        if (angulo < 0)
          angulo = 0;

        servoSolar.write(angulo);

        Serial.println("Izquierda");
      }

      else if (comando == 'D') {

        angulo++;

        if (angulo > 180)
          angulo = 180;

        servoSolar.write(angulo);

        Serial.println("Derecha");
      }

      else if (comando == 'S') {

        digitalWrite(LED_PIN, LOW);

        Serial.println("Detenido");
      }
    }
  }

  //=================================================
  // MODO AUTOMATICO
  //=================================================

  int luzIzq = analogRead(LDR_IZQ_PIN);
  int luzDer = analogRead(LDR_DER_PIN);

  int sensibilidad = map(analogRead(POT_PIN), 0, 1023, 5, 100);

  bool moviendo = false;

  if (!modoManual) {

    int diferencia = luzIzq - luzDer;

    if (abs(diferencia) > sensibilidad) {

      moviendo = true;

      if (diferencia > 0) {

        angulo++;

        if (angulo > 180)
          angulo = 180;
      }
      else {

        angulo--;

        if (angulo < 0)
          angulo = 0;
      }

      servoSolar.write(angulo);

      parpadearLED();
    }
    else {

      digitalWrite(LED_PIN, LOW);
    }
  }
  else {

    digitalWrite(LED_PIN, HIGH);
  }

  //=================================================
  // ENVÍO DE DATOS
  //=================================================

  Serial.print(luzIzq);
  Serial.print(",");

  Serial.print(luzDer);
  Serial.print(",");

  Serial.print(sensibilidad);
  Serial.print(",");

  Serial.print(angulo);
  Serial.print(",");

  Serial.println(modoManual ? "1" : "0");

  delay(50);
}

//=================================================
// PARPADEO LED
//=================================================

void parpadearLED() {

  if (millis() - tiempoLED >= 200) {

    tiempoLED = millis();

    estadoLED = !estadoLED;

    digitalWrite(LED_PIN, estadoLED);
  }
}
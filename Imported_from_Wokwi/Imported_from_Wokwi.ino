#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define PIN_NTC   A0
#define PIN_POT   A1
#define PIN_RELAY 6
#define PIN_LED_V 7
#define PIN_LED_R 8
#define PIN_BUZ   9
#define PIN_BTN   2

const float BETA = 3950.0;   // coefficient de la NTC Wokwi
const float HYST = 2.0;      // hystérésis en °C
bool alarme = false;

float lireTemperature() {
  long somme = 0;
  for (int i = 0; i < 10; i++) { somme += analogRead(PIN_NTC); delay(5); }
  float v = somme / 10.0;                       // moyenne anti-bruit
  return 1 / (log(1 / (1023.0 / v - 1)) / BETA + 1 / 298.15) - 273.15;
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED_V, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_BUZ, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.print("Protection moteur");
  delay(1500);
  lcd.clear();
}

void loop() {
  float t = lireTemperature();
  float seuil = 20.0 + analogRead(PIN_POT) * 60.0 / 1023.0;  // seuil 20 à 80 °C

  // Déclenchement
  if (!alarme && t >= seuil) alarme = true;
  // Réarmement : température redescendue ET bouton appuyé
  if (alarme && t < seuil - HYST && digitalRead(PIN_BTN) == LOW) alarme = false;

  // Sorties
  digitalWrite(PIN_RELAY, alarme ? LOW : HIGH);   // relais coupé en alarme
  digitalWrite(PIN_LED_V, !alarme);
  digitalWrite(PIN_LED_R, alarme);

  if (alarme && (millis() / 300) % 2 == 0) tone(PIN_BUZ, 1000);
  else noTone(PIN_BUZ);

  // Affichage LCD
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t, 1);
  lcd.print("C S:"); lcd.print(seuil, 0); lcd.print("C  ");
  lcd.setCursor(0, 1);
  lcd.print(alarme ? "!! SURCHAUFFE !! " : "Etat: NORMAL     ");

  Serial.print(t); Serial.print(" ; "); Serial.println(seuil);
}
const uint8_t PIN_BTN   = 2;   // кнопка на GND, INPUT_PULLUP
const uint8_t PIN_ENA   = 3;   // PWM -> ENA L298N
const uint8_t PIN_IN1   = 5;
const uint8_t PIN_IN2   = 6;
const uint8_t PIN_TRIG  = 9;
const uint8_t PIN_ECHO  = 10;
const uint8_t PIN_LED_G = 11;
const uint8_t PIN_LED_Y = 12;
const uint8_t PIN_LED_R = 13;

const uint8_t PWM_NEAR = 120;  // ~47 %
const uint8_t PWM_FAR  = 255;  // 100 %
const float DIST_STOP_CM     = 20.0;  // < 20  -> стоп
const float DIST_NEAR_MAX_CM = 50.0;  // 20..50 включительно -> 120

const bool TEST_REVERSE = false;      // true = тест №8 (имитация реверса)

void setLeds(bool r, bool y, bool g) {
  digitalWrite(PIN_LED_R, r);
  digitalWrite(PIN_LED_Y, y);
  digitalWrite(PIN_LED_G, g);
}

void motorStop() {                    // IN1=IN2=LOW, ENA=0 -> выбег (мост выключен)
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, 0);
}

void motorForward(uint8_t pwm) {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, pwm);
}

void motorBackward(uint8_t pwm) {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, HIGH);
  analogWrite(PIN_ENA, pwm);
}

void drive(uint8_t pwm) {
  if (TEST_REVERSE) motorBackward(pwm); else motorForward(pwm);
}

bool buttonPressed() {                // блокирующий дебаунс
  if (digitalRead(PIN_BTN) == LOW) {
    delay(30);
    return digitalRead(PIN_BTN) == LOW;
  }
  return false;
}

float readDistanceCm() {              // НАИВНО: тайм-аут -> pulseIn()=0 -> 0 см
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000UL);
  return us * 0.0343f / 2.0f;
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_Y, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  motorStop();
  setLeds(0, 0, 0);
  Serial.println(F("SRS7 ready"));
}

void loop() {
  if (!buttonPressed()) {             // кнопка отпущена: мотор стоп, LED выкл, датчик не опрашиваем
    motorStop();
    setLeds(0, 0, 0);
    return;
  }

  float cm = readDistanceCm();

  if (cm < DIST_STOP_CM) {
    motorStop();
    setLeds(1, 0, 0);
  } else if (cm <= DIST_NEAR_MAX_CM) {
    drive(PWM_NEAR);
    setLeds(0, 1, 0);
  } else {
    drive(PWM_FAR);
    setLeds(0, 0, 1);
  }

  Serial.print(F("t="));  Serial.print(millis());
  Serial.print(F("ms d=")); Serial.print(cm, 1);
  Serial.println(F(" cm"));
  delay(100);
}

#include <PinChangeInterrupt.h>
#include <EEPROM.h>

const int mosfet = 3;
const int forcedStartButton = 4;
const int timerButton = 5;
const int floaterInterraption = 6;
const int oilOKLed = 7;
const int oilEMPTYLed = 8;

volatile bool floaterInterraptionState;
volatile bool forcedStartButtonState = false;
volatile bool timerButtonState = false;

// this type is needed for sum of result of millis and MOTOR_DURATION
int64_t timerStartTime = 0; // Время начала отсчёта таймера
// uint64_t timerStartTime = 0; // Время начала отсчёта таймера
// unsigned long timerStartTime = 0; // Время начала отсчёта таймера
const unsigned long TIMER_INTERVAL = 100000; // Интервал таймера (5 секунд)
const unsigned long MOTOR_DURATION = 6019; // 6 секунд 19 миллисекунд
bool timerActive = false; // Флаг активности таймера

// сохранять значение каждую пятую часть от таймера
const unsigned long saveRemainingTimerTimeInterval = TIMER_INTERVAL / 5;
unsigned long lastSaveRemainingTimerTime = millis();

#define DEBUG true

void debugTime(){
  if (DEBUG) {
    Serial.print(millis());
    Serial.print(" ");
  }
}

void debugPrint(const char* message) {
  if (DEBUG) {
    Serial.print(message);
  }
}

void debugPrintln(const char* message) {
  if (DEBUG) {
    Serial.println(message);
  }
}

void debugPrintlnTimed(const char* message) {
  if (DEBUG) {
    debugTime();
    Serial.println(message);
  }
}
// print function; can be removed
void debug_print_uint64_t(uint64_t num) {
  if (DEBUG) {
    // Function that can print uint64_t nums
    char rev[128]; 
    char *p = rev+1;

    while (num > 0) {
      // Serial.println('0' + (num % 10));
      *p++ = '0' + (num % 10);
      num /= 10;
    }
    p--;
    /*Print the number which is now in reverse*/
    while (p > rev) {
      Serial.print(*p--);
    }
    Serial.println(*p);
    // Serial.println();
  }
}

// print function; can be removed
void debug_print_int64_t(int64_t num) {
  if (DEBUG) {
    char rev[128]; 
    char *p = rev+1;

    if (num < 0) {
        Serial.print('-');
        num = -num;
    }

    while (num > 0) {
        *p++ = '0' + (num % 10);
        num /= 10;
    }
    p--;
    /*Print the number which is now in reverse*/
    while (p > rev) {
        Serial.print(*p--);
    }
    // Serial.println(*p);
  }
}


void changeTimerButtonState() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) {
    timerButtonState = !timerButtonState;
    if (timerButtonState) {
      timerActive = true; // Активируем таймер при нажатии кнопки
      // timerStartTime = millis(); // Запоминаем время начала отсчёта
      timerStartTime = (int64_t)millis() + loadRemainingTimerTimeFromEEPROM();
      // debug_print_int64_t((int64_t)millis() - timerStartTime);
      // debugPrintlnTimed(" ");
      debugPrintlnTimed("Timer started");
    } else {
      timerActive = false; // Деактивируем таймер при отпускании кнопки
      debugPrintlnTimed("Timer stopped");
      // Delete saved remaining time
      saveRemainingTimerTimeToEEPROM(0);
    }
  }
  lastInterruptTime = interruptTime;
}

void changeForcedStartState() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) {
    forcedStartButtonState = !forcedStartButtonState;
    if (forcedStartButtonState && timerActive) {
      timerStartTime = millis(); // Сбрасываем таймер при нажатии кнопки
      debugPrintlnTimed("Timer reset");
    }
  }
  lastInterruptTime = interruptTime;
}

void changeOilState() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) {
    floaterInterraptionState = !floaterInterraptionState;
    digitalWrite(oilOKLed, !floaterInterraptionState);
    digitalWrite(oilEMPTYLed, floaterInterraptionState);
  }
  lastInterruptTime = interruptTime;
}

void controlMotor(bool start) {
  static unsigned long motorStartTime = 0;

  if (motorStartTime > 0 && millis() - motorStartTime >= MOTOR_DURATION) {
    digitalWrite(mosfet, LOW);
    motorStartTime = 0;
    debugPrintlnTimed("mosfet=off");
  } else if (start && motorStartTime == 0) {
    digitalWrite(mosfet, HIGH);
    motorStartTime = millis();
    debugPrintlnTimed("mosfet=on");
  }
}

void saveRemainingTimerTimeToEEPROM(int64_t remainingTime) { 
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.begin(512);
// #endif
  // Save value to EEPROM
  debug_print_int64_t(remainingTime);
  debugPrintln(" ");
  EEPROM.put(0, remainingTime);
// #if defined(ESP8266)|| defined(ESP32)
//         EEPROM.commit();
// #endif
  debugTime();
  debugPrint("Value ");
  debug_print_int64_t(remainingTime);
  debugPrint(" saved to EEPROM address: ");
  debugPrintln(0);
}

int64_t loadRemainingTimerTimeFromEEPROM() {
    int64_t _remainingTime;
    EEPROM.get(0, _remainingTime);
    debugPrint("Loaded remaining timer time(ms): ");
    debug_print_int64_t(_remainingTime);
    debugPrintln(" ");
    if (_remainingTime <= 0){
      return (int64_t)TIMER_INTERVAL;
    } else {
      return _remainingTime;
    }
}

void setup() {
  Serial.begin(9600);
  pinMode(mosfet, OUTPUT);
  pinMode(oilOKLed, OUTPUT);
  pinMode(oilEMPTYLed, OUTPUT);

  attachPCINT(digitalPinToPCINT(timerButton), changeTimerButtonState, CHANGE);
  // timerButtonState = digitalRead(timerButton);
  // timerStartTime = (int64_t)millis() + loadRemainingTimerTimeFromEEPROM();
  // timerStartTime = (int64_t)millis() + 5000;
  saveRemainingTimerTimeToEEPROM(5000);
  timerStartTime = (int64_t)millis() + loadRemainingTimerTimeFromEEPROM();
  debug_print_int64_t(timerStartTime);

  while(1);

  attachPCINT(digitalPinToPCINT(forcedStartButton), changeForcedStartState, CHANGE);
  // forcedStartButtonState = digitalRead(forcedStartButton);

  attachPCINT(digitalPinToPCINT(floaterInterraption), changeOilState, CHANGE);

  floaterInterraptionState = digitalRead(floaterInterraption);
  digitalWrite(oilOKLed, !floaterInterraptionState);
  digitalWrite(oilEMPTYLed, floaterInterraptionState);
}

void loop() {
  // Проверка таймера
  debug_print_int64_t(timerStartTime);
  debugPrintln(" ");
  if (timerActive && (int64_t)millis() - timerStartTime >= TIMER_INTERVAL) {
    debugPrintlnTimed("Timer elapsed, starting motor");
    controlMotor(true); // Включаем мотор по истечении таймера
    // // timerActive = false; // Деактивируем таймер
    // При окончании таймера, сбрасываем таймер и прибавляем время, 
    // которое необходимо на смазку, чтобы смазка по таймеру 
    // не происходила сразу после принудительной смазки
    timerStartTime = (int64_t)millis() + (int64_t)MOTOR_DURATION;
    // Serial.println(millis());
    // Serial.println(MOTOR_DURATION);
    // print_int64_t(timerStartTime);
    // print_int64_t((int64_t)millis() - timerStartTime);
    // Serial.println(TIMER_INTERVAL);
  }

  // Проверка принудительного запуска
  if (forcedStartButtonState) {
    debugPrintlnTimed("Forced start, starting motor");
    forcedStartButtonState = false;
    controlMotor(true); // Включаем мотор принудительно
    // При принудительном запуске, сброс таймера
    timerStartTime = (int64_t)millis() + (int64_t)MOTOR_DURATION;
  }

  if (timerActive && millis() - lastSaveRemainingTimerTime >= saveRemainingTimerTimeInterval){
    // Save remaining time of timer for next session in case of earlier machine stop
    saveRemainingTimerTimeToEEPROM(timerStartTime - (int64_t)millis());
    lastSaveRemainingTimerTime = millis();
  }

  // Управление мотором
  controlMotor(false); // Выключаем мотор, если время истекло
}
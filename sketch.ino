#include <PinChangeInterrupt.h>
#include <EEPROM.h>

#define DEBUG false

constexpr int MOSFET = 3;
constexpr int FORCED_START_BUTTON = 4;
constexpr int TIMER_BUTTON = 5;
constexpr int FLOATER_INTERRUPT  = 6;
constexpr int OIL_OK_LED  = 7;
constexpr int OIL_EMPTY_LED = 8;

constexpr unsigned long MOTOR_DURATION = 6019; // 6 секунд 19 миллисекунд
constexpr unsigned long TIMER_INTERVAL = 10000; // Интервал таймера (5 секунд)
constexpr unsigned long SAVE_REMAINING_TIMER_TIME_INTERVAL = TIMER_INTERVAL / 5; // Интервал сохранения времени
constexpr unsigned long DEFAULT_AFTER_MOTOR_TIMER_TIME = 2 * TIMER_INTERVAL + MOTOR_DURATION;

constexpr unsigned long DEBOUNCE_DELAY = 200; // Задержка для устранения дребезга
constexpr int EEPROM_ADDRESS = 0; // Адрес для сохранения времени

volatile bool floaterInterraptionState;
volatile bool forcedStartButtonState = false;
volatile bool timerButtonState = false;

volatile unsigned long timerStartTime = 0; // Время окончания таймера
volatile unsigned long lastSaveRemainingTimerTime = millis();

// bool timerActive = false; // Флаг активности таймера
bool shouldSaveRemainingTime = true; // Флаг нужно ли сохранять оставшееся время

#define DEBUG_PRINT(x) if (DEBUG) { Serial.print(x); }
#define DEBUG_PRINTLN(x) if (DEBUG) { Serial.println(x); }
#define DEBUG_PRINT_TIME(x) if (DEBUG) { Serial.print(millis()); Serial.print(": "); Serial.println(x); }
#define DEBUG_PRINTF(fmt, ...) if (DEBUG) { Serial.printf(fmt, __VA_ARGS__); }


void changeTimerButtonState() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    timerButtonState = !timerButtonState;
    if (timerButtonState) {
      // timerActive = true; // Активируем таймер при нажатии кнопки
      // miliseconds that remains from the previous startup of the machine and hardcoded value to next lubrication
      timerStartTime = loadRemainingTimerTimeFromEEPROM() + timerStartTime;
      // Выставляем дефолтное время таймера, чтобы не срабатывало сразу же
      if (timerStartTime == 0){
        timerStartTime = 2 * TIMER_INTERVAL;
      }
      // В первом запуске не учитывается время работы мотора
      timerStartTime = interruptTime + timerStartTime; // Запоминаем время начала отсчёта таймера
      // Start save timer here to do not save one more time after timer is turned off
      lastSaveRemainingTimerTime = interruptTime;
      DEBUG_PRINT_TIME("Timer started");
    } else {
      // timerActive = false; // Деактивируем таймер при повторном нажатии кнопки
      DEBUG_PRINT_TIME("Timer stopped");
      if (shouldSaveRemainingTime && timerStartTime >= interruptTime){
        // Timer is active
        saveRemainingTimerTimeToEEPROM(timerStartTime - interruptTime);
      }
      timerStartTime = 0; // Сбрасываем предыдущий таймер
      // saveRemainingTimerTimeToEEPROM(0); // Delete saved remaining time
    }
  lastInterruptTime = interruptTime;
  }
}

void changeForcedStartState() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 400) {
    forcedStartButtonState = !forcedStartButtonState;
    if (forcedStartButtonState && timerButtonState) {
      // При принудительном запуске, сброс таймера
      timerStartTime = interruptTime + DEFAULT_AFTER_MOTOR_TIMER_TIME;
      DEBUG_PRINT_TIME("Timer reset");
    }
    lastInterruptTime = interruptTime;
  }
}

void changeOilState() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    floaterInterraptionState = !floaterInterraptionState;
    digitalWrite(OIL_OK_LED, !floaterInterraptionState);
    digitalWrite(OIL_EMPTY_LED, floaterInterraptionState);
  }
  lastInterruptTime = interruptTime;
}

void controlMotor(bool start) {
  static unsigned long motorStartTime = 0;
  unsigned long call_time = millis();

  if (motorStartTime > 0 && call_time - motorStartTime >= MOTOR_DURATION) {
    shouldSaveRemainingTime = true;
    digitalWrite(MOSFET, LOW);
    motorStartTime = 0;
    DEBUG_PRINT_TIME("MOSFET=off");
  } else if (start && motorStartTime == 0) {
    shouldSaveRemainingTime = false;
    digitalWrite(MOSFET, HIGH);
    motorStartTime = call_time;
    DEBUG_PRINT_TIME("MOSFET=on");
  }
}

void saveRemainingTimerTimeToEEPROM(unsigned long remainingTime) { 
  // #if defined(ESP8266)|| defined(ESP32)
  //         EEPROM.begin(512);
  // #endif
  EEPROM.put(EEPROM_ADDRESS, remainingTime);
  // #if defined(ESP8266)|| defined(ESP32)
  //         EEPROM.commit();
  // #endif
  DEBUG_PRINTLN("Saved to EEPROM: "); 
  DEBUG_PRINT_TIME(remainingTime);
}

unsigned long loadRemainingTimerTimeFromEEPROM() {
    unsigned long remainingTime;
    EEPROM.get(EEPROM_ADDRESS, remainingTime);
    // If memory was empty before, it returns -1 but as unsigned long it is 2**32-1
    if (remainingTime + 1 == 0) { // Проверка на неинициализированное значение
      remainingTime = 0;
    }
    DEBUG_PRINTLN("Loaded remaining timer time(ms): ");
    DEBUG_PRINT_TIME(remainingTime);
    return remainingTime;
}

void setup() {
  if (DEBUG) Serial.begin(9600);
  pinMode(MOSFET, OUTPUT);
  pinMode(OIL_OK_LED , OUTPUT);
  pinMode(OIL_EMPTY_LED, OUTPUT);

  attachPCINT(digitalPinToPCINT(TIMER_BUTTON), changeTimerButtonState, CHANGE);
  // timerButtonState = digitalRead(TIMER_BUTTON);
  // saveRemainingTimerTimeToEEPROM((unsigned long)15392);

  // unsigned long test = (millis() + 1000);
  // Serial.println(test - millis());
  // delay(1000);
  // Serial.println(test - millis());
  // while(1);

  attachPCINT(digitalPinToPCINT(FORCED_START_BUTTON), changeForcedStartState, CHANGE);

  attachPCINT(digitalPinToPCINT(FLOATER_INTERRUPT), changeOilState, CHANGE);

  floaterInterraptionState = digitalRead(FLOATER_INTERRUPT);
  digitalWrite(OIL_OK_LED , !floaterInterraptionState);
  digitalWrite(OIL_EMPTY_LED, floaterInterraptionState);
}

void loop() {
  unsigned long currentMillis = millis();
  // Копируем volatile переменные в локальные
  bool timerActive = timerButtonState;
  bool forcedStart = forcedStartButtonState;
  unsigned long timerStart = timerStartTime;
  unsigned long lastSaveRemainingTimer = lastSaveRemainingTimerTime;

  // Проверка таймера
  if (timerActive && timerStart > currentMillis && timerStart - currentMillis <= TIMER_INTERVAL) {
    DEBUG_PRINT_TIME("Timer elapsed, starting motor");
    controlMotor(true); // Включаем мотор по истечении таймера
    // При окончании таймера, сбрасываем таймер и прибавляем время, 
    // которое необходимо на смазку, и время таймера, 
    // чтобы смазка по таймеру не происходила сразу после принудительной смазки
    timerStartTime = timerStart = currentMillis + DEFAULT_AFTER_MOTOR_TIMER_TIME;
  }

  // Проверка принудительного запуска
  if (forcedStart) {
    forcedStartButtonState = false;  // Очищаем volatile переменную
    DEBUG_PRINT_TIME("Forced start, starting motor");
    saveRemainingTimerTimeToEEPROM(0);
    controlMotor(true); // Включаем мотор принудительно
  }

  // Сохранение оставшегося времени в EEPROM
  if (timerActive && currentMillis - lastSaveRemainingTimer >= SAVE_REMAINING_TIMER_TIME_INTERVAL){
    // Save remaining time of timer for next session in case of earlier machine stop        
    if (shouldSaveRemainingTime && timerStart > currentMillis){
      // Timer is active
      saveRemainingTimerTimeToEEPROM(timerStart - currentMillis);
    }
    lastSaveRemainingTimerTime = lastSaveRemainingTimer = currentMillis;
  }

  // Управление мотором
  controlMotor(false); // Выключаем мотор, если время истекло
}
#include <PinChangeInterrupt.h>

#define mosfet 3      // мосфет на мотор
#define forcedStartButton 4      // кнопка принудтельного пуска
#define timerButton 5     // кнопка таймера
#define floaterInterraption 6     // кнопка поплавка
#define oilOKLed 7    // зелена сигнальна лампа
#define oilEMPTYLed 8      // червно лампа, кінчилось масло

// unsigned long unsigned long time;
volatile bool floaterInterraptionState;
volatile bool forcedStartButtonState;
volatile bool timerButtonState;

void changeTimerButtonState(){
  timerButtonState = !timerButtonState;
}

void changeForcedStartState(){
  // // if (forcedStartButtonState){
  // digitalWrite(mosfet, HIGH); //включили реле
  // Serial.println("INTERRAPTION mosfet=on");
  // delayMicroseconds(1000); //ждем 6 секунд 19 милисекунд
  // digitalWrite(mosfet, LOW); //выключаем реле
  // Serial.println("INTERRAPTION mosfet=off");
  // // }
  // forcedStartButtonState = !forcedStartButtonState;
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  // примусове вімкнення наососа прям в таймері
  if (interruptTime - lastInterruptTime > 200) {
    forcedStartButtonState = !forcedStartButtonState;
  }
  lastInterruptTime = interruptTime;
}

void changeOilState(){
  // Serial.println("ABOBA");
  floaterInterraptionState = !floaterInterraptionState;
  // if(floaterInterraptionState == HIGH){
  //   digitalWrite(oilOKLed, LOW);
  //   digitalWrite(oilEMPTYLed, HIGH);
  // }
  // else {
  //   digitalWrite(oilOKLed, HIGH);
  //   digitalWrite(oilEMPTYLed, LOW);
  // }
  digitalWrite(oilOKLed, !floaterInterraptionState);
  digitalWrite(oilEMPTYLed, floaterInterraptionState);
}

void setup() {
  Serial.begin(9600);
  // initialize digital pin LED as an output.
  pinMode(mosfet, OUTPUT);
  pinMode(oilOKLed, OUTPUT);
  pinMode(oilEMPTYLed, OUTPUT);

  // pinMode(timerButton, INPUT);
  attachPCINT(digitalPinToPCINT(timerButton), changeTimerButtonState, CHANGE);
  // // initial state
  timerButtonState = digitalRead(timerButton);

  // pinMode(forcedStartButton, INPUT);
  // attachPCINT(digitalPinToPCINT(forcedStartButton), changeforcedStartState, HIGH);
  attachPCINT(digitalPinToPCINT(forcedStartButton), changeForcedStartState, CHANGE);
  // // initial state
  forcedStartButtonState = digitalRead(forcedStartButton);

  // pinMode(floaterInterraption, INPUT);
  // attachInterrupt(digitalPinToPCINT(floaterInterraption), changeOilState, CHANGE);
  attachPCINT(digitalPinToPCINT(floaterInterraption), changeOilState, CHANGE);
  // // initial state of floaterInterraption
  floaterInterraptionState = digitalRead(floaterInterraption);
  digitalWrite(oilOKLed, !floaterInterraptionState);
  digitalWrite(oilEMPTYLed, floaterInterraptionState);
}

void loop() {
  // // ввімкнення сигнальної лампи рівня масла
  // floaterInterraptionState = digitalRead(floaterInterraption);
  // if(floaterInterraptionState == HIGH){
  //   digitalWrite(oilOKLed, LOW);
  //   digitalWrite(oilEMPTYLed, HIGH);
  // }
  // else {
  //   digitalWrite(oilOKLed, HIGH);
  //   digitalWrite(oilEMPTYLed, LOW);
  // }
  // digitalWrite(oilOKLed, HIGH);
  // digitalWrite(oilEMPTYLed, LOW);

  //ввімкнення таймера
  // timerButtonState = digitalRead(timerButton);
  if (timerButtonState == HIGH) {
    for (int i=0; i<=5; i++) { //
      // forcedStartButtonState = digitalRead(forcedStartButton);
      // if (forcedStartButtonState == LOW);
      delay(1000);
      Serial.print(" i=");
      Serial.print(i);
      if (forcedStartButtonState == HIGH){ //ждем нажатия кнопки
        // примусове вімкнення наососа прям в таймері
        digitalWrite(mosfet, HIGH); //включили реле
        Serial.println(" mosfet=on");
        delay(6019); //ждем 6 секунд 19 милисекунд
        digitalWrite(mosfet, LOW); //выключаем реле
        Serial.print(" mosfet=off");
        i = 0;
      }
    } // end of timer
    //включили реле по закінченню таймера
    digitalWrite(mosfet, HIGH);
    Serial.println(" mosfet=on2");
    delay(6019);//ждем 6 секунд 19 милисекунд
    digitalWrite(mosfet, LOW);//выключаем реле
    Serial.print(" mosfet=off2");
  } else {
     //коли таймер виключен, перевіряємо примусове включення насоса
    if(forcedStartButtonState == HIGH){//ждем нажатия кнопки
        digitalWrite(mosfet, HIGH);//включили реле
        Serial.println(" mosfet=on3");
        delay(6019);//ждем 6 секунд 19 милисекунд
        digitalWrite(mosfet, LOW);//выключаем реле
        Serial.print(" mosfet=off3");
    }
  }
}
#include <DHT11.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
//라이브러리 포함

// DHT11 센서 핀 번호 설정
DHT11 dht11(8);

// RGB LED 핀 번호 설정
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

// 팬 모듈 핀 번호 설정
const int fanINAPin = 7;
const int fanINBPin = 6;

// 버튼 핀 번호 설정
const int buttonPin = 2;
bool isButtonPressed = false; // 버튼 상태를 저장하는 변수

// 블루투스 모듈 핀 번호 설정
const int bluetoothRxPin = 4;
const int bluetoothTxPin = 3;


SoftwareSerial bluetooth(bluetoothRxPin, bluetoothTxPin);

// I2C 주소 지정
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {

    Serial.begin(9600);
    bluetooth.begin(9600);

    // RGB LED 핀을 출력 모드로 설정
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    // 팬 모듈 핀을 출력 모드로 설정
    pinMode(fanINAPin, OUTPUT);
    pinMode(fanINBPin, OUTPUT);

    // 버튼 핀을 입력 모드로 설정
    pinMode(buttonPin, INPUT_PULLUP); // 내부 풀업 저항 사용

    // LCD 초기화
    lcd.begin();
    lcd.backlight();
    lcd.print("Initializing...");

    // 초기화 메시지 출력
    Serial.println("System Initialized");
}

void loop() {
    // 버튼 상태 확인 및 토글
    checkButton();

    // 블루투스 입력 확인 및 팬 제어
    checkBluetoothInput();

    if (isButtonPressed) {
        int temperature = 0;
        int humidity = 0;

        // DHT11 센서로부터 온도와 습도 값을 읽음
        int result = dht11.readTemperatureHumidity(temperature, humidity);

        // 온도 값을 시리얼 모니터와 LCD에 출력
        if (result == 0) {
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C\t");

            lcd.clear();
            lcd.setCursor(0, 0); // 첫 번째 줄 첫 번째 칸으로 커서 지정
            lcd.print("Temp: ");
            lcd.print(temperature);
            lcd.print(" C");

            // 온도에 따른 RGB LED 색깔 및 팬 속도 설정
            if (temperature <= 22) {
                // 파란색 출력 및 팬 정지
                setColor(0, 0, 255);
                setFanSpeed(0); // 팬 정지
                Serial.println("Fan OFF");
                lcd.setCursor(0, 1); // 두 번째 줄 첫 번째 칸으로 커서 지정
                lcd.print("Fan: OFF");
            } else if (temperature <= 27) {
                // 노란색 출력 및 팬 느리게 회전
                setColor(255, 255, 0);
                setFanSpeed(128); // 팬 느리게 회전 (50% 속도)
                Serial.println("Fan LOW");
                lcd.setCursor(0, 1); // 두 번째 줄 첫 번째 칸으로 커서 지정
                lcd.print("Fan: LOW");
            } else {
                // 빨간색 출력 및 팬 빠르게 회전
                setColor(255, 0, 0);
                setFanSpeed(255); // 팬 빠르게 회전 (100% 속도)
                Serial.println("Fan HIGH");
                lcd.setCursor(0, 1); // 두 번째 줄 첫 번째 칸으로 커서 지정
                lcd.print("Fan: HIGH");
            }
        } else {
            // 읽기 실패 시 에러 메시지를 시리얼 모니터에 출력
            Serial.println(DHT11::getErrorString(result));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Error reading");
            lcd.setCursor(0, 1);
            lcd.print("sensor");
        }
    } else {
        // 버튼이 눌리지 않았을 경우 LED와 팬을 끔
        setColor(0, 0, 0);
        setFanSpeed(0);
        Serial.println("System OFF");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System OFF");
    }

    // 0.5초 대기
    delay(500);
}

// 버튼 상태 확인 함수
void checkButton() {
    if (digitalRead(buttonPin) == LOW) {
        delay(50); // 안정적인 작동을 위해 약간의 지연시간 추가
        if (digitalRead(buttonPin) == LOW) {
            isButtonPressed = !isButtonPressed; // 버튼 상태 토글
            while (digitalRead(buttonPin) == LOW); // 버튼이 눌린 동안 대기
        }
    }
}

// 블루투스 입력 확인 및 팬 제어 함수
void checkBluetoothInput() {
    if (bluetooth.available() > 0) {
        char input = bluetooth.read();
        Serial.print("Received input: "); // 입력값 디버깅 메시지
        Serial.println(input);

        if (input == '1') {
            setFanSpeed(128); // 팬 느리게 회전 (50% 속도)
            Serial.println("Fan LOW (50%)");
        } else if (input == '2') {
            setFanSpeed(0); // 팬 정지
            Serial.println("Fan OFF");
        }
    }
}

// RGB LED 색상 설정 함수
void setColor(int redValue, int greenValue, int blueValue) {
    analogWrite(redPin, redValue);
    analogWrite(greenPin, greenValue);
    analogWrite(bluePin, blueValue);
}

// 팬 속도 설정 함수
void setFanSpeed(int speed) {
    if (speed == 0) {
        // 팬 정지
        digitalWrite(fanINAPin, LOW);
        digitalWrite(fanINBPin, LOW);
    } else {
        // 팬 회전 (속도 제어)
        analogWrite(fanINAPin, speed);
        digitalWrite(fanINBPin, LOW);
    }
}

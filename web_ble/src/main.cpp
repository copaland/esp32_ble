/*
ESP32 Web BLE 테스트 - BLE 전용

1. 로컬 웹 서버 시작: 터미널에서
cd e:\PlatformIO\Projects\web_ble\data; python -m http.server 8000

2. 브라우저에서
http://localhost:8000/

3.웹 서버는 백그라운드에서 실행 중입니다
중지하려면: 터미널에서 Ctrl+C
*/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pLedCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

const int RledPin = 38;
const int BledPin = 47;
const int potPin = 1;  // GPIO1 (ADC1_CH0) - POT 가변저항 연결

// 타이머 변수
hw_timer_t *timer = NULL;
volatile bool readPot = false;
volatile int potValue = 0;

#define SERVICE_UUID        "5cacdfe5-174d-45b9-950e-476e044d8240"
#define SENSOR_CHARACTERISTIC_UUID "5cacdfe6-174d-45b9-950e-476e044d8240"
#define LED_CHARACTERISTIC_UUID "5cacdfe7-174d-45b9-950e-476e044d8240"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pLedCharacteristic) {
    std::string value = pLedCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Characteristic event, written: ");
      Serial.println(static_cast<int>(value[0])); // Print the integer value

      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue == 1) {
        digitalWrite(RledPin, HIGH);
      } else if (receivedValue == 0) {
        digitalWrite(RledPin, LOW);
      } else if (receivedValue == 3) {
        digitalWrite(BledPin, HIGH);
      } else if (receivedValue == 2) {
        digitalWrite(BledPin, LOW);
      }
    }
  }
};

// 타이머 인터럽트 서비스 루틴 (ISR)
void IRAM_ATTR onTimer() {
  readPot = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(RledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  pinMode(potPin, INPUT);
  
  Serial.println("ESP32 BLE Server Starting...");
  
  // 타이머 설정 (100ms = 100,000 us)
  timer = timerBegin(0, 80, true);  // Timer 0, prescaler 80 (1MHz), count up
  timerAttachInterrupt(timer, &onTimer, true);  // Attach ISR
  timerAlarmWrite(timer, 100000, true);  // 100ms (100,000 microseconds), auto-reload
  timerAlarmEnable(timer);  // Enable timer

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pSensorCharacteristic = pService->createCharacteristic(
                      SENSOR_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create the ON button Characteristic
  pLedCharacteristic = pService->createCharacteristic(
                      LED_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Register the callback for the ON button characteristic
  pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pSensorCharacteristic->addDescriptor(new BLE2902());
  pLedCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  // POT 가변저항 값 읽기 (타이머 인터럽트 발생 시)
  if (readPot && deviceConnected) {
    readPot = false;
    
    // ADC 값 읽기 (0-4095)
    potValue = analogRead(potPin);
    
    // BLE로 전송
    pSensorCharacteristic->setValue(String(potValue).c_str());
    pSensorCharacteristic->notify();
    
    Serial.print("POT value notified: ");
    Serial.println(potValue);
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }
}
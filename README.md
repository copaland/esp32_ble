# esp32_ble

ESP32 Web BLE 테스트 - BLE 전용

1. 로컬 웹 서버 시작: 터미널에서
```cd e:\PlatformIO\Projects\web_ble\data; python -m http.server 8000```

2. 브라우저에서
```http://localhost:8000/```

3.웹 서버는 백그라운드에서 실행 중입니다
중지하려면: 터미널에서 Ctrl+C

4. LITTLEFS PLUGIN 
[LITTLEFS PLUGIN 설치](https://github.com/earlephilhower/arduino-littlefs-upload/releases)

C:\Users\<username>\.arduinoIDE\plugins 폴더에 arduino-littlefs-upload-1.6.0.vsix 넣어둠

[ESP32FS-1.1.zip](https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/)

## BLE UUID 기본 개념

```https://www.uuidgenerator.net/```
[Online UUID Generator Tool](https://www.uuidgenerator.net/)

#### UUID의 역할
Service UUID: GATT 서비스(기능 묶음)를 식별.
Characteristic UUID: 서비스 내의 데이터 항목(읽기/쓰기/알림 등)을 식별.
#### 길이
16-bit: Bluetooth SIG에서 표준으로 등록된 UUID(예: 0x180F = Battery Service).
128-bit: 커스텀 UUID(회사/프로젝트 전용). 주로 서비스·특성 이름 충돌을 피하려면 임의로 생성.
#### 형식: 8-4-4-4-12 (예: 5cacdfe5-174d-45b9-950e-476e044d8240)
#### UUID 버전/variant
제공하신 UUID들에서 중앙의 4자리(예: "45b9")의 첫 숫자가 '4' → UUID v4 (랜덤 기반). 다음 부분의 첫 비트는 RFC4122 variant(9, a, b 등) 조건을 만족.
#### 고유성
128-bit UUID는 사실상 충돌 가능성이 매우 낮으므로(특히 v4 임의 UUID) 로컬 커스텀 서비스/특성에 적합.
#### 전송/바이트 오더
내부/전송 관점에서 바이트 오더(엔디언)를 주의해야 하는 경우가 있음. 대부분의 고수준 BLE API(예: Android, iOS, Arduino BLE 라이브러리)는 문자열/UUID 객체를 그대로 받아 처리하므로 신경쓸 필요가 없음. 다만, 프로토콜 레벨(원시 바이트)로 직접 다룰 때 바이트 순서를 확인할 것.
## Service vs Characteristic 사용 권장(설계)

서비스: 장치의 기능 그룹(예: "Environmental Service", "LED Control Service")
센서 특성: 센서 값은 보통 Read + Notify 권장 (클라이언트가 subscribe하면 주기적/이벤트 기반으로 notify 전송)
LED 특성: 보통 Write (Write or WriteWithoutResponse). 쓰기 요청으로 LED ON/OFF 제어.
CCCD: notify/indicate를 쓰려면 클라이언트가 Client Characteristic Configuration Descriptor(디스크립터)를 통해 구독해야 함.
## 보안/권한

민감 데이터라면 Authentication(암호화), Authorization(권한), 보안 연결(LE Secure Connections) 고려.
특성 권한: readable, writable, notify, indicate 등 설정 가능.
## UUID 생성 방법

Linux: uuidgen 또는 uuidgen -r
Python: python -c "import uuid; print(uuid.uuid4())"
온라인 UUID 생성기 사용
생성 시 충돌 위험을 피하려면 UUID v4 사용(랜덤).

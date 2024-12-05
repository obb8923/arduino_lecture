// Arduino 핀 할당
#define PIN_LED  9
#define PIN_TRIG 12   // 초음파 센서 TRIGGER
#define PIN_ECHO 13   // 초음파 센서 ECHO

// 설정 가능한 파라미터
#define SND_VEL 346.0     // 24도에서의 음속 (단위: m/sec)
#define INTERVAL 25      // 샘플링 간격 (단위: msec)
#define PULSE_DURATION 10 // 초음파 펄스 지속 시간 (단위: usec)
#define _DIST_MIN 100.0   // 측정할 최소 거리 (단위: mm)
#define _DIST_MAX 300.0   // 측정할 최대 거리 (단위: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // 최대 에코 대기 시간 (단위: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // 지속 시간을 거리로 변환하는 계수

unsigned long last_sampling_time;   // 단위: msec

void setup() {
  // GPIO 핀 초기화
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // 초음파 센서 TRIGGER
  pinMode(PIN_ECHO, INPUT);   // 초음파 센서 ECHO
  digitalWrite(PIN_TRIG, LOW);  // 초음파 센서 꺼짐
  
  // 시리얼 포트 초기화
  Serial.begin(57600);
}

void loop() { 
  float distance;

  // 다음 샘플링 시간까지 대기 // 폴링
  // millis()는 프로그램 시작 후 경과된 밀리초를 반환함.
  // 50일 후 오버플로우 발생 가능.
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // 거리 측정

  if ((distance == 0.0) || (distance > _DIST_MAX)) {
      distance = _DIST_MAX + 10.0;    // 더 높은 값으로 설정
      digitalWrite(PIN_LED, 1);       // LED 꺼짐
  } else if (distance < _DIST_MIN) {
      distance = _DIST_MIN - 10.0;    // 더 낮은 값으로 설정
      digitalWrite(PIN_LED, 1);       // LED 꺼짐
  } else {    // 원하는 범위 내
      digitalWrite(PIN_LED, 0);       // LED 켜짐
  }

  // 시리얼 포트에 거리 출력
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.println("");
  
  // 여기에서 다른 작업 수행
  //delay(100); // 50ms 동안 작업이 수행된다고 가정.
  
  // 마지막 샘플링 시간 업데이트
  last_sampling_time += INTERVAL;
}

// 초음파 센서로부터 거리 측정. 반환 값은 밀리미터 단위.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // 단위: mm

  // 펄스 지속 시간을 거리로 변환하는 예시 (목표 거리 = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout)은 마이크로초를 반환 (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 변환하려면 * 1,000이 필요 ==>  SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예, pulseIn()이 100,000일 경우 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 마이크로초 * 0.001 밀리/마이크로 * 0.5 * 346 미터/초
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
}

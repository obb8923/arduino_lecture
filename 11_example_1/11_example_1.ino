#include <Servo.h>

// 핀 할당
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13
#define PIN_SERVO 11

// 초음파 센서 관련 설정 값
#define SND_VEL 346.0     // 음속 (단위: m/sec)
#define INTERVAL 25       // 샘플링 간격 (단위: msec)
#define PULSE_DURATION 10 // 초음파 펄스 지속 시간 (단위: usec)
#define _DIST_MIN 180.0   // 최소 측정 거리 (단위: mm, 18cm)
#define _DIST_MAX 360.0   // 최대 측정 거리 (단위: mm, 36cm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // 최대 에코 대기 시간 (단위: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // 시간(duration)을 거리로 변환하는 계수
#define _EMA_ALPHA 0.1    // EMA 필터 가중치

// 서보 모터 각도 범위
#define _DUTY_MIN 600  // 서보 0도
#define _DUTY_MAX 2400 // 서보 180도
#define _DUTY_NEU 1800
// 전역 변수
unsigned long last_sampling_time;  // 마지막 샘플링 시간 (단위: ms)
float dist_prev = _DIST_MAX;       // 이전 거리
float dist_ema = _DIST_MAX;        // EMA 필터링된 거리

Servo myservo;

void setup() {
  // GPIO 핀 초기화
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  
  // 서보 모터 초기화
  myservo.attach(PIN_SERVO); 
  myservo.writeMicroseconds(_DUTY_MIN); // 서보 초기값 0도
  
  // 시리얼 통신 초기화
  Serial.begin(57600);
}

void loop() {
  float dist_raw;
  
  // 다음 샘플링 시간까지 대기
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // 범위 필터 적용: 측정 값이 _DIST_MIN 이하이거나 _DIST_MAX 이상이면 이전 값을 사용
  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) {
    dist_raw = dist_prev;
    digitalWrite(PIN_LED, HIGH);  // LED OFF
  } else if (dist_raw < _DIST_MIN) {
    dist_raw = dist_prev;
    digitalWrite(PIN_LED, HIGH);  // LED OFF
  } else {
    digitalWrite(PIN_LED, LOW); // 범위 안에서는 LED ON
    dist_prev = dist_raw;        // 유효한 범위일 때 값 저장
  }

  // EMA 필터 적용
  dist_ema = _EMA_ALPHA * dist_raw + (1 - _EMA_ALPHA) * dist_ema;

  // 거리 범위에 따른 서보 제어
  int servo_angle;
  if (dist_ema <= _DIST_MIN) {
    servo_angle = 0;  // 거리 18cm 이하: 0도
  } else if (dist_ema >= _DIST_MAX) {
    servo_angle = 180; // 거리 36cm 이상: 180도
  } else {
    // 거리 18cm ~ 36cm 사이: 거리에 비례하여 0도 ~ 180도
    servo_angle = map(dist_ema, _DIST_MIN, _DIST_MAX, 0, 180);
  }
  
  // 서보 모터 제어
  myservo.write(servo_angle);

  // 시리얼 출력 (플로터용)
  Serial.print("Min:");   
  Serial.print(_DIST_MIN);
  Serial.print(",dist:");  
  Serial.print(dist_raw);
  Serial.print(",ema:");   
  Serial.print(dist_ema);
  Serial.print(",Servo:"); 
  Serial.print(servo_angle);
  Serial.print(",Max:");   
  Serial.print(_DIST_MAX);
  Serial.println("");

  // 마지막 샘플링 시간 업데이트
  last_sampling_time += INTERVAL;
}

// 초음파 센서로 거리 측정. 반환 값은 밀리미터 단위.
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // 단위: mm
}

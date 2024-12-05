#include <Servo.h>

// 아두이노 핀 할당
#define PIN_LED 9
#define PIN_TRIG  12  // 초음파 센서 TRIGGER
#define PIN_ECHO  13  // 초음파 센서 ECHO
#define PIN_SERVO 11

// 초음파 센서를 위한 설정 가능한 매개변수
#define SND_VEL 346.0     // 24도에서의 음속 (단위: m/sec)
#define PULSE_DURATION 10 // 초음파 펄스 지속 시간 (단위: usec)
#define _DIST_MIN 100.0   // 측정 가능한 최소 거리 (단위: mm)
#define _DIST_MAX 300.0   // 측정 가능한 최대 거리 (단위: mm)

// 목표 거리
#define _TARGET_LOW  180.0
#define _TARGET_HIGH 220.0

#define _DUTY_MIN 600  // 서보 모터 시계 방향 끝 위치 (0도)
#define _DUTY_MAX 2400 // 서보 모터 반시계 방향 끝 위치 (180도)
#define _DUTY_NEU (_DUTY_MAX + _DUTY_MIN)/2 // 서보 모터 중립 위치 (90도)

#define _INTERVAL_DIST    50 // 초음파 센서 간격 (단위: msec)
#define _INTERVAL_SERVO   50 // 서보 모터 간격 (단위: msec)
#define _INTERVAL_SERIAL  500 // 시리얼 통신 간격 (단위: msec)

#define TIMEOUT ((_INTERVAL_DIST / 2) * 1000.0) // 최대 에코 대기 시간 (단위: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // 지속 시간을 거리로 변환하는 계수

// 전역 변수

Servo myservo;

unsigned long last_sampling_time_dist;   // 단위: msec
unsigned long last_sampling_time_servo;  // 단위: msec
unsigned long last_sampling_time_serial; // 단위: msec

bool event_dist, event_servo, event_serial; // 이벤트 발생 여부

float dist_prev = _DIST_MIN;   // 초음파 센서 거리 필터

void setup() {
  // GPIO 핀 초기화
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  digitalWrite(PIN_TRIG, LOW); 
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_LED, 1); // LED 켜기

  myservo.attach(PIN_SERVO); 
  myservo.writeMicroseconds(_DUTY_NEU); // 서보 중립 위치
  
  // 시리얼 포트 초기화
  Serial.begin(57600);  
}
  
void loop() {
  float dist_raw;
  unsigned long time_curr = millis();
  
  // 다음 이벤트 시간이 될 때까지 대기
  if (time_curr >= (last_sampling_time_dist + _INTERVAL_DIST)) {
    last_sampling_time_dist += _INTERVAL_DIST;
    event_dist = true;
  }
  if (time_curr >= (last_sampling_time_servo + _INTERVAL_SERVO)) {
    last_sampling_time_servo += _INTERVAL_SERVO;
    event_servo = true;
  }
  if (time_curr >= (last_sampling_time_serial + _INTERVAL_SERIAL)) {
    last_sampling_time_serial += _INTERVAL_SERIAL;
    event_serial = true;
  }
    
  if (event_dist) {
    event_dist = false;
    // 초음파 센서에서 거리 값을 읽어옴
    dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);
    // 거리 필터 적용
    if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
        dist_raw = dist_prev;
    else
        dist_prev = dist_raw;
  }
  
  if (event_servo) {
    event_servo = false;
    // 초음파 센서로 읽은 값에 따라 서보 위치 조정
    if (dist_raw < _TARGET_LOW) {
      myservo.writeMicroseconds(_DUTY_MIN); // 서보를 최소 위치로 이동
      digitalWrite(PIN_LED, 0); // LED 켜기
    } else if (dist_raw > _TARGET_HIGH) {
      myservo.writeMicroseconds(_DUTY_MAX); // 서보를 최대 위치로 이동
      digitalWrite(PIN_LED, 0); // LED 켜기
    } else {
      myservo.writeMicroseconds(_DUTY_NEU); // 서보를 중립 위치로 이동
      digitalWrite(PIN_LED, 1); // LED 끄기
    }
  }
  
  if (event_serial) {
    event_serial = false;
    // 읽은 값을 시리얼 포트에 출력
    Serial.print("Min:");    Serial.print(_DIST_MIN);
    Serial.print(",Low:");   Serial.print(_TARGET_LOW);
    Serial.print(",raw:");   Serial.print(dist_raw);
    Serial.print(",servo:"); Serial.print(myservo.read());  
    Serial.print(",High:");  Serial.print(_TARGET_HIGH);
    Serial.print(",Max:");   Serial.print(_DIST_MAX);
    Serial.println("");
  }
}

// 초음파 센서로부터 거리 값을 읽어옴. 반환 값은 mm 단위.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // 단위: mm
}

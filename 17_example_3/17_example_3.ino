#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    A0         // IR 센서 연결 핀 (A0)
#define PIN_LED   9          // LED 연결 핀
#define PIN_SERVO 10         // 서보 모터 연결 핀

// 서보 제어 각도와 거리 범위 정의
#define _DUTY_MIN 600       // 서보 최소 각도에 해당하는 펄스 폭 (0도)
#define _DUTY_MAX 2400      // 서보 최대 각도에 해당하는 펄스 폭 (180도)
#define _DIST_MIN  40.0    // 최소 거리 (10cm)
#define _DIST_MAX  250.0    // 최대 거리 (25cm)

#define EMA_ALPHA  0.2       // EMA 필터의 알파 값 (조정 가능)
#define INTERVAL 20          // 루프 주기 (20밀리초 이상 유지)

Servo myservo;
unsigned long last_loop_time = 0;

float dist_prev = _DIST_MIN; // 이전 거리 값을 저장
float dist_ema = _DIST_MIN;  // EMA 필터로 평활화된 거리 값

void setup()
{
  pinMode(PIN_LED, OUTPUT);        // LED 핀을 출력 모드로 설정
  myservo.attach(PIN_SERVO);       // 서보 모터 핀 연결
  myservo.writeMicroseconds(_DUTY_MIN);  // 서보 모터를 초기 위치(0도)로 설정
  
  Serial.begin(1000000);           // 시리얼 통신 시작 (속도: 1000000)
}

void loop()
{
  unsigned long time_curr = millis();
  float a_value, dist_raw;
  int duty;

  // 일정 시간(INTERVAL)마다 코드 실행
  if (time_curr < (last_loop_time + INTERVAL))
    return;
  last_loop_time += INTERVAL;

  // A0 핀에서 IR 센서 값 읽기
  a_value = analogRead(PIN_IR);
  
  // IR 센서 값을 거리(mm)로 변환
  dist_raw = (6762.0 / (a_value - 9.0) - 4.0) * 10.0 - 60.0;
  
  // 범위 필터: 거리 값이 설정된 범위 내에 있을 때만 LED 켜기
  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX) {
    digitalWrite(PIN_LED, LOW);   // 거리 범위 내에 있을 때 LED 켜기
  } else {
    digitalWrite(PIN_LED, HIGH);    // 거리 범위 밖일 때 LED 끄기
  }

  // EMA 필터 적용하여 거리 값 평활화
  dist_ema = EMA_ALPHA * dist_raw + (1 - EMA_ALPHA) * dist_prev;
  dist_prev = dist_ema;

  // 거리 값을 서보 각도로 변환 (0~180도) - 직접 계산으로 map() 대체
  double x = ( (470 - a_value) * 1023 ) / 270;
  duty = ( (_DUTY_MAX - _DUTY_MIN) / 1023 ) * x + _DUTY_MIN;

  // 서보 위치 설정
  myservo.writeMicroseconds(duty);

  // 시리얼 모니터 출력 (시리얼 플로터용 포맷)
  Serial.print("IR:");          Serial.print(a_value);   // 원본 IR 센서 값
  Serial.print(",dist_raw:");   Serial.print(dist_raw);  // 변환된 원본 거리 값
  Serial.print(",ema:");        Serial.print(dist_ema);  // 필터링된 거리 값
  Serial.print(",servo:");      Serial.print(duty);      // 서보 제어 펄스 폭
  Serial.print(",_DIST_MAX:");   Serial.print(_DIST_MAX);      
  Serial.print(",_DIST_MIN:");   Serial.print(_DIST_MIN); 
  Serial.println("");
}

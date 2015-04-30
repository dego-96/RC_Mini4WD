/******************************************
 * 
 * ステアリング作動範囲調整用スケッチ
 * 
 * 使用サーボ：Mini-S RB50
 * 動作可能範囲：±70度程度（0～170）
 * 
 * サーボモータ調整用I/F：シリアル通信
 * 
 ******************************************/

#include <Servo.h>

/* ピン番号 */
#define PIN_SERVO (9)
#define PIN_MOTOR1 (5)
#define PIN_MOTOR2 (6)
// */

/* サーボモータ制御用変数 */
Servo steerServo;
int initPosition;
// */

/* シリアル通信用変数 */
char input[4];
int count = 0;
// */

void setup() {
  steerServo.attach(PIN_SERVO);

  Serial.begin(9600);

  initPosition = 70;
}

void loop() {
  int len = Serial.available();
  if (len > 0) {
    Serial.print("size : ");
    Serial.print(len, DEC);
    Serial.print("\n");

    int val = getValue(len);

    if (val > 0) {
      steerServo.write(val);
    }

    Serial.end();
    delay(20);
    Serial.begin(9600);
  }

  delay(20);
}

int getValue(int len) {
  int res = 0;

  if (len > 4) {
    Serial.println("size error.");
    return 0;
  }

  for (int i = 0; i < len; i++) {
    input[i] = Serial.read();
  }
  res = atoi(input);
  Serial.print("result : ");
  Serial.print(res, DEC);
  Serial.println("");

  return res;
}


/*****************************************
 * ラジコン化ミニ四駆
 * テスト用プログラム
 * version : 0.1
 *
 * [通信I/F]
 * 'h' + 2桁整数(サーボ用)
 *     + 1桁整数(進行方向、0が前進、1が後進)
 *     + 3桁整数(DCモータ用)
 *   ※例：h601255
 ****************************************/

/* デバッグモードのコンパイルSW */
#define DEBUG_MODE  // 定義時はデバッグモードが有効
// */

#include <Servo.h>

/******** ピン番号 ********/
#define PIN_SERVO (9)
#define PIN_MOTOR1 (5)
#define PIN_MOTOR2 (6)
// ************************/

/* Bluetooth通信用変数 */
#define DATA_LENGTH (7)
char inputData[DATA_LENGTH];
// */

/******** サーボモータ制御用変数 ********/
//#define STEER_MAX (10)     // +60度
#define STEER_CENTER (55)  //   0度
//#define STEER_MIN (130)    // -60度

Servo steerServo;
int lastPosition;   // 30-80
// **************************************/

/******** DCモータ制御用変数 ********/
#define MOTOR_STOP (0)
int lastMotorSpeed;  // 0-255
int lastWay;  // 0 or 1
// **********************************/

/******** 共通変数 ********/
int delayTime = 50; // 10ms

#if defined(DEBUG_MODE)
int debug_delayTime = 4000;
#endif
// ************************/

// =================================================================================

void setup() {
  // Bluetoothの初期化
  initBluetoothSetting();

  // サーボモータの初期化
  initServoSetting();

  // DCモータの初期化
  initDCMotorSetting();
}

void loop() {
  // ローカル変数の宣言
  int pos, spd, way;

  // 通信データの取得
  receiveData();  // inputData[] に値を格納

  // 各値の取得
  pos = getSteerPosition();
  spd = getMotorSpeed();
  way = getWay();

  if (pos < 0 || spd < 0 || way < 0) {   // データ取得できなかった場合
    pos = lastPosition;
    spd = lastMotorSpeed;
    way = lastWay;
  }

  // マシンの動作を制御
  controllSteerServo(pos);
  controllDCMotor(spd, way);

  // 前回値処理
  lastPosition = pos;
  lastMotorSpeed = spd;
  lastWay = way;

  delay(delayTime);
}

// -------- 初期化関数 ----------------------------------------------

/*
 * Bluetooth通信用の初期化処理
 */
void initBluetoothSetting() {
  Serial.begin(9600);
}
// */

/*
 * サーボモータ用の初期化処理
 */
void initServoSetting() {
  steerServo.attach(PIN_SERVO);

  // 前回値の初期状態は 中心
  lastPosition = STEER_CENTER;
}
// */

/*
 * DCモータ用の初期化処理
 */
void initDCMotorSetting() {
  // ピンモードを設定
  pinMode(PIN_MOTOR1, OUTPUT);
  pinMode(PIN_MOTOR2, OUTPUT);

  // 前回値の初期状態は 停止
  lastMotorSpeed = MOTOR_STOP;
  lastWay = 0; // 前進
}
// */



// -------- Getter --------------------------------------------------

/*
 * byteデータからサーボモータ制御用の値を取得
 */
int getSteerPosition() {
  char inputServo[2];
  inputServo[0] = inputData[1];
  inputServo[1] = inputData[2];

  // 値の取得
  int pos = atoi(inputServo);

  // エラーケース
  if (pos == 0) {
    return -1;
  }

  // 上下限ガード (25-85)
  if (pos < 25) {
    pos = 25;
  }
  if (pos > 85) {
    pos = 85;
  }

#if defined(DEBUG_MODE)
  Serial.print("get value (servo) : ");
  Serial.print(pos, DEC);
  Serial.println("");
#endif

  return pos;
}
// */

/*
 * byteデータからDCモータ制御用の値を取得
 */
int getMotorSpeed() {
  char inputDCMotor[3];
  inputDCMotor[0] = inputData[4];
  inputDCMotor[1] = inputData[5];
  inputDCMotor[2] = inputData[6];

  // 値の取得
  int spd = atoi(inputDCMotor);

  // エラーケース
  if (spd == 0) {
    return -1;
  }

  // 上下限ガード (0-255)
  if (spd < 0) {
    spd = 0;
  }
  if (spd > 255) {
    spd = 255;
  }

#if defined(DEBUG_MODE)
  Serial.print("get value (dc motor) : ");
  Serial.print(spd, DEC);
  Serial.println("");
#endif

  return spd;
}
// */

/*
 * byteデータから進行方向の値を取得
 */
int getWay() {
  char inputWay[1];
  inputWay[0] = inputData[3];
  int way = atoi(inputWay);

  // 範囲外処理  ※文字の場合は直進と同じ扱い
  if (way > 1) {
    way = 0;
  }

  return way;
}



// -------- 制御用関数 ---------------------------------------------

/*
 * シリアル通信で制御用のデータを取得
 */
boolean receiveData() {
  // 受信してない場合
  if (Serial.available() < 0) {
    inputData[0] = -1;
    inputData[1] = -1;
    inputData[2] = -1;
    inputData[3] = -1;
    inputData[4] = -1;
    inputData[5] = -1;
    inputData[6] = -1;
    return false;
  }

  // ローカル変数の宣言
  char header;
  char input1, input2, input3, input4, input5, input6;

  // ヘッダー(h)の取得
  header = Serial.read();
  if (header != 'h') {
    return false;
  }
  for (int i = 1; i < DATA_LENGTH; i++) {
    char tmp = Serial.read();
    if (tmp < 0) {
      return false;
    }
    inputData[i] = tmp;
  }

#if defined(DEBUG_MODE)
  Serial.print(header);
  Serial.print(inputData[0]);
  Serial.print(inputData[1]);
  Serial.print(inputData[2]);
  Serial.print(inputData[3]);
  Serial.print(inputData[4]);
  Serial.print(inputData[5]);
  Serial.print(inputData[6]);
  Serial.println("");
#endif

  return true;
}

/*
 * サーボモータを制御
 */
void controllSteerServo(int pos) {
  steerServo.write(pos);

#if defined(DEBUG_MODE)
  if (Serial.available() > 0) {
    Serial.print("set steer servo : ");
    Serial.print(pos, DEC);
    Serial.println("");
  }
#endif
}
// */

/*
 * DCモータを制御
 */
void controllDCMotor(int spd, int way) {
  if (spd == 0) {  // 停止
    digitalWrite(PIN_MOTOR1, LOW);
    digitalWrite(PIN_MOTOR2, LOW);
    return;
  }
  if (way == 0) {  // 前進
    analogWrite(PIN_MOTOR1, spd);
    analogWrite(PIN_MOTOR2, LOW);
  }
  else {  // 後進
    analogWrite(PIN_MOTOR1, LOW);
    analogWrite(PIN_MOTOR2, spd);
  }

#if defined(DEBUG_MODE)
  if (Serial.available() > 0) {
    Serial.print("set dc motor speed : ");
    Serial.print(spd, DEC);
    Serial.println("");
  }
#endif
}
// */



#include <PDM.h>                    // Microphone
#include <Arduino_APDS9960.h>       // Ambient Light + Proximity
#include <Arduino_BMI270_BMM150.h>  // IMU

short sampleBuffer[256];
volatile int samplesRead = 0;

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}

int audioActivity() {
  static int level = 0;
  if (samplesRead) {
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
      sum += abs(sampleBuffer[i]);
    }
    level = sum / samplesRead;
    samplesRead = 0;
  }
  return level;
}

int ambientBrightness() {
  static int r, g, b, c;
  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b, c);
  }
  return c;
}

int presence() {
  static int proximity = 0;
  if (APDS.proximityAvailable()) {
    proximity = APDS.readProximity();
    // Serial.print("Proximity: ");
    // Serial.println(proximity); // Anything less than 100 = NEAR
  }
  return proximity;
}

bool motion() {
  static float old_x = 0;
  static float old_y = 0;
  static float old_z = 0;
  static bool firstReading = true;
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    if (firstReading) {
      old_x = x;
      old_y = y;
      old_z = z;
      firstReading = false;
      return false;
    }
    float changeX = abs(x - old_x);
    float changeY = abs(y - old_y);
    float changeZ = abs(z - old_z);
    old_x = x;
    old_y = y;
    old_z = z;
    if (changeX > 0.10 || changeY > 0.10 || changeZ > 0.10) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}

void situation() {
  int   audio       = audioActivity();
  int   brightness  = ambientBrightness();
  int   proximity    = presence();
  bool  movement      = motion();

  static int audioFlag = 0;
  static int brightnessFlag = 0;
  static int proximityFlag = 0;
  static int movementFlag = 0;
  if (audio > 30) {
    audioFlag = 1;
  } else {
    audioFlag = 0;
  }

  if (brightness < 25) {
    brightnessFlag = 1;
  } else {
    brightnessFlag = 0;
  }

  if (proximity < 100) {
    proximityFlag = 1;
  } else {
    proximityFlag = 0;
  }

  if (movement == true) {
    movementFlag = 1;
  } else {
    movementFlag = 0;
  }
  Serial.print("raw,mic=<");
  Serial.print(audio);
  Serial.print(">,clear=<");
  Serial.print(brightness);
  Serial.print(">,motion=<");
  Serial.print(movement);
  Serial.print(">,prox=<");
  Serial.print(proximity);
  Serial.println(">");

  Serial.print("flags,sound=<");
  Serial.print(audioFlag);
  Serial.print("/1>,dark=<");
  Serial.print(brightnessFlag);
  Serial.print("/1,moving=<");
  Serial.print(movementFlag);
  Serial.print("/1>,near=<");
  Serial.print(proximityFlag);
  Serial.println("/1>");

  if (audioFlag && !brightnessFlag && !movementFlag && !proximityFlag) {
    Serial.println("state,<NOISY_BRIGHT_STEADY_FAR>");
  } else if (!audioFlag && brightnessFlag && !movementFlag && proximityFlag) {
    Serial.println("state,<QUIET_DARK_STEADY_NEAR>");
  } else if (audioFlag && !brightnessFlag && movementFlag && proximityFlag) {
    Serial.println("state,<NOISY_BRIGHT_MOVING_NEAR>");
  } else if (!audioFlag && !brightnessFlag && !movementFlag && !proximityFlag){
    Serial.println("state,<QUIET_BRIGHT_STEADY_FAR>");
  }
  delay(10000);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  PDM.onReceive(onPDMdata);
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM Microphone.");
    while(1);
  }
  if (!APDS.begin()) {
    Serial.println("Failed to start APDS sensor.");
    while (1);
  }
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU.");
    while(1);
  }
}

void loop() {
  situation();
}
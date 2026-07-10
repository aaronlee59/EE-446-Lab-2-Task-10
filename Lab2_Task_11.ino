#include <Arduino_APDS9960.h>
#include <Arduino_BMI270_BMM150.h>
#include <Arduino_HS300x.h>
#include <math.h>

void situation() {
  float temperature = HS300x.readTemperature();
  float humidity = HS300x.readHumidity();

  static int r = 0;
  static int g = 0;
  static int b = 0;
  static int c = 0;

  static float x = 0.0;
  static float y = 0.0;
  static float z = 0.0;

  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b, c);
  }

  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(x, y, z);
  }

  float mag = sqrt(x * x + y * y + z * z);

  static float oldHumidity = humidity;
  static float oldTemperature = temperature;
  static float oldMag = mag;
  static int oldClear = c;

  int humidFlag = 0;
  int tempFlag = 0;
  int magFlag = 0;
  int lightFlag = 0;

  if (humidity - oldHumidity > 3.0) {
    humidFlag = 1;
  }

  if (temperature - oldTemperature > 1.5) {
    tempFlag = 1;
  }

  if (fabs(mag - oldMag) > 15.0) {
    magFlag = 1;
  }

  if (abs(c - oldClear) > 100) {
    lightFlag = 1;
  }

  // First output line
  Serial.print("raw,rh=<");
  Serial.print(humidity);
  Serial.print(">,temp=<");
  Serial.print(temperature);
  Serial.print(">,mag=<");
  Serial.print(mag);
  Serial.print(">,r=<");
  Serial.print(r);
  Serial.print(">,g=<");
  Serial.print(g);
  Serial.print(">,b=<");
  Serial.print(b);
  Serial.print(">,clear=<");
  Serial.print(c);
  Serial.println(">");

  // Second output line
  Serial.print("flags,humid_jump=<");
  Serial.print(humidFlag);
  Serial.print("/1>,temp_rise=<");
  Serial.print(tempFlag);
  Serial.print("/1>,mag_shift=<");
  Serial.print(magFlag);
  Serial.print("/1>,light_or_color_change=<");
  Serial.print(lightFlag);
  Serial.println("/1>");

  // Third output line
  static bool eventTriggered = false;

  if (!eventTriggered) {
    if (magFlag) {
      Serial.println("event,<MAGNETIC_DISTURBANCE_EVENT>");
      eventTriggered = true;
    } else if (lightFlag) {
      Serial.println("event,<LIGHT_OR_COLOR_CHANGE_EVENT>");
      eventTriggered = true;
    } else if (humidFlag || tempFlag) {
      Serial.println("event,<BREATH_OR_WARM_AIR_EVENT>");
      eventTriggered = true;
    } else {
      Serial.println("event,<BASELINE_NORMAL>");
    }
  } else {
    Serial.println("event,<BASELINE_NORMAL>");
  }

  // Reset debounce when all conditions return to normal.
  if (!humidFlag && !tempFlag && !magFlag && !lightFlag) {
    eventTriggered = false;
  }

  oldHumidity = humidity;
  oldTemperature = temperature;
  oldMag = mag;
  oldClear = c;

  delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  if (!APDS.begin()) {
    Serial.println("Failed to start APDS sensor.");
    while (1) {
    }
  }

  if (!HS300x.begin()) {
    Serial.println("Failed to start HS3003 sensor.");
    while (1) {
    }
  }

  if (!IMU.begin()) {
    Serial.println("Failed to start IMU.");
    while (1) {
    }
  }
}

void loop() {
  situation();
}

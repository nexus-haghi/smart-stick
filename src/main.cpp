#include <Arduino.h>

// ============================================================================
//  Smart Cane v1.0 — Ultrasonic obstacle-detection cane for the visually impaired
// ============================================================================
//  Three HC-SR04 ultrasonic sensors scan three directions:
//      FRONT  → obstacles directly ahead
//      SIDE   → walls / obstacles to the left (handle side)
//      TOP    → low ceilings, branches, overhead hazards
//  Each sensor drives its own vibration motor mounted on the handle. When an
//  object is detected, the motor on that side vibrates — and the vibration gets
//  STRONGER the closer the object is. The user feels the direction and the
//  proximity through the handle.
//
//  Pins (Arduino UNO):
//      Sensor   TRIG  ECHO  Motor(PWM)
//      FRONT    D2    D3    D9
//      SIDE     D4    D5    D10
//      TOP      D6    D7    D11
//      Motors are driven through NPN transistors (2N2222 / BC547) + flyback
//      diodes — never straight from the pin. See WIRING.md.
// ============================================================================

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------
static const uint8_t  SAMPLE_COUNT  = 3;     // readings per sensor, median used
static const float     NEAR_CM       = 15.0f; // distance where vibration = 100%
static const float     FAR_CM        = 200.0f;// beyond this the motor is off
static const uint32_t  SENSOR_GAP_MS = 30;    // pause between sensors (avoid echo cross-talk)
static const uint32_t  LOOP_DELAY_MS = 20;    // loop breathing room

// ----------------------------------------------------------------------------
// Sensor mapping
// ----------------------------------------------------------------------------
enum SensorIndex {
    SENSOR_FRONT = 0,
    SENSOR_SIDE,
    SENSOR_TOP,
    NUM_SENSORS
};

struct Sensor {
    uint8_t trig;
    uint8_t echo;
    uint8_t motorPin;      // PWM pin driving this sensor's motor
    const char* name;      // for serial debug
};

static const Sensor sensors[NUM_SENSORS] = {
    { 2,  3,  9,  "FRONT" },
    { 4,  5,  10, "SIDE " },
    { 6,  7,  11, "TOP  " },
};

// ----------------------------------------------------------------------------
// Median filter — a single spurious echo cannot yank the reading.
// ----------------------------------------------------------------------------
static float medianOf3(float a, float b, float c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
    if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
    return c;
}

// ----------------------------------------------------------------------------
// Fire one ultrasonic ping and return distance in cm (-1 on no echo).
// ----------------------------------------------------------------------------
static float measureCm(const Sensor& s) {
    digitalWrite(s.trig, LOW);
    delayMicroseconds(2);
    digitalWrite(s.trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(s.trig, LOW);

    // 30 ms timeout ≈ 5 m max range
    unsigned long duration = pulseIn(s.echo, HIGH, 30000UL);
    if (duration == 0) {
        return -1.0f;          // no echo within timeout
    }
    return duration * 0.017f;  // cm (round-trip: 0.034 cm/µs / 2)
}

// ----------------------------------------------------------------------------
// Measure a sensor SAMPLE_COUNT times and return the median.
// ----------------------------------------------------------------------------
static float readFilteredCm(const Sensor& s) {
    float a = measureCm(s);
    float b = measureCm(s);
    float c = measureCm(s);
    return medianOf3(a, b, c);
}

// ----------------------------------------------------------------------------
// Distance → vibration strength. Closer object ⇒ stronger buzz.
// ----------------------------------------------------------------------------
static uint8_t intensityFromCm(float cm) {
    if (cm < 0.0f) return 0;             // no valid echo → off
    if (cm <= NEAR_CM) return 255;       // very close → full power
    if (cm >= FAR_CM)  return 0;         // beyond range → off

    float t = (cm - NEAR_CM) / (FAR_CM - NEAR_CM);   // 0..1 far→near
    return (uint8_t)(255.0f * (1.0f - t) + 0.5f);
}

// ----------------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
    Serial.println(F("Smart Cane v1.0 ready"));

    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        pinMode(sensors[i].trig, OUTPUT);
        pinMode(sensors[i].echo, INPUT);
        pinMode(sensors[i].motorPin, OUTPUT);
        digitalWrite(sensors[i].motorPin, LOW);
    }
}

// ----------------------------------------------------------------------------
void loop() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        const Sensor& s = sensors[i];

        float cm = readFilteredCm(s);
        uint8_t pwm = intensityFromCm(cm);
        analogWrite(s.motorPin, pwm);

        // Live distance readout — great for demo / tuning on the serial monitor.
        Serial.print(s.name);
        Serial.print(F(": "));
        if (cm < 0) Serial.print(F("no-echo"));
        else {
            Serial.print(cm, 1);
            Serial.print(F("cm"));
        }
        Serial.print(F("  motor="));
        Serial.println(pwm);

        delay(SENSOR_GAP_MS);
    }
    delay(LOOP_DELAY_MS);
}

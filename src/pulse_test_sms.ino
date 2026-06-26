#include <HardwareSerial.h>

HardwareSerial sim800(1); // UART1

// ===== Modem pins =====
#define MODEM_TX      27
#define MODEM_RX      26
#define MODEM_PWRKEY  4
#define MODEM_POWER_ON 23
#define MODEM_RESET   5

// ===== Pulse sensor =====
#define PULSE_PIN 35

// Filter parameters
float filtered = 0;
float avgLevel = 0;
bool firstSample = true;

const float alpha = 0.2;   // low-pass filter for signal
const float beta  = 0.01;  // slow average for baseline

// Beat detection
int thresholdOffset = 30;  // adjust if needed
bool aboveThreshold = false;
unsigned long lastBeatTime = 0;

// Result variables
float lastBPM = 0.0;
bool bpmValid = false;

const char* PHONE_NUMBER = "+918105215353";  // your number

// ===== Modem helpers =====
void powerOnModem() {
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  pinMode(MODEM_RESET, OUTPUT);

  digitalWrite(MODEM_POWER_ON, HIGH);
  digitalWrite(MODEM_RESET, HIGH);

  Serial.println("Powering on modem...");
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(5000);
}

void sendAT(const char* cmd, uint16_t waitMs = 1000) {
  sim800.print(cmd);
  sim800.print("\r");
  Serial.print(">> ");
  Serial.println(cmd);

  unsigned long start = millis();
  while (millis() - start < waitMs) {
    while (sim800.available()) {
      Serial.write(sim800.read());
    }
  }
  Serial.println();
}

bool waitForNetwork(uint32_t timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    sim800.print("AT+CREG?\r");
    delay(500);

    String resp = "";
    unsigned long t = millis();
    while (millis() - t < 500) {
      if (sim800.available()) {
        resp += (char)sim800.read();
      }
    }
    Serial.print(resp);

    if (resp.indexOf(",1") != -1 || resp.indexOf(",5") != -1) {
      Serial.println("Network registered OK");
      return true;
    }
    delay(1000);
  }
  return false;
}

void sendSMS(const char* number, const char* text) {
  Serial.println("Preparing to send SMS...");

  sendAT("AT+CMGF=1");  // text mode

  // Set recipient
  sim800.print("AT+CMGS=\"");
  sim800.print(number);
  sim800.print("\"\r");
  Serial.print(">> AT+CMGS=\"");
  Serial.print(number);
  Serial.println("\"");
  delay(1000); // wait for '>'

  // Message body
  sim800.print(text);
  Serial.print(">> ");
  Serial.println(text);
  delay(1000);

  // Ctrl+Z to send
  sim800.write(26);
  Serial.println("Sent Ctrl+Z, waiting for modem to send SMS...");

  // Read response for a while
  unsigned long start = millis();
  while (millis() - start < 15000) {
    while (sim800.available()) {
      Serial.write(sim800.read());
    }
  }
}

// ===== Pulse measurement (10 seconds) =====
void measureHeartRate10sec() {
 

  const unsigned long measureTime = 10000; // 10 seconds
  unsigned long startTime = millis();
  int beatCount = 0;

  Serial.println("Measuring for 10 seconds...");
  delay(100);

  // Initialize filter with first read
  int raw = analogRead(PULSE_PIN);
  filtered = raw;
  avgLevel = raw;
  firstSample = false;
  aboveThreshold = false;
  lastBeatTime = 0;

  while (millis() - startTime < measureTime) {
    unsigned long now = millis();
    raw = analogRead(PULSE_PIN);

    // Low-pass filter
    filtered = alpha * raw + (1.0 - alpha) * filtered;

    // Slow moving average for baseline
    avgLevel = avgLevel + beta * (filtered - avgLevel);

    float threshold = avgLevel + thresholdOffset;

    bool nowAbove = (filtered > threshold);

    // Rising edge detection
    if (!aboveThreshold && nowAbove) {
      unsigned long interval = now - lastBeatTime;

      // Ignore first beat interval or impossible values
      if (lastBeatTime == 0 || (interval > 300 && interval < 2000)) {
        beatCount++;
        lastBeatTime = now;

        Serial.print("Beat #");
        Serial.print(beatCount);
        Serial.print(" at ");
        Serial.print(now - startTime);
        Serial.println(" ms");
      }
    }

    aboveThreshold = nowAbove;

    delay(10); // ~100 samples per second
  }

  Serial.println("Measurement finished.");

  if (beatCount == 0) {
    Serial.println("Could not detect any beats. Try again, stay more still and adjust sensor position.");
    bpmValid = false;
  } else {
    // 10-second window → BPM = beats * 6
    float bpm = beatCount * 6.0;
    lastBPM = bpm;
    bpmValid = true;

    Serial.print("\n====================================\n");
    Serial.print("Your heart rate is approximately: ");
    Serial.print(bpm, 1);
    Serial.println(" bpm");
    Serial.println("====================================\n");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Pulse Sensor 10-Second Heart Rate + SMS ===");

  pinMode(PULSE_PIN, INPUT);
  analogReadResolution(12);                 // 0–4095
  analogSetPinAttenuation(PULSE_PIN, ADC_11db);

  // Ask user to place finger
  Serial.println("Place your finger gently on the pulse sensor.");
  Serial.println("Keep your hand still and relaxed.");

  // Countdown before starting measurement
  for (int i = 5; i > 0; i--) {
    Serial.print("Starting measurement in ");
    Serial.print(i);
    Serial.println(" seconds...");
    delay(1000);
  }

  // 1) Measure heart rate for 10 seconds
  measureHeartRate10sec();

  // 2) Prepare SMS text
  String smsText;
  if (bpmValid) {
    smsText = "Heart rate measurement result:\n";
    smsText += "Your heart rate is approx: ";
    smsText += String(lastBPM, 1);
    smsText += " bpm";
  } else {
    smsText = "Heart rate measurement result:\n";
    smsText += "Could not detect heartbeat reliably.\n";
    smsText += "Please try again.";
  }

  // 3) Power on modem and send SMS
  powerOnModem();

  sim800.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  sendAT("AT");
  sendAT("ATE0");
  sendAT("AT+CMGF=1");
  sendAT("AT+CSCS=\"GSM\"");
  sendAT("AT+CMEE=2");   // verbose errors

  if (!waitForNetwork(60000)) {
    Serial.println("Failed to register on network, cannot send SMS.");
    return;
  }

  Serial.println("Network registered, sending SMS with heart rate...");
  sendSMS(PHONE_NUMBER, smsText.c_str());

  Serial.println("Done. Reset the board to measure again.");
}

void loop() {
  // nothing
}

#include <HardwareSerial.h>

HardwareSerial sim800(1); // UART1

#define MODEM_TX 27
#define MODEM_RX 26
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_RESET 5

const char* PHONE_NUMBER = "+918105215353";  // your number

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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Booting T-Call ESP32 SIM800L SMS auto test...");

  powerOnModem();

  sim800.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  sendAT("AT");
  sendAT("ATE0");
  sendAT("AT+CMGF=1");
  sendAT("AT+CSCS=\"GSM\"");
  sendAT("AT+CMEE=2");   // verbose errors

  if (!waitForNetwork(60000)) {
    Serial.println("Failed to register on network.");
    return;
  }

  Serial.println("Network registered, sending SMS...");
  sendSMS(PHONE_NUMBER, "Hello from T-Call ESP32 + SIM800L! (auto)");
}

void loop() {
  // nothing
}

#include <ModbusMaster.h>
#include <WiFiManager.h> // ใช้ WiFiManager
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
// Create an instance of ModbusMaster
ModbusMaster node;


// Create an instance of the WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Define pins for RS485
#define RS485_RX_PIN 19
#define RS485_TX_PIN 18

// PM2100 Modbus address
const uint8_t PM2100_ADDRESS = 1;
long SendDataCounter = 0;
// Function to connect to the MQTT broker
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect with MQTT username and password
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      ESP.restart();
      
    }
  }
}

// Function to read Modbus registers from 3000 to 3076
bool readPM2100Range()
{
  uint8_t result = node.readHoldingRegisters(2999, 38); // อ่านตั้งแต่รีจิสเตอร์ 3000 ถึง 3076

  if (result == node.ku8MBSuccess)
  {
    return true;
  }
  else
  {
    Serial.print("Modbus Error Code: ");
    Serial.println(result);
    return false;
  }
}

// Function to read power values from registers 3054 to 3060
bool readPowerRegisters()
{
  uint8_t result = node.readHoldingRegisters(3053, 8); // อ่านรีจิสเตอร์สำหรับ Active Power (3054 ถึง 3060)

  if (result == node.ku8MBSuccess)
  {
    return true;
  }
  else
  {
    Serial.print("Modbus Error Code: ");
    Serial.println(result);
    return false;
  }
}

// Function to read energy values from registers 2676 to 2680
bool readEnergyRegisters()
{
  uint8_t result = node.readHoldingRegisters(2675, 6); // อ่านรีจิสเตอร์ 2676 ถึง 2680 (6 รีจิสเตอร์)

  if (result == node.ku8MBSuccess)
  {
    return true;
  }
  else
  {
    Serial.print("Modbus Error Code: ");
    Serial.println(result);
    return false;
  }
}

// Function to convert two 16-bit registers into a float (32-bit)
float getFloatFromRegister(uint16_t reg1, uint16_t reg2)
{
  uint32_t value = ((uint32_t)reg1 << 16) | reg2;
  return *((float *)&value); // แปลงเป็น float
}

void setup()
{
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize Modbus communication on Serial2 (RS485)
  Serial2.begin(19200, SERIAL_8E1, RS485_RX_PIN, RS485_TX_PIN);

  // Modbus communication
  node.begin(PM2100_ADDRESS, Serial2);

  // Initialize WiFi using WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP32-PowerMeter2100"); // AP Name if no WiFi is available

  // Setup MQTT
  client.setServer(mqtt_server, 1883);
  client.connect(mqtt_client_id, mqtt_user, mqtt_password);
}

void loop()
{
  // Ensure MQTT connection
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // Read Current, Voltage, and Power data
  if (readPM2100Range())
  {
    // Convert Modbus register values to float32
    float currentA = getFloatFromRegister(node.getResponseBuffer(0), node.getResponseBuffer(1));
    float currentB = getFloatFromRegister(node.getResponseBuffer(2), node.getResponseBuffer(3));
    float currentC = getFloatFromRegister(node.getResponseBuffer(4), node.getResponseBuffer(5));
    float currentN = getFloatFromRegister(node.getResponseBuffer(6), node.getResponseBuffer(7));
    float currentG = getFloatFromRegister(node.getResponseBuffer(8), node.getResponseBuffer(9));
    float currentAvg = getFloatFromRegister(node.getResponseBuffer(10), node.getResponseBuffer(11));

    float voltageAB = getFloatFromRegister(node.getResponseBuffer(20), node.getResponseBuffer(21));
    float voltageBC = getFloatFromRegister(node.getResponseBuffer(22), node.getResponseBuffer(23));
    float voltageCA = getFloatFromRegister(node.getResponseBuffer(24), node.getResponseBuffer(25));
    float voltageLLAvg = getFloatFromRegister(node.getResponseBuffer(26), node.getResponseBuffer(27));
    float voltageAN = getFloatFromRegister(node.getResponseBuffer(28), node.getResponseBuffer(29));
    float voltageBN = getFloatFromRegister(node.getResponseBuffer(30), node.getResponseBuffer(31));
    float voltageCN = getFloatFromRegister(node.getResponseBuffer(32), node.getResponseBuffer(33));

    // Read Power data from Modbus
    if (readPowerRegisters())
    {
      float activePowerA = getFloatFromRegister(node.getResponseBuffer(0), node.getResponseBuffer(1));
      float activePowerB = getFloatFromRegister(node.getResponseBuffer(2), node.getResponseBuffer(3));
      float activePowerC = getFloatFromRegister(node.getResponseBuffer(4), node.getResponseBuffer(5));
      float activePowerTotal = getFloatFromRegister(node.getResponseBuffer(6), node.getResponseBuffer(7));

      // Read Energy data from Modbus
      if (readEnergyRegisters())
      {
        float energyDelivered = getFloatFromRegister(node.getResponseBuffer(0), node.getResponseBuffer(1)); // 2676
        float energyReceived = getFloatFromRegister(node.getResponseBuffer(2), node.getResponseBuffer(3));  // 2678
        float energyTotal = getFloatFromRegister(node.getResponseBuffer(4), node.getResponseBuffer(5));     // 2680

        // Create a JSON object to store all data
        StaticJsonDocument<512> doc;
        doc["Ia"] = currentA;
        doc["Ib"] = currentB;
        doc["Ic"] = currentC;
        doc["Iav"] = currentAvg;

        doc["Van"] = voltageAN;
        doc["Vbn"] = voltageBN;
        doc["Vcn"] = voltageCN;

        doc["Pa"] = activePowerA;
        doc["Pb"] = activePowerB;
        doc["Pc"] = activePowerC;
        doc["Pt"] = activePowerTotal;

        // doc["Ed"] = energyDelivered;
        // doc["Er"] = energyReceived;
        // doc["Et"] = energyTotal;

        // Serialize the JSON data to a string
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer);

        // Wait 5 seconds before the next reading
        Serial.println("Published MQTT");
        Serial.println(jsonBuffer);

        SendDataCounter++;
        Serial.println("Timer= "+ String(SendDataCounter));
        Serial.println(jsonBuffer);
        if (SendDataCounter > 60)
        {
           Serial.println("MQTT Send Data");
          // Publish the JSON data to the MQTT broker
          client.publish(mqtt_topic, jsonBuffer);
          SendDataCounter=0;
        }
      }
      else
      {
        Serial.println("Failed to read Energy registers");
      }
    }
    else
    {
      Serial.println("Failed to read Power registers");
    }
  }
  else
  {
    Serial.println("Failed to read Modbus registers");
  }

  delay(5000);
}

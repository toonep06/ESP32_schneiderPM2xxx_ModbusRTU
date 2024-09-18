# ESP32 PM2xxxx Power Meter with Modbus, MQTT, and WiFi Manager

This project uses an ESP32 to read data from a PM2100 power meter using Modbus RTU communication over RS485. The collected data is published to an MQTT broker, and the ESP32 connects to WiFi using WiFiManager for easy configuration.

## Features

- Reads current, voltage, power, and energy data from the PM2100 power meter.
- Communicates with the power meter via Modbus RTU using RS485.
- Publishes data to an MQTT broker in JSON format.
- Connects to WiFi using WiFiManager for easy setup.
- Automatically reconnects to MQTT broker if the connection is lost.

## Libraries Used

- [ModbusMaster](https://github.com/4-20ma/ModbusMaster): For Modbus communication.
- [WiFiManager](https://github.com/tzapu/WiFiManager): For easy WiFi configuration.
- [PubSubClient](https://github.com/knolleary/pubsubclient): For MQTT communication.
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson): For handling JSON data.

## Hardware Requirements

- ESP32
- PM2100 Power Meter
- RS485 to TTL converter
- MQTT broker (can be hosted locally or in the cloud)

## Pin Configuration

- **RS485 RX Pin:** 19
- **RS485 TX Pin:** 18

## Installation

1. Clone the repository:
    ```bash
    git clone https://github.com/your-repository-url
    cd your-repository-folder
    ```

2. Install the necessary libraries using PlatformIO or the Arduino Library Manager:
    - ModbusMaster
    - WiFiManager
    - PubSubClient
    - ArduinoJson

3. Modify the `config.h` file to add your MQTT and network details:
    ```cpp
    const char* mqtt_server = "your_mqtt_server_ip";
    const char* mqtt_client_id = "your_mqtt_client_id";
    const char* mqtt_user = "your_mqtt_username";
    const char* mqtt_password = "your_mqtt_password";
    const char* mqtt_topic = "your_mqtt_topic";
    ```

4. Build and upload the code to your ESP32 using PlatformIO or the Arduino IDE.

## Usage

1. Upon first boot, the ESP32 will create an access point (AP) named `ESP32-PowerMeter2100`. Connect to this AP using your phone or computer.
2. After connecting, your browser will automatically open a configuration page. Enter your WiFi credentials, and the ESP32 will connect to your network.
3. The ESP32 will automatically connect to the MQTT broker and begin publishing power meter data in JSON format.

## JSON Data Format

The published data will look like this:

```json
{
  "Ia": 10.5,
  "Ib": 10.7,
  "Ic": 10.8,
  "Iav": 10.6,
  "Van": 220.0,
  "Vbn": 220.1,
  "Vcn": 220.3,
  "Pa": 1000.5,
  "Pb": 1005.7,
  "Pc": 1002.3,
  "Pt": 3008.5
}

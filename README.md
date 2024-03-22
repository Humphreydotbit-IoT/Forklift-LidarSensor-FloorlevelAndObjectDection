## Floor level and object detection with two TF mini Lidar Sensors

![309](https://github.com/Humphreydotbit-IoT/Forklift-LidarSensor-FloorlevelAndObjectDection/assets/164144789/e0826926-b030-4dc8-b8e6-a392678e5e93)

---

This project utilizes two TF Mini LIDAR sensors and an ESP32 WROOM S3 microcontroller to monitor a forklift's operation and environment in real-time.

## Functionality

- **Floor Level Detection:**
  - Sensor 1 measures the distance to the floor.
  - The detected floor level is categorized into different layers based on distance thresholds.
  - The detected floor level is sent to the MQTT broker in JSON format as a real-time message payload.

- **Object Detection:**
  - Sensor 2 detects the presence of objects.
  - If an object is detected, the message "true" is sent to the MQTT broker; otherwise, "false" is sent.

- **OTA Update:**
  - AsyncElegantOTA method is implemented for Over-The-Air (OTA) updates to facilitate future software updates.

---

## Dependencies

- **Libraries Used:**
  - Wire.h
  - WiFi.h
  - PubSubClient.h
  - time.h
  - AsyncTCP.h
  - ESPAsyncWebServer.h
  - AsyncElegantOTA.h

## Hardware Configuration

- **Sensor 1 Connection:**
  - Sensor 1 is connected to the ESP32 WROOM S3 microcontroller via UART communication.

- **Sensor 2 Connection:**
  - Sensor 2 is connected to the ESP32 WROOM S3 microcontroller via UART communication.

## Usage

1. Install the required libraries in your Arduino IDE.
2. Upload the provided code to your ESP32 board.
3. Connect the TF Mini LIDAR sensors to the specified pins.
4. Configure the WiFi and MQTT settings in the code.
5. Power on the system and monitor the forklift's operation and environment.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

For inquiries or support, please contact:
- [Your Name](mailto:your.email@example.com)

# QR Code via MQTT with only ESP32


**Note**: Don't connect GPIO2 from WS2812B while flashing

Based on: https://www.makeuseof.com/scanning-qr-codes-with-esp32-cam

Used lib: https://github.com/alvarowolfx/ESP32QRCodeReader

OTA: https://github.com/ayushsharma82/AsyncElegantOTA
  - Whatever system is used for OTA (Arduino OTA, ElegantOTA or anything else), it requires that the partitions are correctly set. (e.g. in Arduino IDE check _Partition Scheme_ )

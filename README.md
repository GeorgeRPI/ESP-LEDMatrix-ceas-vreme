# Ceas LED Matrix cu Vreme
![Version](https://img.shields.io/badge/version-4.04.2025-blue)
![Arduino](https://img.shields.io/badge/ESP-8266-orange.svg)
![Arduino](https://img.shields.io/badge/matrix-LED7219-brightgreen.svg)

Acest proiect afișează ora, data și vremea dupa oras pe un afișaj LED Matrix folosind ESP8266.

## Caracteristici
- Afișare data si oră în format 24h
- Afișază vremea:
  - temperatura
  - temperatura resimtită
  - umiditatea
  - presiune
  - vant
- Se poate alege orașul
- Vremea dupa oraș, sunt extrase de pe OpenWeather (Citiți descrierea API OpenWeather pentru mai multe informații)
- Actualizează ora de la serverul NTP
- Ajustează pentru fusul orar (UTC+2 sau UTC+3 pentru Romania)
- Control automat al afișajului (se oprește si porneste la ora setata)
- Conexiune la WiFi

## Componente necesare
- ESP8266 (NodeMCU sau Wemos D1)
- LED Matrix 8x32 cu driver MAX7219
- Conexiune la rețea WiFi

## Biblioteci necesare
- ESP8266WiFi versiunea 1.0
- Matrix LED7219
- ArduinoJson versiunea 6.21.2
- WiFiUdp
- NTPClient
- SPI

# Ceas LED Matrix cu Vreme
![Version](https://img.shields.io/badge/version-4.04.2025-blue)
![Arduino](https://img.shields.io/badge/ESP-8266-orange.svg)
![Arduino](https://img.shields.io/badge/matrix-LED7219-brightgreen.svg)

Acest proiect afișează ora, data și vremea dupa oras pe un afișaj LED Matrix folosind ESP8266.

## Caracteristici
- Afișare ora în format 24h
- Afișare data și vreme
    - temeratura
    - temperatura resimtita
    - umiditatea
    - presiune
    - vant
- Se poate alege orasul
- Actualizează ora de la serverul NTP
- Ajustează pentru fusul orar (UTC+2 sau UTC+3)
- Vremea dupa oras sunt extrase de pe OpenWeather (Citiți descrierea API OpenWeather pentru mai multe informații)
- Control automat al afișajului (se oprește noaptea la ora setata)
- Conexiune la WiFi și sincronizare NTP

## Componente necesare
- ESP8266 (NodeMCU sau Wemos D1)
- LED Matrix 8x32 cu driver MAX7219
- Conexiune la rețea WiFi

## Biblioteci necesare
- ESP8266WiFi at version 1.0
- Matrix LED7219
- ArduinoJson at version 6.21.2
- WiFiUdp
- NTPClient
- SPI

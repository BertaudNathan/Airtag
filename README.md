# Système de détection de mouvement avec ESP32 et MQTT

## Introduction
Ce projet implémente un système de détection de mouvement utilisant des microcontrôleurs ESP32 et le protocole MQTT pour la communication. Un ESP32 agit comme émetteur, détectant les mouvements à l'aide d'un accéléromètre MPU6050 et publiant des alertes sur un broker MQTT hébergé sur un Raspberry Pi. Un second ESP32 agit comme récepteur, s'abonnant aux alertes MQTT et activant des notifications LED et buzzer en cas de mouvement détecté.

## Architecture

### Composants

1. **ESP32 Émetteur** : Détecte le mouvement à l'aide de l'accéléromètre MPU6050 et publie des alertes via MQTT
2. **ESP32 Récepteur** : S'abonne aux alertes MQTT et active les notifications LED + buzzer
3. **Raspberry Pi** : Héberge le broker MQTT Mosquitto pour le routage des messages

Tous les appareils se connectent au même réseau WiFi.

### Fonctionnalités

- Détection de mouvement en temps réel avec sensibilité configurable
- Architecture publish/subscribe basée sur MQTT
- Reconnexion automatique WiFi et MQTT avec backoff exponentiel
- Alertes audio-visuelles (LED + buzzer)
- Anti-rebond pour éviter les faux positifs
- Indicateurs de statut LED pour la connectivité et les alertes
- Envoie de données de mouvement détaillées (magnitude, timestamp) dans les messages MQTT
- Envoie de données hardware (température, batterie, utilisation CPU et RAM) pour diagnostic


## Structure du projet

```
include/          # Fichiers d'en-tête (.h)
├── drivers/      # Abstraction matérielle (MPU6050, LED, Buzzer)
├── models/       # Structures de données (MovementEvent, DeviceConfig, etc.)
├── services/     # Logique métier (Accéléromètre, MQTT, WiFi, Alert)
└── utils/        # Utilitaires (Logger)

src/              # Fichiers d'implémentation (.cpp)
├── drivers/
├── models/
├── services/
├── utils/
├── main_transmitter.cpp    # Firmware émetteur
└── main_receiver.cpp       # Firmware récepteur

docs/
└── raspberry-pi-setup.md   # Guide d'installation du broker MQTT

specs/001-mqtt-airtag/
├── spec.md                 # Spécification de la fonctionnalité
├── plan.md                 # Plan d'implémentation
└── checklists/
    └── requirements.md     # Validation qualité
```

## Prérequis matériels

### Émetteur

- ESP32
- Accéléromètre MPU6050  (I2C)
- LED de statut (ou utiliser la LED intégrée sur GPIO 2)
- Alimentation USB ou batterie

**Câblage :**
- MPU6050 SDA → ESP32 GPIO 21
- MPU6050 SCL → ESP32 GPIO 22
- MPU6050 VCC → 3.3V
- MPU6050 GND → GND

### Récepteur

- ESP32
- LED de statut (GPIO 2)
- LED d'alerte (GPIO 4)
- Buzzer passif (GPIO 5)
- Alimentation USB ou batterie

**Câblage :**
- LED d'alerte (+) → ESP32 GPIO 4 → Résistance (220Ω) → GND
- Buzzer (+) → ESP32 GPIO 5
- Buzzer (-) → GND

### Raspberry Pi Broker

- Raspberry Pi 5 or Pi 4 (or Pi 3 B+)
- Carte MicroSD avec Raspberry Pi OS
- Alimentation
- Connexion réseau (WiFi ou Ethernet)

## Démarrage rapide

### 1. Configuration du broker MQTT sur Raspberry Pi

Suivez le guide détaillé : [docs/raspberry-pi-setup.md](docs/raspberry-pi-setup.md)

Commandes rapides :
```bash
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
echo "listener 1883" | sudo tee -a /etc/mosquitto/mosquitto.conf
echo "allow_anonymous true" | sudo tee -a /etc/mosquitto/mosquitto.conf
sudo systemctl restart mosquitto
```

Obtenez l'adresse IP du Raspberry Pi :
```bash
hostname -I
```

### 2. Configuration des appareils ESP32

Editer `src/main_transmitter.cpp` et `src/main_receiver.cpp`:

```cpp
void setupConfiguration() {
    strcpy(config.wifiSSID, "YOUR_WIFI_SSID");        // Votre SSID WiFi
    strcpy(config.wifiPassword, "YOUR_WIFI_PASSWORD"); // Votre mot de passe WiFi
    strcpy(config.mqttBrokerIP, "192.168.1.100");     // Adresse IP du Raspberry Pi (broker MQTT)
    // ... 
}
```

### 3. Compilation et téléversement

**Installer PlatformIO CLI** (si non installé):
```bash
pip install platformio
```

**Compiler Émetteur:**
```bash
pio run -e transmitter
pio run -e transmitter -t upload
```

**Compiler Récepteur:**
```bash
pio run -e receiver
pio run -e receiver -t upload
```

### 4. Sortie série pour le débogage

**Émetteur:**
```bash
pio device monitor -e transmitter
```

**Récepteur:**
```bash
pio device monitor -e receiver
```

### 5. >Test de bout en bout

1. Allumez le Raspberry Pi et vérifiez que Mosquitto fonctionne
2. Téléversez le firmware de l'émetteur sur le premier ESP32
3. Téléversez le firmware du récepteur sur le second ESP32
4. Déplacez l'appareil émetteur - le récepteur devrait alerter !

## Options de configuration

### Sensibilité au mouvement

Dans `setupConfiguration()`:
```cpp
config.sensitivityThreshold = 2.0f;  // Seuil en G (par défaut : 2G)
```

Augmenter pour moins de fausses alertes, diminuer pour plus de sensibilité.

### Durée de l'alerte

```cpp
config.alertDuration = 5000;  // Durée en millisecondes (par défaut : 5 secondes)
```

### Intervalle de rebond

```cpp
config.debounceWindow = 1000;  // Millisecondes (par défaut : 1 seconde)
```

Évite les alertes répétées rapides lors d'un mouvement continu.

### Channel MQTT

```cpp
strcpy(config.mqttTopic, "airtag/motion");  // Topic MQTT pour les alertes de mouvement
```

## Indicateurs LED de statut

### Émetteur

- **Éteint** : Entièrement connecté et inactif
- **Clignotement lent** : Problème de connexion WiFi/MQTT (nouvelle tentative)
- **Flash rapide** : Mouvement détecté et publié

### Récepteur

- **Éteint** : Entièrement connecté et à l'écoute
- **Clignotement lent** : Problème de connexion WiFi/MQTT (nouvelle tentative)
- **Clignotement rapide + buzzer** : Alerte de mouvement active

## Surveillance

### Surveillance du trafic MQTT

Sur Raspberry Pi:
```bash
# Voir tous les messages
mosquitto_sub -h localhost -t '#' -v

# Voir uniquement les événements de mouvement
mosquitto_sub -h localhost -t 'airtag/motion' -v
```

### Test MQTT manuellement

Publier un événement de test :
```bash
mosquitto_pub -h localhost -t 'airtag/motion' -m '{"deviceID":"test","timestamp":1000,"magnitude":1.5,"type":"start"}'
```

Le récepteur devrait déclencher une alerte.


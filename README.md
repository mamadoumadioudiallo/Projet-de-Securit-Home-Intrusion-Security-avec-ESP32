# Projet-de-Securit-Home-Intrusion-Security-avec-ESP32
Mamadou Madiou Diallo 

Intelligent home security system based on the ESP32, capable of detecting intrusions using motion and door sensors, and if an intrusion is detected, the system can trigger an audible alarm.

# Introduction : 
Pour mon projet de microcontrôleurs, j'ai décidé de fabriquer un système d'alarme maison en utilisant un ESP32. L'idée est d'avoir un boîtier capable de surveiller si quelqu'un entre dans une pièce ou ouvre une porte quand le système est activé.

Le montage utilise deux types de détection : un capteur pour capter les mouvements (PIR) et un autre pour l'ouverture des portes. Si le système est armé et qu'il détecte quelque chose, il déclenche automatiquement un buzzer pour donner l'alerte. C'est un projet qui mélange électronique de base et programmation pour gérer les capteurs en temps réel.

# Pourquoi ce projet ?
Pendant mes analyse pour un projet de microcontroleur durant nos cours, j'ai voulu relever le défi de coder un système "multi-tâches" sans utiliser les fonctions de pause classiques, permettant ainsi à l'alarme de surveiller l'environnement, de faire sonner un buzzer et d'écouter les commandes de l'utilisateur simultanément.

# Ce que fait ce système :
* Surveillance active : Détecte l'ouverture d'une porte (via un contact magnétique) et les mouvements (via un capteur PIR).

* Contrôle intelligent : Le système peut être armé ou désarmé via une interface série sécurisée.

* Traitement du signal : Intégration d'un anti-rebond (debounce) logiciel pour éviter les déclenchements accidentels.

* Alerte sonore : Pattern de bip spécifique géré de manière non-bloquante.

# Hardware
Le système est conçu autour d'une architecture simple. Comme j'avais pas les autres composants du materiel Arduino, j'ai utilisé l'ESP32 comme unique microcontrôleur, j'ai profité de ses broches GPIO pour connecter directement les capteurs et le buzzer.

![Architecture du système](Diagramme du systeme Home Intrsusion Security.drawio)
# Nomenclature (BOM)
Pour ce projet, j'ai choisi d'utiliser une configuration minimale centrée sur l'ESP32. Les capteurs physiques sont simulés par des connexions directes sur les broches GPIO, ce qui permet de tester toute la logique du système de sécurité sans composants supplémentaires.

| Composant | Quantité | Description / Rôle | Connexion / Pin |
| :--- | :---: | :--- | :--- |
| **ESP32 DevKit V1** | 1 | Microcontrôleur principal. Il gère le code, la surveillance des entrées et les alertes. | - |
| **Simulateur de Porte** | 1 | Entrée configurée en `INPUT_PULLUP`. On simule l'ouverture en débranchant un fil relié au GND. | GPIO 26 |
| **Simulateur PIR** | 1 | Entrée numérique pour la détection de mouvement. | GPIO 27 |
| **Sortie Alarme** | 1 | Sortie numérique pour le signal sonore (Buzzer). | GPIO 25 |
| **Câble Micro-USB** | 1 | Alimentation du module et transfert du code depuis l'ordinateur. | Port USB |

# Software
Le code a été développé avec l'IDE PlatformIO et utilise le framework Arduino. J'ai structuré le programme pour qu'il soit "réactif", c'est-à-dire qu'il puisse surveiller plusieurs capteurs tout en écoutant les commandes de l'utilisateur.

# Organisation du Code
Le projet est découpé en plusieurs fonctions spécifiques pour rendre le code lisible et facile à maintenir :

* setup() : Initialise la communication série (115200 baud) et configure les broches GPIO. Les entrées sont mises en INPUT_PULLUP pour permettre la simulation avec de simples câbles vers la masse (GND).

* loop() : C'est le cœur du système. Elle tourne en boucle pour vérifier en permanence si une commande arrive sur le port série ou si un capteur change d'état.

* readSerialCommands() : Cette fonction gère l'interface utilisateur. Elle lit les caractères envoyés (A, D, S, R) pour armer ou désarmer le système.

* doorIsOpenDebounced() : Une fonction essentielle qui utilise un filtre de temps (40ms) pour s'assurer qu'un signal sur la broche de la porte est bien réel et n'est pas dû à un mauvais contact électrique.

* alarmBeepPattern() : Gère le clignotement sonore du buzzer.

# Fonctionnement technique
Le point le plus important de mon code est l'absence de la fonction delay().

J'utilise à la place la fonction millis(). Cela permet de créer un multitâche coopératif :

+ Le microcontrôleur vérifie si le système est armé.

* Si une intrusion est détectée, il active un état d'alarme.

* Pendant que l'alarme sonne (bips de 200ms), l'ESP32 reste disponible pour recevoir une commande de désarmement immédiat.

# Tests et Résultats
Cette section détaille les tests effectués pour valider le bon fonctionnement du système. L'objectif était de vérifier que l'ESP32 réagit correctement aux intrusions et aux commandes manuelles.

| Objectif du test | Action effectuée | Résultat attendu | Résultat obtenu | Statut |
| :--- | :--- | :--- | :--- | :---: |
| **Vérifier l'armement** | Envoyer A via le moniteur série. | Le terminal affiche [SYSTEM] ARMED. | Le système confirme l'activation. | Vrai |
| **Détection d'ouverture** | Débrancher le fil de la **GPIO 26**. | Déclenchement immédiat de l'alarme (Bips). | Alerte reçue : [INTRUSION] Door OPEN. | Vrai |
| **Anti-rebond** | Tapoter rapidement le fil sur le GND. | Pas de fausse alerte grâce au délai de 40ms. | Le système reste stable. | Vrai |
| **Détection de mouvement** | Envoyer un signal HIGH sur la **GPIO 27**. | Déclenchement de l'alarme sonore. | Alerte reçue : [INTRUSION] Motion detected. | Vrai |
| **Désarmement à chaud** | Envoyer D pendant que l'alarme sonne. | Arrêt immédiat du son et passage en "Disarmed". | Le buzzer s'arrête instantanément. | Vrai |
| **Consultation d'état** | Envoyer S dans le moniteur série. | Affichage des valeurs en temps réel des pins. | Rapport complet affiché à l'écran. | Vrai |




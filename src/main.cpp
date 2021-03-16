#include <Arduino.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include <DHT_U.h>

// L'adresse MAC pour le shield
byte mac[] = { 0xAE, 0xAD, 0xAE, 0xAE, 0xAE, 0xAD };

//Infos MQTT a adapter
const char* mqttServer = "http://openmediavault.local";  //Adresse IP du Broker MQTT
const int mqttPort = 1883;               //Port utilisé par le Broker MQTT


// On créé l'objet client pour la connexion au serveur
EthernetClient client;
// Creation d un objet client publication MQTT s appuyant sur le client WIFI
PubSubClient monClientMqtt(client);

//On définie les pins
#define DHT11PIN 2     // PIN pour la sonde DHT11
#define Relais_1 4
#define Relais_2 5


//Connexion
IPAddress IParduino(192, 168, 0, 159);
// IP de la freebox (va servir pour : dns, passerelle)
IPAddress IPDNS(192,168,0,1);


// On créé l'objet pour la sonde DHT11
DHT dht(DHT11PIN, DHT11);


// Variables pour comparer l'ancienne valeur des sondes à la nouvelle
int t_old = 0;
int h_old = 0;

// POUR DEBUG
// Variable pour compter le nombre de connexion échouée de client.connect
int NombreErreurReseau = 0 ;
int NombreErreurReseau_old = -1;
int NombreProblemeDeconnexion = 0 ;
boolean etat_ventil_1 = 0;
boolean etat_ventil_2 = 0;
boolean etat_ventil_1_old = 0;
boolean etat_ventil_2_old = 0;

// Variable de Tempo pour déclenchement de lecture
unsigned long previousMillis = 0;



void setup() {
  // put your setup code here, to run once:
  // On initialise la sonde DHT11
  dht.begin();
  
  // On ouvre le port série pour DEBUG
  Serial.begin(9600);
  
  // On initialise le shield avec IP fixe
  Ethernet.begin(mac, IParduino , IPDNS , IPDNS);

    // On attend que le shield s'initialise
  delay(500);

    // Init client MQTT pour la publication
  monClientMqtt.setServer(mqttServer, mqttPort);

      // On attend que le shield s'initialise
  delay(500);

  Serial.println(F("*** Fin de la configuration ***"));
 
  //Définition des sortie pour les relais
  pinMode(Relais_1, OUTPUT);
  pinMode(Relais_2, OUTPUT);

  //Pour DEBUG
  //Obtenir l'adresse IP de l'arduino
  IPAddress IP_Arduino = Ethernet.localIP();
  Serial.println(IP_Arduino);

}

void loop() {
  // put your main code here, to run repeatedly:
  //Mesure du temps
      unsigned long currentMillis = millis();
  // Connexion au serveur MQTT
  while (!monClientMqtt.connected()) {
    Serial.println("Connexion au serveur MQTT ...");
    if (monClientMqtt.connect("53salonlolin")) {
      Serial.println("MQTT connecte");
    }
    else {
      Serial.print("Echec connexion serveur MQTT, code erreur= ");
      Serial.println(monClientMqtt.state());
      Serial.println("nouvel essai dans 2s");
    delay(2000);
    }
  }
if ( currentMillis - previousMillis >= 60000 ) 
    {
      //Incrémentation du temps
      previousMillis = currentMillis;

      //Récupération des valeurs
      int h = dht.readHumidity();
      int t = dht.readTemperature();

      //Débug
      Serial.print(F("T11 "));
      Serial.print(t);
      Serial.print(F("C - H11 "));
      Serial.print(h);
      Serial.print(F("%"));
      Serial.print(F("C || Millis : "));
      Serial.print(millis()/1000);
      Serial.print(F(" || NbPbDeco : "));
      Serial.print(NombreProblemeDeconnexion);
      Serial.print(F(" || NbErReseau : "));
      Serial.println(NombreErreurReseau);

      if((float)t>= 28.00){
        digitalWrite(Relais_1, HIGH);
        Serial.println("Relais 1 allimenté");
        etat_ventil_1= 1;
        if((float)t>=34.00){
          digitalWrite(Relais_2, HIGH);
          Serial.println("Relais 2 allimenté"); 
          etat_ventil_2= 1;
        }
      }
      else {
        digitalWrite(Relais_1, LOW);
        digitalWrite(Relais_2, LOW);
        Serial.println("Les relais sont éteint");
        etat_ventil_1= 0;
        etat_ventil_2= 0;
      }

      // Publication MQTT
      monClientMqtt.publish("Baie/temp", String(t).c_str());
      monClientMqtt.publish("Baie/hygro", String(h).c_str());
      if(etat_ventil_1 != etat_ventil_1_old){
        monClientMqtt.publish("Baie/vent/etat_vent_1", String(etat_ventil_1).c_str());
        etat_ventil_1_old = etat_ventil_1;
        Serial.println("Trame ventilateur 1 envoyé");                
      }
      if(etat_ventil_2 != etat_ventil_2_old){
        monClientMqtt.publish("Baie/vent/etat_vent_2", String(etat_ventil_2).c_str());
        etat_ventil_2_old = etat_ventil_2;
        Serial.println("Trame ventilateur 2 envoyé");                  
      }
      //Débug
      Serial.print(etat_ventil_1);
      Serial.print(etat_ventil_1_old);
      Serial.print(etat_ventil_2);
      Serial.print(etat_ventil_2_old);
    }
 
}
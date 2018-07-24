// This is a demo of the RBBB running as webserver with the Ether Card
// 2010-05-28 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,1,203 };

byte Ethernet::buffer[500];
BufferFiller bfill;

const int relay_pin_8 = 8; // Relay pin
static char active_relay_pin_8 = LOW; // Relay activity

const int ana_relay_pin_8 = 8; // Relay pin

float zero_senseur; 
int PIN_ACS712 = A0;

float courant; 
float courant_efficace;     
float tension_efficace = 230; // tension efficace du réseau electrique
float puissance_efficace; 
float ACS712_RAPPORT = 100; // nbr de millivolts par ampère

// Obtient la valeur du senseur de courant ACS712
//
// Effectue plusieurs lecture et calcule la moyenne pour pondérer
// la valeur obtenue.
float valeurACS712( int pin ){
  int valeur;
  float moyenne = 0;
  
  int nbr_lectures = 50;
  for( int i = 0; i < nbr_lectures; i++ ){
      valeur = analogRead( pin );
      moyenne = moyenne + float(valeur);
  }
  moyenne = moyenne / float(nbr_lectures);
  return moyenne;
}

void setup () {
  Serial.begin(57600);

  pinMode(relay_pin_8,OUTPUT);
  // calibration du senseur  (SANS COURANT)
  zero_senseur = valeurACS712( PIN_ACS712 );

  // Init
  digitalWrite(relay_pin_8, LOW);
  active_relay_pin_8 =  LOW;
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  ether.staticSetup(myip);
}

static int getIntArg(const char* data, const char* key, int value =-1) {
  char temp[10];
  if (ether.findKeyVal(data + 7, temp, sizeof temp, key) > 0)
    value = atoi(temp);
  return value;
}


static void homePage (BufferFiller& buf) {
  bfill.emit_p(PSTR(
        "HTTP/1.0 300\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>Home page</h1>"
        "<a href='a'>Config</a>"
        ));
}

static void activatePage (const char* data) { 
   if (data[6] == '?') {
      byte b = getIntArg(data, "b", 8);

      if(active_relay_pin_8 == LOW) {
        digitalWrite(relay_pin_8, HIGH);
        active_relay_pin_8 =  HIGH;  
      } else {
        digitalWrite(relay_pin_8, LOW);
        active_relay_pin_8 =  LOW;
      }

      /*float valeur_senseur = valeurACS712( PIN_ACS712 );
      // L'amplitude en courant est ici retournée en mA
      // plus confortable pour les calculs
      courant = (float)(valeur_senseur-zero_senseur)/1024*5/ACS712_RAPPORT*100000;
      // Courant efficace en mA
      courant_efficace = courant / 1.414; // divisé par racine de 2
    
      // Calcul de la puissance.
      //    On divise par 1000 pour transformer les mA en Ampère
      puissance_efficace = (courant_efficace * tension_efficace/1000);

      Serial.println( "zero_senseur - lecture - courant efficace (mA) - Puissance (W)" );
    Serial.print( zero_senseur );
    Serial.print( " - " );
    Serial.print( valeur_senseur );
    Serial.print( " - " );
    Serial.print( courant_efficace );
    Serial.print( "mA - " );
    Serial.print( puissance_efficace );
    Serial.println( " W" );
         
      bfill.emit_p(PSTR(
            "HTTP/1.0 300\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>Config page</h1>"
            "<p>Data:$S</p>\r\n"
            "<p>active_relay_pin_8 $S</p>\r\n"
            "<p>sensor_value_8 $S</p>\r\n"
            "<p>sensor_value $I</p>\r\n"
            "<p>zero_sensor $S</p>\r\n"
            ), b, active_relay_pin_8, valeur_senseur, puissance_efficace,  (char)zero_senseur );*/
            bfill.emit_p(PSTR(
            "HTTP/1.0 300\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>ON/OFF page</h1>"
            "<a href='a?b=8'>ON/OFF</a>"
            ));
   } else {
      bfill.emit_p(PSTR(
            "HTTP/1.0 300\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>Start page</h1>"
            "<a href='a?b=8'>Active</a>"
            ));
   }
}

void loop () {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) {  // check if valid tcp data is received
    bfill = ether.tcpOffset();
    char* data = (char *) Ethernet::buffer + pos;
    
    // receive buf hasn't been clobbered by reply yet
    if (strncmp("GET / ", data, 6) == 0)
      homePage(bfill);
    else if (strncmp("GET /a", data, 6) == 0)
      activatePage(data);
    else
      bfill.emit_p(PSTR(
        "HTTP/1.0 401 Unauthorized\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>401 Unauthorized</h1>"));  
    ether.httpServerReply(bfill.position()); // send web page data
  }
}



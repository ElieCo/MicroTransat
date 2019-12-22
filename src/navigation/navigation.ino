// Libraries GPS
#include <Adafruit_GPS.h>
#define GPSSerial Serial2
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
#include <TinyGPS++.h>

#include <SD.h>
#include <Servo.h>

File myFile;
Servo barre;
Servo aile;

// Variables utilisés par la routine d'interruption qui remplace pulseIn()   (reception radio)
volatile unsigned long LastPulseTime_ch3;
int duration;
#define ch3Pin 5

int course_aileron = 10;
int pres_VMG = 40;
int portant_VMG = 150;

int pos1 = 80 - 10;
int pos3 = 80 + 10;


int ledPin = 13;                  // LED test pin
int current_mode = 0;
int last_mode = 0;
int i = 0;
int ch3;
int ch6;
int ch5;
int angle_regulateur;
int angle1 = 0;
int precision = 0;
int cap_moyen;
boolean first_loop = true;
boolean led_state = true;
boolean hors_couloir = true;

// liste points GPS
float distanceToWaypoint;
double angleToWaypoint;
const float wp_lat[] = {48.431550, 48.430987};//{48.431643, 48.430960};// 0)quais 1) 100 mètres du quais NO
const float wp_lon[] = {-4.615716, -4.614247};//{-4.614617, -4.615505};//
unsigned int index_wpt = 0;              // index dans la liste des wpt
int seuil_valid_wpt = 25;

int estim_vent = -1;            // angle estimé du vent. Si -1 incertitude trop importante
int tab_cap[10];
unsigned int index_tab_cap = 0;
int nouvel_angle_regulateur;                 // Consigne du régulateur pour atteindre le waypoint
uint32_t timer = millis();
unsigned long timer1;           // timer
unsigned long timer2;           // interval de calcul cible
unsigned long timer3;           // Temp entre deux ligne du datalogguer
unsigned long timer4;           // Clignotement de la led
uint32_t timer_mesure = millis();
unsigned long timer6;
unsigned long interval_calcul = 10000;

// variables globales pour le data logger
unsigned long interval_datalogging = 1000;//1000;
String var_name_log[] = {"Heures","Minutes", "Secondes","HDOP", "Vitesse", "Cap", "Angle_regulateur", "Pos_aile", "Cap_moy", "precision", "Nb_satellites", "Latittude", "Longitude","Lat_next_point","Lon_next_point","Wpt_angle", "Wpt_dst","ecart_axe","Presence_couloir", "Mode","Index_wpt", "Commentaire"};
int buf[sizeof(var_name_log)];

int index_buffer_lignes = 0;
int taille_buffer_lignes = 10;
String lines_buffer[10];

// fonction de data logging
void datalog(String var_name, int value){

    if (var_name == "push") {
        // 4) Sur appel du mot clef "push", copier le buffer dans la carte sd. Et clear buffer.
        String line;
        for (unsigned int i = 0; i < sizeof(var_name_log)/sizeof(var_name_log[0]); i++){
            line += buf[i];
            line += ";";
        }
        lines_buffer[index_buffer_lignes] = line;
        Serial.println(line);
        index_buffer_lignes ++;
        //buf[sizeof(var_name_log)];  // reset buffer
    }

    else if (var_name == "init") {  // initialisation de l'entête csv.
      myFile = SD.open("log.csv", FILE_WRITE);
      String line;
      for (unsigned int i = 0; i < sizeof(var_name_log)/sizeof(var_name_log[0]); i++){
          line += var_name_log[i];
          line += ";";
      }
      Serial.println(line);
      myFile.println(line);
      myFile.close();
    }
    else {
        // 1) Trouver la position de la variable dans la trame csv
        for (unsigned int index_var = 0; index_var < sizeof(var_name_log); index_var ++){
            if (var_name == var_name_log[index_var]){
            // 2) Ajouter la valeur dans la ligne
                buf[index_var] = value;
            }
        }
    }
    if (index_buffer_lignes == taille_buffer_lignes){
      myFile = SD.open("log.csv", FILE_WRITE);
      for (int i=0; i<taille_buffer_lignes; i++){
        myFile.println(lines_buffer[i]);
      }
      myFile.close();
      index_buffer_lignes = 0;
    }
}

// récupère l'angle du régulateur d'allure, le cap et en déduis la direction du vent.
int analyse_vent(int regulateur, int cap){
  int estim = cap - regulateur;
  if (estim < 0){
    estim += 360;
  }
  return estim;
}

int filtrage_cap(int cap_instant){
  int cap = 0;
  if (index_tab_cap >= sizeof(tab_cap)/sizeof(int)-1){
    index_tab_cap = 0;
  }
  else {
    index_tab_cap ++;
  }

  tab_cap[index_tab_cap] = cap_instant;

  float x_cap = 0;
  float y_cap = 0;

  for (unsigned int i=0; i<sizeof(tab_cap)/sizeof(int); i++){
    x_cap += cos(radians(tab_cap[i]));
    y_cap += sin(radians(tab_cap[i]));
  }

  //precision = 0;//x_cap*x_cap + y_cap*y_cap;   // sur 10 mesures, la norme va être entre 0 (tout les angles sont opposés) et 10 (le cap venait toujours du même angle).
  //datalog("precision", precision);

  if (x_cap != 0){
    cap = (int)(180.0*atan(y_cap/x_cap)/PI);
    if (x_cap < 0){
      cap += 180;
    }
    else if (y_cap < 0){
      cap += 360;
    }
  }
  return cap;
}


boolean next_point(float dist){  // unité : mètres
  if (dist <= seuil_valid_wpt && dist != 0){ // && GPS.satellites > 3 // dist = 0 -> sacrément improbable !
    if (index_wpt < (sizeof(wp_lat)/sizeof(float))-1){
      index_wpt ++;
    }
    else {
      index_wpt = 0;
    }
    return true;
  }
  return false;
}

// paramètre : largeur du couloir de bord à bord. Return : true si le bateau est dans le couloir, false sinon
boolean test_couloir(int largeur){
  // calcul de l'axe du parcours
  float axe_parcours;
  if (index_wpt < (sizeof(wp_lat)/sizeof(float))-1){
    axe_parcours = TinyGPSPlus::courseTo(wp_lat[index_wpt], wp_lon[index_wpt], wp_lat[index_wpt+1], wp_lon[index_wpt+1]);
  }
  else {
    axe_parcours = TinyGPSPlus::courseTo(wp_lat[index_wpt], wp_lon[index_wpt], wp_lat[0], wp_lon[0]);
  }

  int ecart = abs(sin(((axe_parcours - angleToWaypoint)*180.0)/PI) * distanceToWaypoint);
  datalog("ecart_axe",ecart);
  if (ecart < largeur/2 && ecart != 0){  // quand on ne capte pas les satellites l'écart tombe a 0...
    return true;
  }
  else {
    return false;
  }
}

// Reçois un angle et l'adapte à la commande de barre avant de l'appliquer.
void commande_barre( int angle){
  datalog("Angle_regulateur",angle);
  // L'entrée doit être comprise entre 0 et 360°. 0 correspond à face au vent
  if (angle <= 360 && angle >= 0){
    angle_regulateur = angle;
    angle = angle/2+90; // on s'adapte a la course du servo (180°)
    // centrage des valeurs autour de 90° (pour qu'un 0 en entrée corresponde au milieu de la course du servo : 90)
    if (angle > 180){
      angle = angle - 180;
    }
    barre.write(angle);
  }
  else {
    datalog("Angle_regulateur",404);
  }
}

void reglage_aile_auto(int angle3){
  if (angle3 < 180){   // La position de l'aile dépend de l'angle du régulateur
    datalog("Pos_aile",1);
    aile.write(pos1);
  }
  else {
    datalog("Pos_aile",3);
    aile.write(pos3);
  }
}

void mode_semi_auto(int angle2){
  /////////////////////////////////////
  //        Mode semi auto           //
  /////////////////////////////////////
  estim_vent = -1;
  commande_barre(angle2);
  reglage_aile_auto(angle2);
}

void mode_autonome(){
  /////////////////////////////////////
  //        Mode full auto           //
  /////////////////////////////////////
  if (first_loop == true){  // première entrée dans le boucle autonome
    timer2 = millis();
    first_loop = false;
    commande_barre(50); // on se met au près le temps d'avoir une bonne mesure du cap
    reglage_aile_auto(50);
    Serial.println("Début de mode autonome");
  }

  if (millis() - timer2 > interval_calcul){  // calcul toutes les 10 secondes
    timer2 = millis();
    boolean virement_prevu = false;
    if (true){ //precision >= 6){  // cap stabilisé sur une trajectoire presque rectiligne
      int ecart_route = analyse_vent(cap_moyen, angleToWaypoint);
      nouvel_angle_regulateur = angle_regulateur + ecart_route;
      if (nouvel_angle_regulateur > 360){
        nouvel_angle_regulateur -= 360;
      }

      if (nouvel_angle_regulateur > (360-pres_VMG) || nouvel_angle_regulateur < pres_VMG){ // On bloque les positions face au vent
        if (angle_regulateur < 180){
          nouvel_angle_regulateur = pres_VMG;    // on louvoie en continuant sur le bord en cours
        }
        else {
          nouvel_angle_regulateur = 360 - pres_VMG;    // on louvoie en continuant sur le bord en cours
        }

      }

      if (nouvel_angle_regulateur > 150 && nouvel_angle_regulateur < 210){  // on bloque le vent arrière pour faire du largue
         if (angle_regulateur < 180){
          nouvel_angle_regulateur = portant_VMG;    // on louvoie en continuant sur le bord en cours
        }
        else {
          nouvel_angle_regulateur = 360 - portant_VMG;    // on louvoie en continuant sur le bord en cours
        }
      }

      if (angle_regulateur != pres_VMG && angle_regulateur != 360 - pres_VMG){  // si on n'est pas au près :
        if (nouvel_angle_regulateur > 0 && nouvel_angle_regulateur <= 180 && angle_regulateur > 180 && angle_regulateur <= 360){  // On passe de tribord amure à babord amure
          nouvel_angle_regulateur = 360-pres_VMG;
        }
        if (angle_regulateur > 0 && angle_regulateur <= 180 && nouvel_angle_regulateur > 180 && nouvel_angle_regulateur <= 360){  // On passe de babord amure à tribord amure
          nouvel_angle_regulateur = pres_VMG;
        }
      }

      // Test du couloir
      boolean precence_couloir = test_couloir(60); //test_couloir() retourne true si le bateau est dans le couloir
      datalog("Presence_couloir",precence_couloir);
      if (not precence_couloir){
        if (not hors_couloir){ // si on n'était pas déjà hors couloir on change la consigne pour retourner vers le couloir.
          hors_couloir = true;
          nouvel_angle_regulateur = 360 - nouvel_angle_regulateur;
        }
      }
      else {  // si on est dans le couloir :
        hors_couloir = false;
      }

      // application des consignes calculées sur les servos
      commande_barre(nouvel_angle_regulateur);
      reglage_aile_auto(nouvel_angle_regulateur);
    }
  }
}

void lecture_gps(){
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);
  if (GPS.newNMEAreceived()) {
     if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  if (timer > millis()) timer = millis();

  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    if (GPS.fix) {
      datalog("Vitesse",(int)(GPS.speed*100));
      datalog("Cap",(int) GPS.angle);
      //datalog("Nb_satellites",(int)GPS.satellites);      // /!\ semble tout faire bugger ?!!!!
      datalog("Latittude",(int)(GPS.latitudeDegrees*100000));
      datalog("Longitude",(int)(GPS.longitudeDegrees*100000));
      datalog("Heures", GPS.hour);
      datalog("Minutes", GPS.minute);
      datalog("Secondes", GPS.seconds);

      datalog("HDOP",GPS.HDOP*100);  // multiplié par 100 pour rester en int avec une bonne précision
      if(GPS.HDOP > 0 && GPS.HDOP < 100){ // vérification la validité des données reçues avant de les exploiter
        // Calcul du prochain waypoint si waypoint en cours atteint
        distanceToWaypoint = (float)TinyGPSPlus::distanceBetween(GPS.latitudeDegrees, GPS.longitudeDegrees,wp_lat[index_wpt], wp_lon[index_wpt]);
        angleToWaypoint = TinyGPSPlus::courseTo(GPS.latitudeDegrees, GPS.longitudeDegrees, wp_lat[index_wpt], wp_lon[index_wpt]);
        if (next_point(distanceToWaypoint)){
          distanceToWaypoint = (float)TinyGPSPlus::distanceBetween(GPS.latitudeDegrees, GPS.longitudeDegrees,wp_lat[index_wpt], wp_lon[index_wpt]);
          angleToWaypoint = TinyGPSPlus::courseTo(GPS.latitudeDegrees, GPS.longitudeDegrees, wp_lat[index_wpt], wp_lon[index_wpt]);
        }
        datalog("Wpt_angle",angleToWaypoint);
        datalog("Wpt_dst",distanceToWaypoint);
        datalog("Lat_next_point",int(wp_lat[index_wpt]*100000));
        datalog("Lon_next_point",int(wp_lon[index_wpt]*100000));
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  digitalWrite(13, HIGH);
  delay(5000);
  // initialisation GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPSSerial.println(PMTK_Q_RELEASE);

  // declaration des ports
  //pinMode(A16, INPUT);   // sortie du filtre passe bas lié à la PWM du switch C
  pinMode(ch3Pin, INPUT);
  attachInterrupt(ch3Pin, EchoPinISR_ch3, CHANGE);  // Pin 2 interrupt on any change

  pinMode(13, OUTPUT);       // Initialize LED pin
  barre.attach(3);
  aile.attach(2);
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization carte SD : failed");
  }
  else {
    Serial.println("initialization carte SD : OK");
  }

  // préparation du fichier txt
  datalog("init", 0);

  delay(1000);
  digitalWrite(13, LOW);
}

void loop() {
  lecture_gps();

  if (millis() - timer_mesure > 1000){  // cadencement 1 Hz
    timer_mesure = millis();
    cap_moyen = filtrage_cap((int)GPS.angle);
    datalog("Cap_moy",cap_moyen);
  }

  //ch6 = LastPulseTime_ch6;  // Reception de la donnée switch "C" de la radiocommande (position d'aile)
  ch3 = LastPulseTime_ch3; // Reception de la donnée de cap de la radiocommande
  if (ch3 > 990 && ch3 < 2000){
    angle1 = map(ch3, 990,2000,0,360)-180;   // 0 = face au vent
    if (angle1 < 0){
      angle1 += 360;
    }
  }

  datalog("Index_wpt", index_wpt);
  if (not(angle1 < 180 && angle1 > 160)){
    current_mode = 0;
    first_loop = true;
  }
  else {
    current_mode = 1;
  }

  if (current_mode == 1){
    datalog("Mode", 1);
    mode_autonome();
  }
  else if (current_mode == 0){
    datalog("Mode", 0);
    mode_semi_auto(angle1);
  }

  if (GPS.fix){
    if (millis() - timer4 > 1000 ) {  // témoin visuel sur la carte d'acroche des satelites.
      timer4 = millis();
      led_state = not(led_state);
    }
    if (led_state){
      digitalWrite(13, HIGH);
    }
    else {
      digitalWrite(13, LOW);
    }
  }
  // Cadencement du dataloggeur et informations complémentaires
  if (millis() - timer3 > interval_datalogging) {
    timer3 = millis();
    datalog("push", 0);
  }
}

// Routine d'interruption qui compte les impulsions de la radiocommande
void EchoPinISR_ch3() {
  static unsigned long startTime;
  if (digitalRead(ch3Pin)) // Gone HIGH
    startTime = micros();
  else  // Gone LOW
  LastPulseTime_ch3 = micros() - startTime;
}

// Libraries GPS
#include "TinyGPS.h"
TinyGPS gps;
#define GPS_SERIAL Serial2

#include <SD.h>
#include <Servo.h> 

File myFile;
Servo barre;
Servo aile;

int pres_VMG = 40;
int portant_VMG = 150;

int pos1 = 80 + 10;
int pos3 = 80 - 10;

int ledPin = 13;                  // LED test pin
int navLigth = 33;
int current_mode = 0;
int angle_regulateur;
int cap_moyen;
boolean first_loop = true;
boolean hors_couloir = true;

// liste points GPS
float distanceToWaypoint;
double angleToWaypoint;
float wp_lat[] = {47.731775, 47.731042};
float wp_lon[] = {-3.394241, -3.393050};
unsigned int index_wpt = 0;              // index dans la liste des wpt
int seuil_valid_wpt = 25;

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
String var_name_log[] = {"Time","HDOP", "Vitesse", "Cap", "Angle_regulateur", "Pos_aile", "Cap_moy", "Nb_satellites", "Latittude", "Longitude","Lat_next_point","Lon_next_point","Wpt_angle", "Wpt_dst","ecart_axe","Presence_couloir", "Mode","Index_wpt", "Commentaire"};
int buf[sizeof(var_name_log)];

int index_buffer_lignes = 0;
int taille_buffer_lignes = 10;
String lines_buffer[10];

// GPS data
float lat, lon;
unsigned long fix_age, time, date;
float speed, course;
unsigned long chars;
unsigned short sentences, failed_checksum;
unsigned long hdop;

// fonction de data logging
void datalog(String var_name, int value){

    if (var_name == "push") {
        // 4) Sur appel du mot clef "push", copier le buffer dans la carte sd. Et clear buffer.
        String line;
        for (unsigned int i = 0; i < sizeof(var_name_log)/sizeof(var_name_log[0]) - 1; i++){
            line += buf[i];
            line += ";";
        }
        lines_buffer[index_buffer_lignes] = line;
        // Serial1.println(line);
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
      // Serial1.println(line);
      myFile.println(line);
      myFile.close();
    }
    else {
        // 1) Trouver la position de la variable dans la trame csv
        for (unsigned int index_var = 0; index_var < (sizeof(var_name_log)/sizeof(var_name_log[0])); index_var ++){
            if (var_name == var_name_log[index_var]){
            // 2) Ajouter la valeur dans la ligne
                buf[index_var] = value;
                break;
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
    axe_parcours = TinyGPS::course_to(wp_lat[index_wpt], wp_lon[index_wpt], wp_lat[index_wpt+1], wp_lon[index_wpt+1]);
  }
  else {
    axe_parcours = TinyGPS::course_to(wp_lat[index_wpt], wp_lon[index_wpt], wp_lat[0], wp_lon[0]);
  }

  int ecart = abs(sin(radians(axe_parcours - angleToWaypoint)) * distanceToWaypoint);
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
  // L'entrée doit être comprise entre 0 et 360°. 0 correspond à face au vent
  if (angle <= 360 && angle >= 0){
    angle_regulateur = angle;
    angle = (int)(((float)angle)/3.8+90); // on s'adapte a la course du servo (180°)
    // centrage des valeurs autour de 90° (pour qu'un 0 en entrée corresponde au milieu de la course du servo : 90)
    if (angle > 180){
      angle = angle - 180;
    }
    barre.write(angle);
    datalog("Angle_regulateur",angle);
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
    boolean presence_couloir = test_couloir(20); //test_couloir() retourne true si le bateau est dans le couloir
    datalog("Presence_couloir",presence_couloir);
    if (not presence_couloir){
      if (not hors_couloir){ // si on n'était pas déjà hors couloir on change la consigne pour retourner vers le couloir.
        nouvel_angle_regulateur = 360 - nouvel_angle_regulateur;
      }
    }
    hors_couloir = !presence_couloir;

    // application des consignes calculées sur les servos
    commande_barre(nouvel_angle_regulateur);
    reglage_aile_auto(nouvel_angle_regulateur);
  }
}

void get_new_point(float actual_lat, float actual_lon, float bearing, float distance, float &new_lat, float &new_lon){
  actual_lat = radians(actual_lat);
  actual_lon = radians(actual_lon);
  bearing = radians(bearing);
  float R = 6371000;
  new_lat = asin( sin(actual_lat) * cos(distance/R) + cos(actual_lat) * sin(distance/R) * cos(bearing) );
  new_lon = actual_lon + atan2( sin(bearing) * sin(distance/R) * cos(actual_lat), cos(distance/R) - sin(actual_lat) * sin(new_lat));
  new_lat = degrees(new_lat);
  new_lon = degrees(new_lon);
}

void lecture_gps(){

  if(GPS_SERIAL.available()){
    
    char c = GPS_SERIAL.read();
    if (gps.encode(c))
    {
      // process new gps info here

      long _lat, _lon;
      // retrieves +/- lat/long in 1000000ths of a degree
      gps.get_position(&_lat, &_lon, &fix_age);
      lat = _lat;
      lon = _lon;
      
      // time in hhmmsscc, date in ddmmyy
      gps.get_datetime(&date, &time, &fix_age);
      
      // returns speed in 100ths of a knot
      speed = 0.51444444444*gps.speed()/100;
      
      // course in 100ths of a degree
      course = float(gps.course())/100;

      hdop = gps.hdop();
      
      Serial.print("time: ");
      Serial.println(time);
      Serial.print("lat: ");
      Serial.println(lat);
      Serial.print("lon: ");
      Serial.println(lon);
      Serial.print("course: ");
      Serial.println(course);
      Serial.print("speed: ");
      Serial.println(speed);
      Serial.println("====================");
      

      datalog("Vitesse",(int)(speed*100));
      datalog("Cap",(int) course);
      datalog("Latittude",(int)(lat));
      datalog("Longitude",(int)(lon));
      datalog("Date", (int)(date));
      datalog("Time", (int)(time));

      datalog("HDOP",(int)(hdop*100));  // multiplié par 100 pour rester en int avec une bonne précision
      
      if(hdop > 0 && hdop < 100){ // vérification la validité des données reçues avant de les exploiter
        // Calcul du prochain waypoint si waypoint en cours atteint
        distanceToWaypoint = (float)TinyGPS::distance_between(lat/1000000, lon/1000000, wp_lat[index_wpt], wp_lon[index_wpt]);
        angleToWaypoint = TinyGPS::course_to(lat/1000000, lon/1000000, wp_lat[index_wpt], wp_lon[index_wpt]);
        if (next_point(distanceToWaypoint)){
          distanceToWaypoint = (float)TinyGPS::distance_between(lat/1000000, lon/1000000, wp_lat[index_wpt], wp_lon[index_wpt]);
          angleToWaypoint = TinyGPS::course_to(lat/1000000, lon/1000000, wp_lat[index_wpt], wp_lon[index_wpt]);
        }
        datalog("Wpt_angle",angleToWaypoint);
        datalog("Wpt_dst",distanceToWaypoint);
        datalog("Lat_next_point",int(wp_lat[index_wpt]*1000000));
        datalog("Lon_next_point",int(wp_lon[index_wpt]*1000000));
      }
    }
  }
}

void navSetup() {
  // init serial debug
  Serial.begin(9600);

  //switch on the led
  pinMode(ledPin, OUTPUT);       // Initialize LED pin
  digitalWrite(ledPin, HIGH);
  pinMode(navLigth, OUTPUT);       // Initialize mast LED
  digitalWrite(navLigth, HIGH);
  delay(1000);
  
  // initialisation GPS
  GPS_SERIAL.begin(9600);

  // init servo
  barre.attach(7);
  aile.attach(5);

  // init SD card
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization carte SD : failed");
  }
  else {
    Serial.println("initialization carte SD : OK");
  }

  // préparation du fichier txt
  datalog("init", 0);

  // switch off the led
  delay(500);
  digitalWrite(ledPin, LOW);
}

void navLoop() {
  // Read GPS data
  lecture_gps();

  if (millis() - timer_mesure > 1000){  // cadencement 1 Hz
    timer_mesure = millis();
    cap_moyen = filtrage_cap((int)course);
    datalog("Cap_moy",cap_moyen);
  }

  datalog("Mode", 1);
  mode_autonome();
  
  // Cadencement du dataloggeur et informations complémentaires
  if (millis() - timer3 > interval_datalogging) {
    timer3 = millis();
    datalog("push", 0);
  }
}





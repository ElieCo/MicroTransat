#include <SD.h> // crée automatiquement un objet racine SD représentant la carte mémoire SD

File myDir; // objet file
boolean test=false;
char nom[50];

void setup()
{
  delay(2000);
  Serial.begin(115200);

  //----- initialisation de la carte SD -----
  Serial.println("Initialisation de la SD card...");
  test=SD.begin(BUILTIN_SDCARD);

  if (test!=true) { // si initialisation n'est pas réussie
    Serial.println("Init SD failed");
  }
  else { // si nitialisation réussie
    Serial.println("Init SD OK");

  //----- affiche le contenu du répertoire
  myDir = SD.open("/"); // ouvre la SD Card à la racine
  afficheContenu(myDir, 0); // affiche contenu d'un répertoire avec 0 tab
  }
}

void loop(){
   while(Serial.available()) {
    String a= Serial.readString();// read the incoming data as string
    if (a.substring(0,2) == "rm"){
      Serial.print("Delete file : ");
      String b = a.substring(3);
      Serial.println(b);

      if (SD.remove(b.c_str())){
        Serial.println("File deleted");
      }
      else {
        Serial.println("Delete error, please retry");
      }
    }
    else {
    a.toCharArray(nom, 50);
    readFile(nom); 
   }
  }
}

void readFile(char* dir){
  File f = SD.open(dir);
  if (f) {
    // read from the file until there's nothing else in it:
    while (f.available()) {
      Serial.write(f.read());
    }
    f.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("read error");
  }
}

void afficheContenu(File dir, int numTabs) { 

   while(true) {
     File entry =  dir.openNextFile(); // ouvre le fichier ou repertoire suivant
     if (! entry) { // si aucun nouveau fichier /repertoire
       entry.close();
       break; // sort de la fonction
     }
     
     // affiche le nombre de tab voulu - 0 si racine, 1 si sous Rép, 2si sous-sous rép, etc.. 
     for (int i=0; i<numTabs; i++) {
       Serial.print('\t');
     }

     Serial.print(entry.name()); // affiche le nom du fichier/repertoire
     
     if (entry.isDirectory()) { // si le fichier est un répertoire
       Serial.println("/"); // affiche un slash
       afficheContenu(entry, numTabs+1); // affiche le contenu en décalant d'un tab 
       }

     else { // sinon affiche la taille - les fichiers ont une taille,pas les répertoires // files have sizes, directories do not
       Serial.print("\t\t"); // affiche de Tab de décalé
       Serial.print(entry.size(), DEC); // affiche la taille
       Serial.println(" octets"); // affiche la taille
     }

    entry.close();
   }

}

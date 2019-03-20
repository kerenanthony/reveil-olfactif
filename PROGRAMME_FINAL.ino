#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "RTClib.h"
#include<LiquidCrystal_I2C.h>
#include<SoftwareSerial.h>
//inclusion des bibliothèques utiles

LiquidCrystal_I2C lcd(0x27,20,4);
SoftwareSerial BlueT(5,6);
Adafruit_NeoPixel led = Adafruit_NeoPixel(14, 10, NEO_GRB + NEO_KHZ800);
RTC_DS3231 rtc;
//initialisation des modules


const byte TRIGGER_PIN = 2;           // Broche TRIGGER du sonar
const byte ECHO_PIN = 3;  // Broche ECHO du sonar
const int buzzer = 9;
const int bouton = 8 ;
const int vapoteur =7;

int val=0;
//initialisation de la variable qui contiendra la valeur du bouton poussoir

int x=0;
int y=0;
//les variables x et y serviront de compteurs 
//pour éviter les réveils accidentels


/* Constantes pour le timeout */
const unsigned long MEASURE_TIMEOUT = 3600UL;      

/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;


void setup() {
  
  // Initialise la bande de led
  led.begin();
  led.show();
  // Initialise le port série 
  Serial.begin(9600);
  // Initialise le module bluetooth
  BlueT.begin(9600);
  // Initialise les broches du sonar
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit être à LOW au repos
  pinMode(bouton,INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(vapoteur,OUTPUT);
  Wire.begin();
  lcd.begin(20,4);
  lcd.backlight();
  lcd.home();
  rtc.begin();

 if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    rtc.adjust(DateTime(2019, 1, 13,20 , 13, 10));
  }
}


void diffuse(){
    digitalWrite(vapoteur,LOW);
//allume le vapoteur pendant seconndes afin de faire chauffer les résistances et ainsi diffuser le parfum
}

void Eteintdiffuse(){
    digitalWrite(vapoteur,HIGH);
}

void reveilSonore(){
      digitalWrite(buzzer,HIGH);
      delay(100);
      digitalWrite(buzzer,LOW);
      delay(100);
// Fonction qui allume le buzzer en propageant des impulsions sonores
// à intervalles de 100 millisecondes
}


void reveilLumineux(){
    for(int i = 0; i < 14; i++ ) { 
      led.setPixelColor(i, random(0, 223), random(0, 223), random(0, 223));       
    }
    led.show(); // on affiche 
    delay(100);
//Allume la bande de leds en modifiant la couleur respective
//de chaque led aléatoirement
}


void eteindreLed(){
    for(int i = 0; i < 14; i++ ) { 
        led.setPixelColor(i,0,0,0);       
    }
    led.show();
//Eteint toutes les leds
}


void afficheHeure(int a, int b, int c){

    lcd.setCursor(6,2);
    lcd.print(a, DEC);
    lcd.print(':');
    if(b<10){
      lcd.print("0");
    }
    lcd.print(b,DEC);
    lcd.print(":");
    if(c<10){
      lcd.print("0");
    }
    lcd.print(c,DEC);
    delay(500);
    lcd.clear();

/*Affiche l'heure sur l'écran lcd avec :
 -a l'heure,
 -b les minutes;
 -c les secondes,
*/
}


void loop() {
  
   digitalWrite(TRIGGER_PIN, HIGH);
   delayMicroseconds(10);
   digitalWrite(TRIGGER_PIN, LOW);
   //initialise le sonar
   
   val=digitalRead(bouton);//donne l'état du bouton
  
    // Mesure le temps entre l'envoi de l'impulsion ultrasonique et son écho (si il existe)
    long measure = pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT);

    // Calcul la distance à partir du temps mesuré 
    float distance_mm = measure / 2.0 * SOUND_SPEED;
    
    Serial.print(F("Distance: "));
    Serial.print(distance_mm / 10.0, 2);
    Serial.println(F("cm, "));
    Serial.println(val);

    DateTime now = rtc.now();
    int a=now.hour();   // variable stockant l'heure
    int b=now.minute(); //variable stockant les minutes
    int c=now.second(); // variable stockant les secondes
    
    int d;
    int e;
    // d et e représenteront les heure et minute du réveil,
    //envoyés via le téléphone
    
  if(BlueT.available()){
    char Data=BlueT.read();
    if(Data=='A'){
      d=BlueT.parseInt();
      Serial.println(d);
    }
    if(Data==':'){
      e=BlueT.parseInt();
      Serial.println(e);
    }
  }
//Permet de récupérer l'heure souhaitée du réveil envoyée depuis un 
//smartphone sous le format heure:minute
  
    if(c<26){
      c=c+33;
    }
    else{
      b=b+1;
      c=c-26;
    }

    if(b<53){
      b=b+7;
    }
    else{
      a=a+1;
      b=b-53;
    }
    if(a<4){
      a=a+19;
    }
    else{
      a=a-5;
    }
//règle l'horaire puisque l'horloge a été déréglée sans possibilité 
// de réactualiser l'heure à l'heure actuelle
    
 afficheHeure(a,b,c);// affiche l'heure sur l'écran

 if (a==d&&b==(e-1)){
    diffuse();
    afficheHeure(a,b,c); 
    /*if(val==0){
            Eteintdiffuse();
            delay(180000);
 //si on appuie sur le bouton, le réveil s'éteint directement
          }*/
      if ((distance_mm/10)>0){
             if(x==2){
                 if((distance_mm/10)>0){
                    Eteintdiffuse();
                    delay(180000);  
                 }
             }
          delay(1000);
          x=x+1;
          }
 
 }
 
 if(a==d&&b==e){
          Eteintdiffuse();
          if(((distance_mm/10)==0)){//&&(val==1)){
            reveilLumineux();
            afficheHeure(a,b,c);
 //fait sonner le réveil si le sonar ne détecte pas de mouvement et
 //si le bouton n'est pas "enclenché" et continue d'afficher l'heure
          }
          
          /*else if(val==0){
            eteindreLed();
            delay(120000);
 //si on appuie sur le bouton, le réveil s'éteint directement
          }*/
          
          else if ((distance_mm/10)>0){
             if(x==2){
                 if((distance_mm/10)>0){
                    eteindreLed();
                    delay(120000);  
                 }
             }
          delay(1000);
          x=x+1;
          }
//si le sonar détecte un mouvement, au bout de 3 secondes (compteur avec x) il revérifie si il y a
//toujours du mouvement, dans ce cas le réveil s'éteint ainsi que l'horloge
  }

  
  if(a==d&&b==e+1){
     
     if(((distance_mm/10)==0)){//&&(val==1)){
          reveilLumineux();
          reveilSonore();
          afficheHeure(a,b,c);
//si 1 minute plus tard, le réveil n'a pas été éteint, la bande de led s'allume en plus du buzzer 
        }
        
      else if(val==0){
            eteindreLed();
            digitalWrite(buzzer,LOW);
            delay(60000);
          }
      else if ((distance_mm/10)>0){
          if(y==2){
              if((distance_mm/10)>0){
                    eteindreLed();
                    digitalWrite(buzzer,LOW);
                    delay(60000);  
                 }
             }
            delay(1000);
            y=y+1;
          }
  }

  //diffuse();  

}

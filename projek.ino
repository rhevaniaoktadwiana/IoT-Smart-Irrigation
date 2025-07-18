#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// identifikasi konstanta 
const float k1 = 0.05; //konstanta laju penguapan oleh suhu (0,03 - 0,06)
const float k2 = 1.5; //konstanta banyak kelembapan tanah naik saat disiram pompa (1,2 - 2)
const float k3 = 0.02; //konstanta laju penguapan oleh kelembapan udara rendah (0,01 - 0,04)
const float dt = 0.5; //selang waktu

// identifikasi pin sensor suhu DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//identifikasi pin sensor kelembapan tanah Potensiometer
#define SOIL_PIN 34 //Potensiometer 

//identifikasi aktuator
#define RELAY_PIN 5 //pompa
#define LED_PIN 2 //led
LiquidCrystal_I2C lcd(0x27, 20, 4); //0x27 sebagai alamat I2C, ukuran lcd 20 kolom x 3 baris

//identifikasi variabel
float T = 0; //suhu
float H = 0; //kelembapan udara
float M = 0; //kelembapan tanah
float dM = 0; //perubahan kelembapan tanah
int u = 0; //status pompa

//identifikasi kelembapan tanah
int soilADC;

//identifikasi batas sensor
const float M_MIN = 30.0; //batas tanah kering
const float M_MAX = 70.0; //batas tanah basah

//setup awal
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin(); //mengaktifkan sensor DHT22
  lcd.init(); //menyiapkan lcd
  lcd.backlight(); //menyalakan lampu latar lcd

  pinMode(RELAY_PIN, OUTPUT); //GPIO atau pin 5 sebagai output pompa
  pinMode(LED_PIN, OUTPUT); //GPIO atau pin 2 sebagai output led
  digitalWrite(RELAY_PIN, LOW); //pompa mati
  digitalWrite(LED_PIN, LOW); //led mati

  soilADC = analogRead(SOIL_PIN);
  M = map(soilADC, 1023, 0, 0, 100);

  lcd.setCursor(0, 0);
  lcd.print("Init Soil: ");
  lcd.print(M);
  lcd.print("%");
  delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
  T = dht.readTemperature();
  H = dht.readHumidity();
  if (isnan(T) || isnan(H)) {
    Serial.println("DHT sensor failed!");
    delay(1000);
    return;
  }


  float currentSoil = map (soilADC, 1023, 0, 0, 100);

  //hitung perubahan kelembapan tanah
  dM = ((-k1 * T) - (k3 * (100 - H)) + (u * k2)) * dt;
  M += dM; //kelembapan tanah = kelembapan saat ini + perubahannya
  M = constrain(M, 0, 100);

  //logika kontrol kelembapan tanah
  if (M < M_MIN) {
    u = 1;
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
  } else if (M > M_MAX) {
    u = 0;
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
  } else { //saat kelembapan tanah diantara batas
    digitalWrite(LED_PIN, HIGH);
  }

  //Output di lcd
  Serial.println("---------------------");

  Serial.print("Temperature: "); 
  Serial.print(T); 
  Serial.print(" C \n");

  Serial.print("Air Humidity: "); 
  Serial.print(H); 
  Serial.print( " % \n");

  Serial.print("Soil Humidity: "); 
  Serial.print(M); 
  Serial.print(" % \n");

  Serial.print("Pump: "); 
  Serial.println(u == 1 ? "ON" : "OFF"); //u == 1, jika benar maka ON, jika salah maka OFF

  //Output di lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("==Smart Irigation==");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(T);
  lcd.print(" C");

  lcd.setCursor(0, 2);
  lcd.print("Air: ");
  lcd.print(H);
  lcd.print(" %"); 

  lcd.setCursor(0, 3);
  lcd.print("Soil: ");
  lcd.print(M);
  lcd.print(" %");

  lcd.setCursor(0, 4);
  lcd.print("Pump: ");
  lcd.print(u == 1 ? "ON" : "OFF");

  delay(dt * 1000);
}

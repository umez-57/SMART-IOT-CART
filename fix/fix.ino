#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial SIM900(D6, D5); // RX, TX

const char* ssid = "SMART CART"; // Replace with your network SSID
const char* password = "smart@cart123";         // Replace with your network password

ESP8266WebServer server(80);

String page = "";
char input[12];
int count = 0;

int a;
int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
int stock[4] = {5, 5, 5, 5}; // Initial stock levels for each product

double total = 0;
int count_prod = 0;

const String shopkeeperNumber = "+918905749182"; // Shopkeeper's phone number
const String customerNumber = "+916377474438";   // Customer's phone number

void setup() {
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, OUTPUT);

  Serial.begin(9600);  // Initialize hardware serial
  SIM900.begin(9600);  // Initialize software serial for GSM module
  WiFi.begin(ssid, password);
  Wire.begin(D2, D1);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   WELCOME TO       ");
  lcd.setCursor(0, 1);
  lcd.print("   SMART CART       ");

  delay(2000);

  lcd.clear();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connecting...  ");
  }
  Serial.print(WiFi.localIP());
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print(" PLZ ADD ITEMS     ");
  lcd.setCursor(0, 1);
  lcd.print("    TO CART          ");

  server.on("/", []() {
    page = "<html><head><title>E Cart using IoT</title>";
    page += "<script>function payNow() { location.reload(); }</script>";
    page += "</head><style type=\"text/css\">";
    page += "table{border-collapse: collapse;}th {background-color:  #3498db ;color: white;}table,td {border: 4px solid black;font-size: x-large;";
    page += "text-align:center;border-style: groove;border-color: rgb(255,0,0);}</style><body><center>";
    page += "<h1>Smart Shopping Cart using IoT</h1><br><br><table style=\"width: 1200px;height: 450px;\"><tr>";
    page += "<th>ITEMS</th><th>QUANTITY</th><th>COST</th></tr><tr><td>Biscuit</td><td>" + String(p1) + "</td><td>" + String(c1) + "</td></tr>";
    page += "<tr><td>Soap</td><td>" + String(p2) + "</td><td>" + String(c2) + "</td></tr><tr><td>Rice(1KG)</td><td>" + String(p3) + "</td><td>" + String(c3) + "</td>";
    page += "</tr><tr><td>Tea(50g)</td><td>" + String(p4) + "</td><td>" + String(c4) + "</td></tr><tr><th>Grand Total</th><th>" + String(count_prod) + "</th><th>" + String(total) + "</th>";
    page += "</tr></table><br><button onclick=\"payNow()\" style=\"width: 200px;height: 50px\">Pay Now</button></center></body></html>";

    server.send(200, "text/html", page);
  });

  server.begin();
}

void sendSMS(String number, String message) {
  SIM900.print("AT+CMGF=1\r");                     // AT command to send SMS message
  delay(1000);
  SIM900.print("AT + CMGS = \"");
  SIM900.print(number);
  SIM900.println("\""); // recipient's mobile number, in international format

  delay(1000);
  SIM900.println(message);                         // message to send
  delay(1000);
  SIM900.println((char)26);                        // End AT command with a ^Z, ASCII code 26
  delay(1000);
  SIM900.println();
  delay(100);                                     // give module time to send SMS
}

void loop() {
  int a = digitalRead(D3);
  if (Serial.available()) {
    count = 0;
    while (Serial.available() && count < 12) {
      input[count] = Serial.read();
      count++;
      delay(5);
    }
    if (count == 12) {
      if ((strncmp(input, "1F00500B4501", 12) == 0) && (a == 1) && stock[0] > 0) {
        lcd.setCursor(0, 0);
        lcd.print("Biscuit Added       ");
        lcd.setCursor(0, 1);
        lcd.print("Price(Rs):35.00      ");
        p1++;
        digitalWrite(D4, HIGH);
        delay(2000);
        total = total + 35.00;
        count_prod++;
        stock[0]--;
        digitalWrite(D4, LOW);
        lcd.clear();
      } else if ((strncmp(input, "1F00500B4501", 12) == 0) && (a == 1) && stock[0] == 0) {
        sendSMS(shopkeeperNumber, "Biscuit is out of stock!"); // Send SMS if the product is out of stock
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Product Out  "); 
        lcd.setCursor(0, 1);
        lcd.print("   Of Stock  ");
        delay(2000);
        lcd.clear();
      } else if ((strncmp(input, "1F004D159ED9", 12) == 0) && (a == 1) && stock[1] > 0) {
        lcd.setCursor(0, 0);
        lcd.print("Soap Added          ");
        lcd.setCursor(0, 1);
        lcd.print("Price(Rs):38.00         ");
        total = total + 38.00;
        digitalWrite(D4, HIGH);
        delay(2000);
        p2++;
        count_prod++;
        stock[1]--;
        digitalWrite(D4, LOW);
        lcd.clear();
      } else if ((strncmp(input, "1F004D159ED9", 12) == 0) && (a == 1) && stock[1] == 0) {
        sendSMS(shopkeeperNumber, "Soap is out of stock!"); // Send SMS if the product is out of stock
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Product Out  "); 
        lcd.setCursor(0, 1);
        lcd.print("   Of Stock  ");
        delay(2000);
        lcd.clear();
      } else if ((strncmp(input, "1F004D5F2C21", 12) == 0) && (a == 1) && stock[2] > 0) {
        lcd.setCursor(0, 0);
        lcd.print("Rice(1KG) Added       ");
        lcd.setCursor(0, 1);
        lcd.print("Price(Rs):55.00      ");
        total = total + 55.00;
        digitalWrite(D4, HIGH);
        delay(2000);
        count_prod++;
        p3++;
        stock[2]--;
        lcd.clear();
        digitalWrite(D4, LOW);
      } else if ((strncmp(input, "1F004D5F2C21", 12) == 0) && (a == 1) && stock[2] == 0) {
        sendSMS(shopkeeperNumber, "Rice(1KG) is out of stock!"); // Send SMS if the product is out of stock
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Product Out  "); 
        lcd.setCursor(0, 1);
        lcd.print("   Of Stock  ");
        delay(2000);
        lcd.clear();
      } else if ((strncmp(input, "1F004D542F29", 12) == 0) && (a == 1) && stock[3] > 0) {
        lcd.setCursor(0, 0);
        lcd.print("Tea(50g) Added            ");
        lcd.setCursor(0, 1);
        lcd.print("Price(Rs):45.00        ");
        total = total + 45.00;
        count_prod++;
        digitalWrite(D4, HIGH);
        p4++;
        delay(2000);
        stock[3]--;
        lcd.clear();
        digitalWrite(D4, LOW);
      } else if ((strncmp(input, "1F004D542F29", 12) == 0) && (a == 1) && stock[3] == 0) {
        sendSMS(shopkeeperNumber, "Tea(50g) is out of stock!"); // Send SMS if the product is out of stock
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Product Out  "); 
        lcd.setCursor(0, 1);
        lcd.print("   Of Stock  ");
        delay(2000);
        lcd.clear();
      } else if (strncmp(input, "54006DD99575", 12) == 0) {
        sendSMS(customerNumber, "Total bill: " + String(total)); // Send total bill to customer
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Total Prod:");
        lcd.setCursor(11, 0);
        lcd.print(count_prod);
        lcd.setCursor(0, 1);
        lcd.print("Price:");
        lcd.setCursor(6, 1);
        lcd.print(total);

        digitalWrite(D4, HIGH);
        delay(2000);

        lcd.clear();
        digitalWrite(D5, LOW);
        lcd.setCursor(0, 0);
        lcd.print("   Thank you        ");
        lcd.setCursor(0, 1);
        lcd.print("  for Shopping        ");
        digitalWrite(D4, LOW);
        total = 0; // Reset total
        count_prod = 0; // Reset count of products
      }
    }
    c1 = p1 * 35.00;
    c2 = p2 * 38.00;
    c3 = p3 * 55.00;
    c4 = p4 * 45.00;
  }
  server.handleClient();
}

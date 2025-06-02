#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial SIM900(D6, D5); // RX, TX

const char* ssid = "SMART CART";
const char* password = "smart@cart123";

ESP8266WebServer server(80);

char input[12];
int count = 0;

int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
int stock[4] = {5, 5, 5, 5}; // Initial stock levels for each product

double total = 0;
int count_prod = 0;

const String shopkeeperNumber = "+918905749182"; // Shopkeeper's phone number
const String customerNumber = "+916377474438";   // Customer's phone number

void setup() {
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, OUTPUT);

  Serial.begin(9600);
  SIM900.begin(9600);
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
  Serial.println(WiFi.localIP());
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print(" PLZ ADD ITEMS     ");
  lcd.setCursor(0, 1);
  lcd.print("    TO CART          ");

  setupWebServer();

  server.begin();
}

void setupWebServer() {
  server.on("/", []() {
    bool autoRefresh = server.hasArg("autoRefresh") ? server.arg("autoRefresh") == "on" : false;
    String autoRefreshMetaTag = autoRefresh ? "<meta http-equiv='refresh' content='3'>" : "";
    String autoRefreshCheckbox = autoRefresh ? "checked" : "";

    String webpage = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>Smart Shopping Cart</title>";
    webpage += autoRefreshMetaTag;
    webpage += "<style>body{font-family: 'Arial', sans-serif; background-color: #f0f0f0; color: #333;}";
    webpage += "table{width: 50%; margin: 20px auto; border-collapse: collapse;}";
    webpage += "th, td {border: 1px solid #ccc; padding: 10px; text-align: left;}";
    webpage += "th {background-color: #4CAF50; color: white;}";
    webpage += "tr:nth-child(even) {background-color: #f2f2f2;}";
    webpage += "button, .button {padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; display: inline-block;}";
    webpage += "button {background-color: #4CAF50; color: white; margin-right: 10px;}"; // Added margin for spacing between buttons
    webpage += ".slider {width: 40px; height: 20px; margin-left: 20px;}</style>"; // Added margin-left to space out the Auto Refresh checkbox
    webpage += "<script>function setAutoRefresh(checkbox) { var query = checkbox.checked ? '?autoRefresh=on' : '?autoRefresh=off'; window.location.search = query; }</script>";
    webpage += "</head><body>";
    webpage += "<h1 style='text-align:center;'>Smart Shopping Cart</h1>";
    webpage += "<table><tr><th>Item</th><th>Quantity</th><th>Price</th></tr>";
    webpage += "<tr><td>Biscuits</td><td>" + String(p1) + "</td><td>₹" + String(p1 * 35) + "</td></tr>";
    webpage += "<tr><td>Soap</td><td>" + String(p2) + "</td><td>₹" + String(p2 * 38) + "</td></tr>";
    webpage += "<tr><td>Rice (1KG)</td><td>" + String(p3) + "</td><td>₹" + String(p3 * 55) + "</td></tr>";
    webpage += "<tr><td>Tea (50g)</td><td>" + String(p4) + "</td><td>₹" + String(p4 * 45) + "</td></tr>";
    webpage += "<tr><td colspan='2'>Total</td><td>₹" + String(total) + "</td></tr></table>";
    webpage += "<div style='text-align:center;'>";
    webpage += "<button onclick=\"window.location.href='https://smart-cart-payment.netlify.app/'\">Proceed Payment of ₹" + String(total) + "</button>";
    webpage += "<button onclick=\"window.location.href='https://smart-cart-admin-login.netlify.app/'\">Admin Login</button>";
    webpage += "<label class='button' for='auto-refresh'>Auto Refresh:</label><input type='checkbox' id='auto-refresh' class='slider' " + autoRefreshCheckbox + " onclick='setAutoRefresh(this)'>";
    webpage += "</div>";
    webpage += "</body></html>";
    server.send(200, "text/html", webpage);
  });
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
      if ((strncmp(input, "1F00500B4501", 12) == 0) && (a == 1)) {
        processItem("Biscuit", 35.00, 0, &p1);
      } else if ((strncmp(input, "1F004D159ED9", 12) == 0) && (a == 1)) {
        processItem("Soap", 38.00, 1, &p2);
      } else if ((strncmp(input, "1F004D5F2C21", 12) == 0) && (a == 1)) {
        processItem("Rice(1KG)", 55.00, 2, &p3);
      } else if ((strncmp(input, "1F004D542F29", 12) == 0) && (a == 1)) {
        processItem("Tea(50g)", 45.00, 3, &p4);
      } else if (strncmp(input, "54006DD99575", 12) == 0) {
        sendTotalSMSAndReset();
      }
    }
    updateCosts();
  }
  server.handleClient();
}

void processItem(String item, double price, int index, int *productCounter) {
  if (stock[index] > 0) {
    lcd.setCursor(0, 0);
    lcd.print(item + " Added");
    lcd.setCursor(0, 1);
    lcd.print("Price: Rs " + String(price));
    digitalWrite(D4, HIGH);
    delay(2000);
    (*productCounter)++;
    total += price;
    stock[index]--;
    digitalWrite(D4, LOW);
    lcd.clear();

    if (stock[index] == 0) {
      sendOutOfStockSMS(item);
    }
  } else {
    sendOutOfStockSMS(item);
  }
}

void sendOutOfStockSMS(String item) {
  sendSMS(shopkeeperNumber, item + " is out of stock!");
  lcd.setCursor(0, 0);
  lcd.print("   "+item.substring(0, 8)+"  ");
  lcd.setCursor(0, 1);
  lcd.print(" Out of Stock  ");
  delay(2000);
  lcd.clear();
}

void sendTotalSMSAndReset() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Total Items: " + String(p1 + p2 + p3 + p4));
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Thank you        ");
  lcd.setCursor(0, 1);
  lcd.print("  for Shopping!        ");
  sendSMS(customerNumber, "Total bill: Rs" + String(total)); // Sending total bill to customer
  digitalWrite(D4, LOW);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" PLZ ADD ITEMS     ");
  lcd.setCursor(0, 1);
  lcd.print("    TO CART          ");
  total = 0;
  stock[0] = stock[1] = stock[2] = stock[3] = 5;  // Resetting stock levels
  p1 = p2 = p3 = p4 = 0;  // Resetting product quantities
}

void updateCosts() {
  // Recalculating costs after any transaction
}

void sendSMS(String number, String message) {
  SIM900.print("AT+CMGF=1\r");
  delay(1000);
  SIM900.print("AT + CMGS = \"");
  SIM900.print(number);
  SIM900.println("\"");
  delay(1000);
  SIM900.println(message);
  delay(1000);
  SIM900.println((char)26);
  delay(1000);
  SIM900.println();
  delay(100);
}

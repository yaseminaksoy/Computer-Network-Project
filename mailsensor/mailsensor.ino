#include <SimpleDHT.h>
#include <SPI.h>
#include <Ethernet.h>

boolean statusCheck = false;
int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

String readString;
 int mailSayisi = 0;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);

char serverMail[] = "mail.smtp2go.com";
int port = 2525;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP)

EthernetServer server(80);
EthernetClient client;
void setup() {
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  Serial.println("Sample DHT11...");

  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;

  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err=");
    Serial.println(err);
    delay(1000);
    return;
  }

  Serial.print("Sample OK: ");
  //Serial.print((int)temperature); Serial.print(" *C, ");
  //Serial.print((int)humidity); Serial.println(" H");

  // DHT11 sampling rate is 1HZ.
  delay(1500);

  // listen for incoming clients
  EthernetClient client = server.available();

  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (readString.length() < 100)
        {
          readString += c;
        }

        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          client.println("<body>");
          client.println("<div>");
          client.println("<h3>Temperature and Humidity</h3>");
          client.println("<p>Temperature :");
          client.println("<span>");
          client.println((int)temperature);
          client.println(" C </span><br> &nbsp;</p>");
          client.println("<p>Humidity :");
          client.println("<span> % ");
          client.println((int) humidity);
          client.println("</span><br> &nbsp;</p></div>");
          client.println({"<style>div {text-align: center; font-size: 25px; font-family: Times; color: purple;}"});

          client.println("</html> ");

          if (mailSayisi == 0) {
            if ((int)temperature > 26 ) {
              mailSayisi++;
              sendEmail((int)temperature);
            }
          }
          break;

        }

      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    readString = "";
  }
}

byte sendEmail(int t)
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(serverMail, port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!eRcv()) return 0;

  Serial.println(F("Sending hello"));
  //set to arduino ip
  client.println("EHLO 192.168.1.100");
  if (!eRcv()) return 0;

  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!eRcv()) return 0;

  Serial.println(F("Sending User"));
  // Change to your base64 encoded user
  client.println(F("YWdsYXJwcm9qZXNp"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending Password"));
  // change to your base64 encoded password
  client.println(F("YWdsYXJwcm9qZXNpMTYyMTIy"));
  if (!eRcv()) return 0;

  // change to your email address (sender)
  Serial.println(F("Sending From"));
  client.println("MAIL From: <aglarprojesi@gmail.com>");
  if (!eRcv()) return 0;

  // change to recipient address
  Serial.println(F("Sending To"));
  client.println("RCPT To: <seymakotan34@gmail.com>");
  if (!eRcv()) return 0;
  Serial.println(F("Sending To"));
  client.println("RCPT To: <seymakotan34@gmail.com>");
  if (!eRcv()) return 0;

  Serial.println(F("Sending DATA"));
  client.println("DATA");
  if (!eRcv()) return 0;

  Serial.println(F("Sending email"));

  // change to your address
  client.println("From: HomeALARM <aglarprojesi@gmail.com>");
  client.println("Subject: Your Subject");
  // message - add/remove lines as needed
  client.println("UYARIIIIIII Evinizin sicakligi 26 dereceyi gecmistir.");
  client.println(); //blank line
  //client.println("Sicaklik = ");
  //client.print(t, ".");
  client.println("BU ODEV BURADA BITER ŞOVMEN MİYİZ NEYİZ.");
  client.println(); //blank line
  client.println(".");
  if (!eRcv()) return 0;

  Serial.println(F("Sending QUIT"));
  client.println("QUIT");
  if (!eRcv()) return 0;

  client.stop();

  Serial.println(F("disconnected"));

  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println(F("QUIT"));

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  client.stop();

  Serial.println(F("disconnected"));
}

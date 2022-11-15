#include <SPI.h>
#include <Ethernet.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		USE_SERIAL.printf("%02X ", *src);
		src++;
	}
	USE_SERIAL.printf("\n");
}


constexpr auto kBufferSize = 900;
char message[kBufferSize] {};
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);

			    // send message to server when Connected
				    DynamicJsonDocument doc(256);

            doc.add(2);
            doc.add("5e029f64-6665-45fb-82ad-79a6e7a1c56f");
            doc.add("BootNotification");
            JsonObject doc_3 = doc.createNestedObject();
            doc_3["chargePointModel"] = "esp32_ocpp_ercan";
            doc_3["chargePointVendor"] = "125464145";

            serializeJson(doc, message);
            String ser = String(message);
            webSocket.sendTXT(ser);
            break;
            }
            
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);

			// send message to server
			// webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
    }

}

bool messageSend = true;
void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(50);
      }

    WiFiMulti.addAP("YAZILIM", "1234567890");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }
    const char cert[]  = \
    "-----BEGIN PRIVATE KEY-----\n" \
    "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC3anTLUhxh1oqU\n" \
    "v5bMzDiwDKTLyJmYzL6SQ8tKxc/s/XlBoATc5wbGCmAS+gin1ea1RWtJGpIE/A4y\n" \
    "cxDMmkDXf/IY9GApSjFBvalZrORdPi6LsYA+GC5aNn4lxU/UdtgzqhB0NupegQlx\n" \
    "GTBM6QSUTOa6UIvmHfstTE+lOq4BqNfKLvnT3MoGxcpMa1PrIgrZgHwnyyLV8zxB\n" \
    "Ef6lqWOPcxUgnk0mHQoXMlCk4atw3TMo8uG9JljDzIbco7oLKInO48YmzzOXnQPa\n" \
    "EuDMThKyMrqiZhwAk8ZAkYnnE+GUUqPs2LPqQhFoWTZ0MKRRCXEDNJNHbuv1qyj8\n" \
    "tiHd2tK5AgMBAAECggEAFYTt1k+IUTHP0U8vsRBKW9xl5tteqwLlnolJ9BhEkop0\n" \
    "NC8DNww7exeLSVqw0ok5/91YzqtJg8BzZXei2lCESExkgglE8X85C3ymPoVWhzqV\n" \
    "ETJ2iIKDiLXRQ9r0KGo3qEsdEleBlKwoyI5TIIjrzr79iJFL8qgkmL5V7120vfCn\n" \
    "e+s1/6bnScnA69f5a2es0qN8olWzH7mLdHKpipM89C/k8dvHnoifPEn/JNzToriI\n" \
    "fGuZbuX/KJk9wL3heF8Tbs28oXS76iSv8ZRmtPxvYl9YIAwIZgG8Wx4AFsFMFpr9\n" \
    "MBEGP7w1lHNA+cdhayyKFNyXSX0WApyIfjL9xv4B0QKBgQDqlFk8hAUaK8s7KMhQ\n" \
    "Rgd59LHmpFUSJin8189mag8FER31e/ShcnOBBXfG9s9GuV+gpArJMG+inTwr7WPb\n" \
    "Q5OsZ42yBH2IHiAIc/BUXEqCfJMGOQiVidQ1/rgp1ditqWPsc2zp4cQ9vPvJ7b8d\n" \
    "US6gh3UNsT/ArvWUzf/bqv53KQKBgQDIKhRzeZrAxDNDklNN6SUQBiC9ZabzPJaU\n" \
    "wX9KIVVO/DipTwgzZ7YLgCXndfKl2r0k4mJmu1owvUNfWcj97hagI4gwHo+zT54u\n" \
    "bN6ft+lIPjkBiTwVXiTV00bmmEQrD4YghLrOPYemhqsSVu4BldYWJDRpS/oGHjGI\n" \
    "QWxzQQfBEQKBgQDBBuGHUnuAVZkeTSjIJVfxmtDpIUB/drgGPu+DLrK9UKB+aEmc\n" \
    "sDkrafxt1JorcE6oOVRGyXyTKx9gZi9NNgZGT8/hNKC3aVKiogSY5njJdwjkjfnq\n" \
    "U0g0Ri30/usVu9VltHVi30xEIUZvmxswXKUpo01Gvxveyhl7ISfw8nwCQQKBgEOs\n" \
    "5ok8XjQ6odKA0KWQ5DUMvVkL22x12bulyHG532v7HvUvgWhP8l7lDuu5Fzc4Q6cK\n" \
    "25Y8VfwQoYzFgI1KSGAQY2VRj+hiTOsJaCO8PKVuVDvOuH/I+s9IxboFVVbxwrmP\n" \
    "5tEAQLLu6TwkJAhpLp8B0q6fP4N+BeU5qX82R3bhAoGBAK2W4+8ZR6D0vUTPH25+\n" \
    "5ZO3dsQwcQHllLhgd+vn0TFiNh9lS+l1GNZGUkez3w6nZD/UaphNhAwLYP4vrPxZ\n" \
    "6hgHrf5CPh0fC1XQ9rf10/wpFDDe2Si/O+4rxKt47QrMSIda3vzrpMm29fv2soLs\n" \
    "u0TclhfR8G8IfA8Lpo7PixmN\n" \
    "-----END PRIVATE KEY-----\n" \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDazCCAlOgAwIBAgIUTDPFT+rr2IgZV1MmXATYSrooeDcwDQYJKoZIhvcNAQEL\n" \
    "BQAwRTELMAkGA1UEBhMCVFIxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n" \
    "GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yMjExMDkwOTE2NThaFw0yMzEx\n" \
    "MDkwOTE2NThaMEUxCzAJBgNVBAYTAlRSMRMwEQYDVQQIDApTb21lLVN0YXRlMSEw\n" \
    "HwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwggEiMA0GCSqGSIb3DQEB\n" \
    "AQUAA4IBDwAwggEKAoIBAQC3anTLUhxh1oqUv5bMzDiwDKTLyJmYzL6SQ8tKxc/s\n" \
    "/XlBoATc5wbGCmAS+gin1ea1RWtJGpIE/A4ycxDMmkDXf/IY9GApSjFBvalZrORd\n" \
    "Pi6LsYA+GC5aNn4lxU/UdtgzqhB0NupegQlxGTBM6QSUTOa6UIvmHfstTE+lOq4B\n" \
    "qNfKLvnT3MoGxcpMa1PrIgrZgHwnyyLV8zxBEf6lqWOPcxUgnk0mHQoXMlCk4atw\n" \
    "3TMo8uG9JljDzIbco7oLKInO48YmzzOXnQPaEuDMThKyMrqiZhwAk8ZAkYnnE+GU\n" \
    "UqPs2LPqQhFoWTZ0MKRRCXEDNJNHbuv1qyj8tiHd2tK5AgMBAAGjUzBRMB0GA1Ud\n" \
    "DgQWBBRLk0KTpEny3sXC3lb7YJGdau/PNjAfBgNVHSMEGDAWgBRLk0KTpEny3sXC\n" \
    "3lb7YJGdau/PNjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQBT\n" \
    "J97PlQPsDBPim7QojMSrRxwHXiR8uxVLOm4F4B0BPIBAnMIFaWxNp366H56oSZ7Z\n" \
    "HSuqHbHbTtlmhM+VDue0F4fFP7eG8xtUQ37dpHi0qBTe4gUcKHjGstcrcqnaBf68\n" \
    "iHHiY07pYjOZ1YQfs/FB5BsBIp9l670ZXIf1plmnWMjH0pMIYtGo0p3E6txzDTka\n" \
    "06Sw2b/mAtvgJZuOHrH6kVgrIyogZdaAlx1y0CjZbRPgRfXCtKorXM9VZYXk+bGh\n" \
    "2BchWPfssoE2ugHpjrWn5p1XM7c40PWLJK6XKd1eRO2Y74V4se7us+Lq0kPtm6wx\n" \
    "55++Lbe3KkRNFSG7tFZC\n" \
    "-----END CERTIFICATE-----\n";

    //webSocket.beginSSL("192.168.1.164", 9000);
    webSocket.beginSslWithCA("192.168.1.164",9000,"/100100",NULL,"ocpp1.6");
    //webSocket.setExtraHeaders();
    //webSocket.setAuthorization();


    webSocket.onEvent(webSocketEvent);

}

void loop() {
  webSocket.loop();
}



/*#include <SPI.h>
#include <WiFi.h>
 #include <WiFiClientSecure.h>
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}
void setup() {
  Serial.begin(115200);
  Ethernet.init(5);
  while (!Serial) {
    ; 
  }

  if (Ethernet.begin(mac) == 0) 
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    for (;;)
      ;
  }
  printIPAddress();
}

void loop() {
}

*/
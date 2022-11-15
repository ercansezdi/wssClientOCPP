#include <SPI.h>
#include <Ethernet.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <pgmspace.h>

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

enum states { available, occupied, reserved, unavailable};
enum error_codes {no_cable , no_error, missing_param, unknown_connector_id , unknown_connector_type, unknown_evse};
enum transactionEvent  {ended, started, updated};
enum ChargingStateType  {charging, ev_connected, suspended_ev, suspended_evse, idle};
enum TriggerReasonType  {authorized, cable_plugged_in, ev_communication_lost, ev_connect_timeout, trigger, remote_stop, remote_start, reset_command, stop_authorized,abnormal_condition};
bool waitAnswers[2] = {false,false}; //bootNotification
String getStringFromEnumStates(states e);
String getStringFromEnumError(error_codes e);
String getStringFromEnumTransactionEvent(transactionEvent e);
String getStringFromEnumTransactionTriggerReason(TriggerReasonType e);
String getStringFromEnumTransactionChargingState(ChargingStateType e);
void bootNotification(String chargePointModel,String chargePointVendor );
String get_unique_text();
char *  create_timestamp();
void heartbeat();
void againConnect();
char timestamp[20];
unsigned long currenttime=millis();
unsigned long heartBeatTimeSetter=millis();
unsigned long heartbeatTime = 120*1000;

String DEVICEID = "100100";
constexpr auto kBufferSize = 900;
char message[kBufferSize] {};
String stringMessage;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) 
{
    //Serial.printf("[WSc][ALL] Receive Message : %s\n",  payload);

    switch(type) {
        case WStype_DISCONNECTED:
        {
            Serial.printf("[WSc] Disconnected!\n");
            webSocket.disconnect();
            //while(!webSocket.isConnected())
            //{
            //delay(10);}
            break;
        }
        case WStype_CONNECTED:
        {
            bootNotification("Ercan","Sezdi");
            waitAnswers[0] = true;
          break;
        }
            
        case WStype_TEXT:
        {
          Serial.print("[WSc][ANSWER] Receive Message :");
          Serial.printf("%s\n",payload);
          if(waitAnswers[0])
          {
            StaticJsonDocument<192> doc;

            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
            }
            JsonObject root_2 = doc[2];
            const char* root_2_currentTime = root_2["currentTime"]; // "2022-11-15T12:49:12Z"
            int root_2_interval = root_2["interval"]; // 60
            heartbeatTime = root_2_interval*1000;
            const char* root_2_status = root_2["status"]; // "Accepted
            if(root_2_status == "Rejected")
            {
              while(true)
              {
                ;
              }
            }
            waitAnswers[0] = false;
          }
          break;
        }

		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
    }

}

void setup() {
    Serial.begin(115200);


    WiFiMulti.addAP("YAZILIM", "1234567890");
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    const char cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDazCCAlOgAwIBAgIUTDPFT+rr2IgZV1MmXATYSrooeDcwDQYJKoZIhvcNAQEL
BQAwRTELMAkGA1UEBhMCVFIxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM
GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yMjExMDkwOTE2NThaFw0yMzEx
MDkwOTE2NThaMEUxCzAJBgNVBAYTAlRSMRMwEQYDVQQIDApTb21lLVN0YXRlMSEw
HwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwggEiMA0GCSqGSIb3DQEB
AQUAA4IBDwAwggEKAoIBAQC3anTLUhxh1oqUv5bMzDiwDKTLyJmYzL6SQ8tKxc/s
/XlBoATc5wbGCmAS+gin1ea1RWtJGpIE/A4ycxDMmkDXf/IY9GApSjFBvalZrORd
Pi6LsYA+GC5aNn4lxU/UdtgzqhB0NupegQlxGTBM6QSUTOa6UIvmHfstTE+lOq4B
qNfKLvnT3MoGxcpMa1PrIgrZgHwnyyLV8zxBEf6lqWOPcxUgnk0mHQoXMlCk4atw
3TMo8uG9JljDzIbco7oLKInO48YmzzOXnQPaEuDMThKyMrqiZhwAk8ZAkYnnE+GU
UqPs2LPqQhFoWTZ0MKRRCXEDNJNHbuv1qyj8tiHd2tK5AgMBAAGjUzBRMB0GA1Ud
DgQWBBRLk0KTpEny3sXC3lb7YJGdau/PNjAfBgNVHSMEGDAWgBRLk0KTpEny3sXC
3lb7YJGdau/PNjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQBT
J97PlQPsDBPim7QojMSrRxwHXiR8uxVLOm4F4B0BPIBAnMIFaWxNp366H56oSZ7Z
HSuqHbHbTtlmhM+VDue0F4fFP7eG8xtUQ37dpHi0qBTe4gUcKHjGstcrcqnaBf68
iHHiY07pYjOZ1YQfs/FB5BsBIp9l670ZXIf1plmnWMjH0pMIYtGo0p3E6txzDTka
06Sw2b/mAtvgJZuOHrH6kVgrIyogZdaAlx1y0CjZbRPgRfXCtKorXM9VZYXk+bGh
2BchWPfssoE2ugHpjrWn5p1XM7c40PWLJK6XKd1eRO2Y74V4se7us+Lq0kPtm6wx
55++Lbe3KkRNFSG7tFZC
-----END CERTIFICATE-----
)EOF";

    webSocket.beginSslWithCA("192.168.1.164",9000,"/100100",NULL,"ocpp2.0.1");
    webSocket.setAuthorization("100100","123qweASDzxc");


    webSocket.onEvent(webSocketEvent);

}

void loop() 
{
  currenttime=millis();
  if(currenttime - heartBeatTimeSetter > heartbeatTime) 
  {
    heartBeatTimeSetter = millis();
    heartbeat();
  }
  if(webSocket.isConnected())
  {againConnect();}
  webSocket.loop();

}
void againConnect()
{
  webSocket.beginSslWithCA("192.168.1.164",9000,"/100100",NULL,"ocpp2.0.1");
    webSocket.setAuthorization("100100","123qweASDzxc");


    webSocket.onEvent(webSocketEvent);


}

void bootNotification(String chargePointModel,String chargePointVendor )
{
  DynamicJsonDocument doc(256);
  doc.add(2);
    doc.add(get_unique_text());
    doc.add("BootNotification");
    JsonObject doc_3 = doc.createNestedObject();
    JsonObject doc_3_chargingStation = doc_3.createNestedObject("chargingStation");
    doc_3_chargingStation["model"] = chargePointModel;
    doc_3_chargingStation["vendorName"] = chargePointVendor;
    doc_3["reason"] = "Unknown";

  serializeJson(doc, message);
  stringMessage = String(message);
  webSocket.sendTXT(stringMessage);
}

void heartbeat()
  {
    DynamicJsonDocument doc(128);
    doc.add(2);
    doc.add(get_unique_text());
    doc.add("Heartbeat");
    JsonObject doc_3 = doc.createNestedObject();
    serializeJson(doc, message);
    stringMessage = String(message);
    webSocket.sendTXT(stringMessage);
  }
void authorize(String idTag) 
  {
    DynamicJsonDocument doc(256);
    doc.add(2);
    doc.add(get_unique_text());
    doc.add("Authorize");
    JsonObject doc_3_idToken = doc[3].createNestedObject("idToken");
    JsonObject doc_3_idToken_additionalInfo_0 = doc_3_idToken["additionalInfo"].createNestedObject();
    doc_3_idToken_additionalInfo_0["additionalIdToken"] = idTag;
    doc_3_idToken_additionalInfo_0["type"] = "VID";
    doc_3_idToken["idToken"] = idTag;
    doc_3_idToken["type"] = "MacAddress";
    serializeJson(doc, message);
    stringMessage = String(message);
    webSocket.sendTXT(stringMessage);
  }
void statusNotification(int connectorID, states status, error_codes error, int evseId) 
  {
    DynamicJsonDocument doc(256);

    doc.add(2);
    doc.add(get_unique_text());
    doc.add("StatusNotification");
    JsonObject doc_3 = doc.createNestedObject();
    doc_3["connectorId"] = connectorID;
    doc_3["connectorStatus"] = getStringFromEnumStates(status);
    doc_3["evseId"] = evseId;
    doc_3["timestamp"] = create_timestamp();
    serializeJson(doc, message);
    stringMessage = String(message);
    webSocket.sendTXT(stringMessage);
  }
char *  create_timestamp()
  {
    
    sprintf (timestamp, "%4d-%02d-%02dT%02d:%02d:%02dZ", "1996", "11", "05", "09", "09", "09");
    return timestamp;
  }

String getStringFromEnumTransactionChargingState(ChargingStateType e)
{
  switch (e)
  {
    case charging: return "Charging";
    case ev_connected: return "EVConnected";
    case suspended_ev: return "SuspendedEV";
    case suspended_evse: return "SuspendedEVSE";
    case idle: return "Idle";
    default: return "SG";
  }
}

String getStringFromEnumTransactionTriggerReason(TriggerReasonType e)
{
  switch (e)
  {
    case authorized: return "Authorized";
    case cable_plugged_in: return "CablePluggedIn";
    case ev_communication_lost: return "EVCommunicationLost";
    case ev_connect_timeout: return "EVConnectTimeout";
    case remote_stop: return "RemoteStop";
    case remote_start: return "RemoteStart";
    case reset_command: return "ResetCommand";
    case stop_authorized: return "StopAuthorized";
    case abnormal_condition: return "AbnormalCondition";
    default: return "SG";
  }
}

String getStringFromEnumTransactionEvent(transactionEvent e)
{
  switch (e)
  {
    case ended: return "Ended";
    case started: return "Started";
    case updated: return "Updated";
    default: return "SG";
  }
}

String  getStringFromEnumStates(states e)
{
  switch (e)
  {
    case available: return "Available";
    case occupied: return "Occupied";
    case reserved: return "Reserved";
    case unavailable: return "Unavailable";
    default: return "Faulted";
  }
}

String getStringFromEnumError(error_codes e)
{
  switch (e)
  {
    case no_cable: return "NoCable";
    case no_error: return "NoError";
    case missing_param: return "MissingParam";
    case unknown_connector_id: return "UnknownConnectorId";
    case unknown_connector_type: return "UnknownConnectorType";
    case unknown_evse: return "UnknownEvse";
    default: return "SG";
  }
}

String get_unique_text()
{
  //randomSeed(analogRead(A3));
  bool exit2 = true;
  bool exit1 = true;
  String uniq_kod = "";
  String  randString;
  String oldKey  = "";
  char letters[36] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
  int j;
  int uniqKeyLenght[5] = {2, 4, 4, 4, 12};

  while (exit1)
  {
    uniq_kod = "";
    uniq_kod += DEVICEID;
    j = 0;
    for ( j = 0 ; j < 5 ; j++)
    {
      randString = "";
      exit2 = true;
      while (exit2)
      {
        randString = randString + letters[random(0, 36)];
        if (randString.length() >= uniqKeyLenght[j])
        {
          uniq_kod = uniq_kod + randString;
          exit2 = false;
        }
      }
      if (j != 4)
        uniq_kod = uniq_kod + "-";
    }
    if (oldKey != uniq_kod)
    {
      oldKey = uniq_kod;
      exit1 = false;
    }
    else
    {
      uniq_kod = "";
      uniq_kod += DEVICEID;
    }
  }
  return uniq_kod;

}
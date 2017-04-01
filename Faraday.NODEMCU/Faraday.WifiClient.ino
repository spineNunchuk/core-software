#if defined(ENABLEWIFI)

typedef union
{
 float number;
 char  bytes[4];
} FLOATUNION_t;

const unsigned int UDP_Port = 9999;
#if defined(ENABLEWIFINUNCHUK)
WiFiUDP portUDP;
IPAddress addressUDP(10, 10, 100, 97);

#define UPD_NO_USER_CONNECTED      0u
#define UPD_VALID_USER_CONNECTED   1u

char packetBuffer[255];
char packetBufferTx[255];
char dataHeaderData[] = "$DATA:";
char dataHeaderConnect[] = "$CONNECT:";
uint8_t udp_nunchuckState = UPD_NO_USER_CONNECTED;
IPAddress UDP_NunchukAddress(10, 10, 100, 97); /* Initial IP, to be updated later */
#endif
IPAddress address(10, 10, 100, 254);
IPAddress subnet(255, 255, 255, 0);

void setupWIFI()
{
  Serial.print("Configuring access point...");

  //If we dont disable the wifi, then there will be some issues with connecting to the device sometimes
  WiFi.disconnect(true);
  byte channel = 11;
  float wifiOutputPower = 20.5; //Max power
  WiFi.setOutputPower(wifiOutputPower);
  WiFi.setPhyMode(WIFI_PHY_MODE_11B);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP);
  //C:\Users\spe\AppData\Roaming\Arduino15\packages\esp8266\hardware\esp8266\2.1.0\cores\esp8266\core_esp8266_phy.c
  //TRYING TO SET [114] = 3 in core_esp8266_phy.c 3 = init all rf

  WiFi.persistent(false);
  WiFi.softAPConfig(address, address, subnet);
  WiFi.softAP(ssid, password, channel);
  IPAddress myIP = WiFi.softAPIP();

  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
  //Set delay = true retarts the esp in version 2.1.0, check in later versions if its fixed
  server.setNoDelay(true);

  #if defined(ENABLEWIFINUNCHUK)
  portUDP.begin(UDP_Port);
  udp_nunchuckState = UPD_NO_USER_CONNECTED;
  #endif
}

#if defined(ENABLEWIFINUNCHUK)
void Wifi_sendUdpPacket(mc_values *values)
{
    uint8_t iloop;
    uint8_t checksum = 0;
   
    if (udp_nunchuckState == UPD_VALID_USER_CONNECTED)
    {
       portUDP.beginPacket(UDP_NunchukAddress, UDP_Port);
       
       /* Build packet information Header */
       portUDP.write(dataHeaderData);
       for (iloop = 0; iloop < (sizeof(dataHeaderData)-1); iloop++) {
           checksum += dataHeaderData[iloop];
       }
       yield();
       /* Build packet information Data */
       checksum += Upd_SendFloat(values->v_in);
       checksum += Upd_SendFloat(values->current_motor);
       checksum += Upd_SendFloat(values->current_in);
       checksum += Upd_SendFloat(values->rpm);
       checksum += Upd_SendFloat(values->amp_hours);
       checksum += Upd_SendFloat(values->amp_hours_charged);
       checksum += Upd_SendFloat(values->watt_hours);
       checksum += Upd_SendFloat(values->watt_hours_charged);
       checksum += Upd_SendInt(values->tachometer);
       checksum += Upd_SendInt(values->tachometer_abs);
       /* Build packet information: EndOfPaquet */
       portUDP.write('*');
       checksum += '*';
       portUDP.write(checksum);
       yield();
       portUDP.endPacket(); 
    } 
    else{
        /* Waiting to receive a UDP "connect" packet from Nunchuck, so reset timeout monitoirn */
        RefreshNunchukTimeout();
    }
}

void Wifi_ResetUdpState(void){
    /* Change state to force Nunchuk reconnection */
    udp_nunchuckState = UPD_NO_USER_CONNECTED;
}

void Wifi_receiveUdpPacket(void)
{
    /* Check for incomming new packet!! */
    int packetSize = portUDP.parsePacket();
    uint8_t new_packed_valid = 0;
    
    int iloop;
    
    if (packetSize) {
        int len = portUDP.read(packetBuffer, 254);
        if (len > 0) packetBuffer[len] = 0; /* in case of string, assure last 0 position */
        yield();
        /* Verify packet validity */
        new_packed_valid = Wifi_CheckUdpPacket(packetBuffer, len);
        yield();
        if (new_packed_valid == 1u){
            RefreshNunchukTimeout();
            byte data_position = sizeof(dataHeaderData) - 1;
            ProcessPacketDataNunchuk(&packetBuffer[data_position]);
            yield();
            #if defined(ENABLEDEVMODE)
            Serial.println("NunOK");
            #endif
        }
        else if (new_packed_valid == 2u){
            RefreshNunchukTimeout();
            /* Get the IP of the nunchuck connected */
            UDP_NunchukAddress = portUDP.remoteIP();
            Serial.print("IP: ");
            Serial.println(UDP_NunchukAddress);
            /* Change state to start sending VESC data */
            udp_nunchuckState = UPD_VALID_USER_CONNECTED;
            #if defined(ENABLEDEVMODE)
            Serial.println("NunCON");
            #endif
        }
        else{
            #if defined(ENABLEDEVMODE)
            Serial.println("NOK");
            #endif
        }
    }
}


uint8_t Wifi_CheckUdpPacket(char* dataBuff, char len)
{
    int iloop;
    
    /* Verify header information */
    int comp_headerData = memcmp(dataBuff, dataHeaderData, sizeof(dataHeaderData)-1);
    int comp_headerConnect = memcmp(dataBuff, dataHeaderConnect, sizeof(dataHeaderConnect)-1);
    
    /* Calculate checksum of the packet */
    char checksum = 0;
    for (iloop = 0; iloop < (len - 1); iloop++) {
        checksum += dataBuff[iloop];
    }
   
    /* Check packet validity */
    if (!comp_headerData & (checksum == dataBuff[len-1])) {
        return 1; /* Valid Data packet received! */
    }
    else if (!comp_headerConnect & (checksum == dataBuff[len-1])) {
        return 2; /* Valid Connection packet received! */
    }
    else {
        return 0;  /* No valid packet received... */
    }
}

#endif

void hasClients()
{
  uint8_t i;
  //Check if there are any new clients
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i])
          serverClients[i].stop();
        serverClients[i] = server.available();
        serverClients[i].setNoDelay(true);
#if defined(ENABLEDEVMODE)
        Serial.print("New client: ");
        Serial.println(i);
#endif
        continue;
      }
      //Lets allow for background work
      yield();
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
    yield();
  }
}

void readFromWifiClient()
{
  uint8_t i;
  //Check clients for data
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      unsigned int timeout = 3;
      unsigned long timestamp = millis();

      while (serverClients[i].available() == 0 && ((millis() - timestamp) <= timeout))
        yield();

      unsigned int streamLength = serverClients[i].available();
      if (streamLength > 0)
      {
#if defined(ENABLEDEVMODE)
        Serial.println("available:");
        Serial.println(streamLength);
#endif
        //size_t len
        byte packetLength = 11;
        byte packetCount = 0;
        byte m[packetLength];

        unsigned int packages = int(float(streamLength / packetLength));
#if defined(ENABLEDEVMODE)
        Serial.println("packages:");
        Serial.println(packages);
#endif
        if (packages > 0) {
          //Read all the first packages and keep only the last one
#if defined(ENABLEDEVMODE)
          Serial.print("Skipped packets:");
          Serial.println((packages - 1) * packetLength);
#endif
          for (uint8_t ii = 0; ii < (packages - 1) * packetLength; ii++)
          {
            serverClients[i].read();
          }
          for (uint8_t ii = 0; ii < packetLength; ii++)
          {
            m[packetCount++] = serverClients[i].read();
          }
          yield();


#if defined(ENABLEDEVMODE)
          unsigned long inputduration = millis() - lastinputduration;
          if (inputduration < mininputduration)
            mininputduration = inputduration;

          if (inputduration > maxinputduration)
            maxinputduration = inputduration;
          lastinputduration = millis();

          Serial.println("Inputduration:");
          Serial.println(inputduration);
          Serial.println("mininputduraton:");
          Serial.println(mininputduration);
          Serial.println("maxinputduration:");
          Serial.println(maxinputduration);
#endif
          if (validateChecksum(m, packetCount))
          {
            yield();
            //Set the power and specify controller 1, the wifi reciever
            if (controlType == 0 || controlType == 1)
            {
              controlEnabled = true;
              setPower(m[4], 1);
            }
          }
          else
          {
            //Checksum invalid
          }
        }
      }
    }
  }
}

byte getChecksum(byte* array, byte arraySize)
{
  long validateSum = 0;
  for (byte i = 0; i < arraySize; i++) {
    validateSum += array[i];
  }
  validateSum -= array[arraySize - 1];
  return validateSum % 256;
}

bool validateChecksum(byte* array, byte arraySize)
{
  return array[arraySize - 1] == getChecksum(array, arraySize);
}


void FloatToBytes(float val, char b[4])
{
  FLOATUNION_t float_temp;
  
  float_temp.number = val;
  
  b[3] = float_temp.bytes[3];
  b[2] = float_temp.bytes[2];
  b[1] = float_temp.bytes[1];
  b[0] = float_temp.bytes[0];
}

uint8_t Upd_SendFloat(float in_data)
{
    FLOATUNION_t float_temp;
    uint8_t checksum_data = 0;
    uint8_t iloop;
    
    float_temp.number = in_data;
    for (iloop = 0; iloop < 4; iloop++) {
        checksum_data += float_temp.bytes[iloop];
        portUDP.write(float_temp.bytes[iloop]);
    }
    return checksum_data;
}

uint8_t Upd_SendInt(int32_t in_data)
{
    uint8_t checksum_data = 0;
    uint8_t iloop;
    uint8_t data;

    for (iloop = 0; iloop < 4; iloop++) {
        data = (uint8_t)((int32_t)in_data >> (iloop * 8u));
        checksum_data += data;
        portUDP.write(data);
    }
    return checksum_data;
}
#endif

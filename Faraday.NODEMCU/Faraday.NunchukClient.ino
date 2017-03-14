#if defined(ENABLEWIFINUNCHUK)

//The max number of missing frames before setting default power
int maxNunchukFrameMissingCount = 10;

//Nunchuk configuration
byte defaultNunchukMinBrake = 117;
byte defaultNunchukMaxBrake = 10;
byte defaultNunchukMinAcceleration = 130;
byte defaultNunchukMaxAcceleration = 254;
byte defaultNunchukNeutral = 127;


byte wifi_NunchukX = 0;
byte wifi_NunchukY = 0;
byte wifi_NunchukIsZPressed = 0;
byte wifi_NunchukIsCPressed = 0;
byte wifi_NunchukIsOK = 0;

byte wifi_nunchuk_fresh = 0;

void RefreshNunchukTimeout(void){
   /* mark data as fresh! new!! */
   wifi_nunchuk_fresh = 0;
}

void ProcessPacketDataNunchuk(char* dataBuff)
{
   /* Get the nunchuck data from the buffer */
   byte data_position = 0;
   wifi_NunchukX = dataBuff[data_position++];
   wifi_NunchukY = dataBuff[data_position++];
   wifi_NunchukIsZPressed = dataBuff[data_position++];
   wifi_NunchukIsCPressed = dataBuff[data_position++];
   wifi_NunchukIsOK = dataBuff[data_position++];
   yield();
   
   /* Transfer new command to the ESC */
   ProcessNunchukCommand();
   yield();
}

void MonitorNunchukClient()
{
    wifi_nunchuk_fresh++;
    if (wifi_nunchuk_fresh >= maxNunchukFrameMissingCount){
      //The nunchuk is not connected, wifi conection lost...
      if (controlType == 2)
      {
        setDefaultPower();
        Wifi_ResetUdpState();
        #if defined(ENABLEDEVMODE)
        Serial.println("NUNCHUK TIMEOUT!!!");
        #endif
      }
    }

}

void ProcessNunchukCommand()
{
    /* Get data from upd packets */
    if (wifi_NunchukIsOK)
    {
      if (controlType == 0 || controlType == 2)
      {
        int nunchukY = constrain((byte)wifi_NunchukY, defaultNunchukMaxBrake, defaultNunchukMaxAcceleration);
        #if defined(ENABLEDEVMODE)
        Serial.print("nunchukY: ");
        Serial.println(nunchukY);
        #endif
        if (((nunchukY > defaultNunchukMinBrake) && (nunchukY < defaultNunchukMinAcceleration) ) || (wifi_NunchukIsZPressed == 0))
        {
          //Neutral
          #if defined(ENABLEDEVMODE)
          Serial.print("NEUTRAL: ");
          Serial.println(defaultInputNeutral);
          #endif
          controlEnabled = true;
          setPower(defaultInputNeutral, 2);
        }
        else if (nunchukY >= defaultNunchukMinAcceleration)
        {
          byte input = map(nunchukY, defaultNunchukMinAcceleration, defaultNunchukMaxAcceleration, defaultInputMinAcceleration, defaultInputMaxAcceleration);
          #if defined(ENABLEDEVMODE)
          Serial.print("ACCEL: ");
          Serial.println(input);
          #endif
          controlEnabled = true;
          setPower(input, 2);
        }
        else
        {
          byte input = map(nunchukY, defaultNunchukMinBrake, defaultNunchukMaxBrake, defaultInputMinBrake, defaultInputMaxBrake);
          #if defined(ENABLEDEVMODE)
          Serial.print("BRAKE: ");
          Serial.println(input);
          #endif
          controlEnabled = true;
          setPower(input, 2);
        }
      }
    }
    else
    {
      /* data frames received had too many of the same checksum values in a row */
      setDefaultPower();
    }

}


#endif

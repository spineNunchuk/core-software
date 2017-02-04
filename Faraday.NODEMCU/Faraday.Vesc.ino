#if  defined(ENABLEVESC)

/**
 * Setup the VESC Interface.
 */

void setupVESC()
{
  // Initialize VESC Communication
  vesc.init(vescSend, vescProcess, valuesCallback);
  //Call this method every millisecond
  vescTicker.attach_ms(1, triggerUpdate, 0);
  vescGetValuesTicker.attach_ms(2000, vescGetValues, 0);
}


// Callback function for the payloads comming in from the VESC motorcontroller.
void vescProcess(unsigned char b)
{
  vesc.process(b);
}
// Required to be able to read from data from vesc.
void triggerUpdate(int i)
{
  vesc.update();
}


// This is called every 2 seconds by the vescGetValuesTicker()
// This requests values from the VESC
void vescGetValues(int i) {
  vesc.get_values();
}

/**
 * Process data sent from VESC
 * TODO: Now this function can only process one type of output.
 * 		 We need some sort of abstraction or switch. 
 * 		 We could also do this in the Faraday.Vesc Library.
 */

void valuesCallback(mc_values *val) {
//#if defined(ENABLEDEVMODE)
  Serial.print("Input voltage: ");
  Serial.println(val->v_in);
  Serial.print("Temp: ");
  Serial.println(val->temp_pcb);
  Serial.print("Current motor: ");
  Serial.println(val->current_motor);
  Serial.print("Current in: ");
  Serial.println(val->current_in);
  Serial.print("Current motor: ");
  Serial.println(val->current_motor);
  Serial.print("Current rpm: ");
  Serial.println(val->current_motor);  
  Serial.print("Duty cycle: ");
  Serial.println(val->current_motor);  
  Serial.print("Duty cycle: ");
  Serial.println(val->duty_now * 100.0);
  Serial.print("Ah Drawn: ");
  Serial.println(val->amp_hours);
  Serial.print("Ah Regen: ");
  Serial.println(val->amp_hours_charged);
  Serial.print("Wh Drawn: ");
  Serial.println(val->watt_hours);
  Serial.print("Wh Regen: ");
  Serial.println(val->watt_hours_charged);
  Serial.print("Tacho: ");
  Serial.println(val->tachometer);  
  Serial.print("Tacho ABS: ");
  Serial.println(val->tachometer_abs);
//#endif

}


/**
 * Sending payloads to the VESC motorcontroller. 
 */

void vescSend(unsigned char *data, unsigned int len)
{

#if defined(ENABLEDEVMODE)
  Serial.println("Package Sent to VESC: ");
  Serial.print(data[0]);
  Serial.print(" | ");
  Serial.print(data[1]);
  Serial.print(" | ");
  Serial.print(data[2]);
  Serial.print(" | "); 
  Serial.print(data[3]); 
  Serial.print(" | "); 
  Serial.print(data[4]);
  Serial.print(" | ");  
  Serial.print(data[5]);
  Serial.print(" | ");  
  Serial.print(data[6]);
  Serial.print(" | ");
  Serial.print(data[7]);
  Serial.print(" | ");  
  Serial.print(data[8]);
  Serial.print(" | ");  
  Serial.print(data[9]);
  Serial.print(" | ");  
  Serial.print(data[10]);
  Serial.print(" | ");  
  Serial.print(data[11]);
  Serial.print(" | ");  
  Serial.print(data[12]);
  Serial.println(" | ");
#endif

  // Write Data to UART
  Serial.write(data, len);

}

// #endif defined(ENABLEVESC)
#endif



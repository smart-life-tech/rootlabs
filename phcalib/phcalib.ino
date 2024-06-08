#define phSensorPin A0      // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue; // Store the average value of the sensor feedback
float b;
int temporarData[10], temp;
float calibration_value = 20.34;
void setup()
{
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.println("Ready"); // Test the serial monitor
}
void loop()
{
    for (int i = 0; i < 10; i++) // Get 10 sample value from the sensor for smooth the value
    {
        temporarData[i] = analogRead(phSensorPin);
        delay(10);
    }
    for (int i = 0; i < 9; i++) // sort the analog from small to large
    {
        for (int j = i + 1; j < 10; j++)
        {
            if (temporarData[i] > temporarData[j])
            {
                temp = temporarData[i];
                temporarData[i] = temporarData[j];
                temporarData[j] = temp;
            }
        }
    }
    avgValue = 0;
    for (int i = 2; i < 8; i++) // take the average value of 6 center sample
        avgValue += temporarData[i];
    float collectedValue = (float)avgValue * 5.0 / 1023 / 6; // convert the analog into millivolt
    float voltage = collectedValue;
    collectedValue = 14 - (1.9 * collectedValue) + 1.5; // convert the millivolt into pH value
    Serial.print("pH:");
    Serial.print(collectedValue, 2);
    Serial.print(" ");
    Serial.print("    raw reading pH:");
    Serial.print(analogRead(phSensorPin));
    Serial.print(" ");
    Serial.print(" voltage mV:");
    Serial.print(voltage);
    float pH_Value = analogRead(A0);
    float Voltage = pH_Value * (5.0 / 1023.0);
    float ph_act = -5.60 * voltage + calibration_value;
    Serial.print(" voltage mV raw:");
    Serial.print(Voltage);
    Serial.print("    voltage ph calib: ");
    Serial.println(ph_act);
    int analogReading = analogRead(A0);                                                                                              // Read the analog value from the sensor
                                                                                                                                     // float pHValue = 0.0194 * analogReading - 0.4859;  // Convert the analog value to pH
    //float pHValue = -0.0003115483257838544 * analogReading * analogReading + 0.20570516936468186 * analogReading - 23.7626644577051; // Quadratic conversion
 float pHValue = -9.963651947265824e-06 * analogReading * analogReading + 0.03148564371250025 * analogReading - 3.0426979282475384; // Quadratic conversion
    Serial.print("Analog Reading: ");
    Serial.print(analogReading);
    Serial.print(" => pH Value: ");
    Serial.println(pHValue);

    delay(1000); // Wait for 1 second before taking the next reading
}
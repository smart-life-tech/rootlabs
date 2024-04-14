#define phSensorPin 0         // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue; // Store the average value of the sensor feedback
float b;
int temporarData[10], temp;

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
    float collectedValue = (float)avgValue * 5.0 / 1024 / 6; // convert the analog into millivolt
    collectedValue = 3.5 * collectedValue;                          // convert the millivolt into pH value
    Serial.print("    pH:");
    Serial.print(collectedValue, 2);
    Serial.println(" ");
    digitalWrite(13, HIGH);
    delay(800);
    digitalWrite(13, LOW);
}
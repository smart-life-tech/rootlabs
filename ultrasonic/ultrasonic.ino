// Define Trig and Echo pin:
#define trigPin D4
#define echoPin D5
int TdsSensorPin = A0;
// Define variables:
long duration;
int distance;
void setup()
{
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // Begin Serial communication at a baudrate of 9600:
  Serial.begin(115200);
}
void loop()
{
  // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance:
  distance = duration * 0.034 / 2;
  // Print the distance on the Serial Monitor (Ctrl+Shift+M):
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
  Serial.print("voltage mV:");
  Serial.print(analogRead(TdsSensorPin), 2);
  Serial.println("V   ");
   delay(1000);
}
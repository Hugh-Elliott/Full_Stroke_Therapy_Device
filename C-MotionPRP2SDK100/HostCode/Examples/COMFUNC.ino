// Pin Numbers
#define LEDHIGH     2
#define LEDLOW      3
#define BUTTON      4

const byte numBytes = 32;
byte receivedBytes[numBytes];
byte numReceived = 0;

boolean newData = false;

void setup() {
  pinMode(LEDHIGH, OUTPUT);
  pinMode(LEDLOW, OUTPUT);
  pinMode(BUTTON, INPUT);
  digitalWrite(2, LOW);
	digitalWrite(3, LOW);


  Serial.begin(9600);

}

void loop() {
  recvBytesWithStartEndMarkers();
  showNewData();
}

void recvBytesWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    byte startMarker = 0x3C;
    byte endMarker = 0x3E;
    byte rb;
   

    while (Serial.available() > 0 && newData == false) {
        rb = Serial.read();

        if (recvInProgress == true) {
            if (rb != endMarker) {
                receivedBytes[ndx] = rb;
                ndx++;
                if (ndx >= numBytes) {
                    ndx = numBytes - 1;
                }
            }
            else {
                receivedBytes[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                numReceived = ndx;  // save the number for use when printing
                ndx = 0;
                newData = true;
            }
        }

        else if (rb == startMarker) {
            recvInProgress = true;
        }
    }
}
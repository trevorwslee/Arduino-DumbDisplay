
// * needs SoftwareSerial.h
// * assume OTG adaptor or DumbDisplayWifiBridge


// --------------------------------------

// *****
// * code for reading GPS signal from NEO-7M U-BLOX module
// * . simply read and interpret fixed position data
// * . not very interesting
// *****

// will need software serial for reading data from NEO-7M U-BLOX
#include <SoftwareSerial.h>


struct GpsSignal {
  char utc_time[10];
  bool position_fixed;
  float latitude;
  float longitude;
  float altitude;
};

class GpsSignalReader {
  public:
    GpsSignalReader(SoftwareSerial& gpsSerial)
      : gpsSerial(gpsSerial) {}
    bool readOnce(GpsSignal& gpsSignal);
  private:
    float latLongConvert(float latLong);  
  private:
    const char* HEADER = "$GPGGA,";
    const int HEADER_LEN = strlen(HEADER);
  private:
    SoftwareSerial& gpsSerial;
  private:
    bool reading = false;
    int reading_header_idx = 0;
    int reading_field_idx = 0;
    char field_buffer[32];
    int read_field_count = 0;
    char utc_time[10];
    bool position_fixed = false;
    float latitude;
    float longitude;
    float altitude;
};

bool GpsSignalReader::readOnce(GpsSignal& gpsSignal) {
  if (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (reading) {
      if (reading_header_idx < HEADER_LEN) {
        if (c != HEADER[reading_header_idx++]) {
          reading = false;
          return false;
        }
      } else {
        if (c == '\r') {
          reading = false;
          if (utc_time[0] != 0) {
            memcpy(gpsSignal.utc_time, utc_time, sizeof(utc_time));
            gpsSignal.position_fixed = position_fixed;
            gpsSignal.latitude = latLongConvert(latitude);
            gpsSignal.longitude = latLongConvert(longitude);
            gpsSignal.altitude = altitude;
            return true;
          }
        }
        if (c == ',') {
          if (reading_field_idx > 0) {
            if (read_field_count == 0) {
              utc_time[0] = field_buffer[0];
              utc_time[1] = field_buffer[1];
              utc_time[2] = ':';
              utc_time[3] = field_buffer[2];
              utc_time[4] = field_buffer[3];
              utc_time[5] = ':';
              utc_time[6] = field_buffer[4];
              utc_time[7] = field_buffer[5];
              utc_time[8] = 0;
            } else if (read_field_count == 1) {
              latitude = atof(field_buffer);
            } else if (read_field_count == 3) {
              longitude = atof(field_buffer);
            } else if (read_field_count == 5) {
              position_fixed = field_buffer[0] == '1' || field_buffer[0] == '2';
            } else if (read_field_count == 8) {
              altitude = atof(field_buffer);
            }
          }
          reading_field_idx = 0;
          field_buffer[0] = 0;
          read_field_count++;
        } else {
          field_buffer[reading_field_idx++] = c;
          field_buffer[reading_field_idx] = 0;
        }
      }
    } else {
      if (c == '\n') {
        reading = true;
        reading_header_idx = 0;
        reading_field_idx = 0;
        field_buffer[0] = 0;
        read_field_count = 0;
        utc_time[0] = 0;
        position_fixed = false;
        latitude = 0;
        longitude = 0;
      }
    }
  }
  return false;
}

float GpsSignalReader::latLongConvert(float latLong) {
  int second = 60 * (latLong - (int) latLong);
  int degree = (int) (latLong / 100);
  int minute = (int) (latLong - 100 * degree);
  return (float) degree + ((60 * (float) minute + (float) second) / (60 * 60));
}  


// --------------------------------------




#include "dumbdisplay.h"


#define NEO_RX 6   // RX pin of NEO-7M U-BLOX
#define NEO_TX 5   // TX pin of NEO-7M U-BLOX


SoftwareSerial gpsSerial(NEO_TX, NEO_RX);
GpsSignalReader gpsSignalReader(gpsSerial);


// create the DumbDisplay object; assuming USB connection with 115200 baud
DumbDisplay dumbdisplay(new DDInputOutput(115200));

// declares several layers for the UI
GraphicalDDLayer* graphical;    // for showing the UTC time and position fixed, according to the readings of the GPS module
TerminalDDLayer* terminal;      // for showing trace of reading data from the GPS module
TomTomMapDDLayer *tomtommap;    // for showing the location fixed, on a TomTom map



long startMillis;  // the "time" the sketch starts running
long lastMillis;   // the "time" "waiting progress" (a dot) is printed

void setup() {
  startMillis = millis();
  lastMillis = startMillis;

  // it appears that it is better to explicityly make connection to DD app first
  dumbdisplay.connect();

  // the default UART baud rate for communicating with NEO-7M is 9600
  gpsSerial.begin(9600);

  // create the layers, as described above
  graphical = dumbdisplay.createGraphicalLayer(250, 30);
  graphical->setTextFont("DL::Ubuntu Mono");  // DL::Ubuntu Mono ==> specify that the downloadable Google font Ubuntu Monto should be used
  graphical->border(3, "black");
  graphical->padding(3);
  graphical->penColor("blue");
  graphical->backgroundColor("white");
  terminal = dumbdisplay.createTerminalLayer(600, 800);
  terminal->border(5, "blue");
  terminal->padding(5);
  tomtommap = dumbdisplay.createTomTomMapLayer("", 600, 800);  // *** a TomTom map api key is required; here, "" is used for demo purpose
  tomtommap->border(5, "blue");
  tomtommap->padding(5);
  tomtommap->visible(false);  // initially hidden

  // 'pin' the layers one by one vertically
  //  note that tomtommap is not visible initially
  dumbdisplay.configAutoPin(DD_AP_VERT);

}


bool waited = false;  // a flag indicating that the sketch waited for GPS readings
GpsSignal gpsSignal;  // a structure for receiving GPS readings

void loop() {
  long nowMillis = millis();
  if ((nowMillis - lastMillis) > 2000) {
    // if it has been 2000 milli-seconds since last print "waiting progress" (a dot), print it again
    terminal->print(".");
    waited = true;
    lastMillis = nowMillis;
  }
  if (gpsSignalReader.readOnce(gpsSignal)) {  // try read GPS data once
    // read GPS data
    if (waited) {
      // waited before (i.e. dots printed before), terminate the dots
      terminal->println();
      waited = false;
    }
    // print out the GPS info read
    terminal->print("- utc: ");  // UTC time
    terminal->print(gpsSignal.utc_time);
    terminal->print(" ... ");
    if (gpsSignal.position_fixed) {
      // if position fixed, print the latitute / longitude / altitude of the position
      terminal->print("position fixed -- ");
      terminal->print("lat:");
      terminal->print(gpsSignal.latitude);
      terminal->print(" long:");
      terminal->print(gpsSignal.longitude);
      terminal->print(" alt:");
      terminal->print(gpsSignal.altitude);
    } else {
      // position not yet fixed
      terminal->print("position NOT fixed");
    }
    terminal->println();
    // print out the UTC time, and position fixed to the graphical layer
    graphical->clear();
    graphical->setCursor(0, 0);
    graphical->println("UTC -- " + String(gpsSignal.utc_time));
    graphical->print("LOC -- ");
    if (gpsSignal.position_fixed) {
      graphical->println("LAT:" + String(gpsSignal.latitude, 4) + " / LONG:" + String(gpsSignal.longitude, 4));
    } else {
      graphical->print("not fixed");
    }
    if (gpsSignal.position_fixed) {
      // if position fixed, show the location on the TomTom map
      tomtommap->zoomTo(gpsSignal.latitude, gpsSignal.longitude);
    }
    if (((nowMillis - startMillis) > 20000) && gpsSignal.position_fixed) {
      // hide terminal, and make tomtommap visible ... notice the "20000 millis from start" requirement
      terminal->visible(false);
      tomtommap->visible(true);
    }  
    lastMillis = nowMillis;
  }
}

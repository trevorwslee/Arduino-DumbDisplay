
// * needs SoftwareSerial.h
// * assume OTG adaptor or DumbDisplayWifiBridge


// --------------------------------------

// *****
// * code for eading GPS signal from NEO-7M U-BLOX module
// * . simply read and interpret fixed position info
// * . not very interesting
// *****

// will need software serial for reading data from NEO-7M U-BLOX
#include <SoftwareSerial.h>


struct GpsSignal {
  char utc_time[10];
  bool position_fixed = false;
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

// declares serverial layers for the UI
GraphicalDDLayer* graphical;    // for showing the UTC time and position fixed, from the GPS module
TerminalDDLayer* terminal;      // for showing trace of reading data from GPS module
TomTomMapDDLayer *tomtommap;    // for showing the location fixed, on a TomTom map



long startMillis;
long lastMillis;
void setup() {
  startMillis = millis();
  lastMillis = startMillis;

  // it appears that it is better to explicityly make connection to DD app first
  dumbdisplay.connect();

  // the default UART baud rate for communicating with NEO-7M is 9600
  gpsSerial.begin(9600);


  //dumbdisplay.recordLayerSetupCommands();

  // create the layers, as described above
  graphical = dumbdisplay.createGraphicalLayer(250, 30);
  graphical->setTextFont("DL::Ubuntu Mono");
  graphical->border(3, "black");
  graphical->padding(3);
  graphical->penColor("blue");
  graphical->backgroundColor("white");
  terminal = dumbdisplay.createTerminalLayer(600, 800);
  terminal->border(5, "blue");
  terminal->padding(5);
  tomtommap = dumbdisplay.createTomTomMapLayer("", 600, 800);
  tomtommap->border(5, "blue");
  tomtommap->padding(5);
  tomtommap->visible(false);  // initially hidden

  // basically, 'pin' the layers one by one vertically
  dumbdisplay.configAutoPin(
    DD_AP_VERT_3(graphical->getLayerId(),
                 terminal->getLayerId(),
                 tomtommap->getLayerId()));

  //dumbdisplay.playbackLayerSetupCommands("ddgpsmap");
}

bool waited = false;
GpsSignal gpsSignal;
void loop() {
  long nowMillis = millis();
  if ((nowMillis - lastMillis) > 2000) {
    terminal->print(".");
    waited = true;
    lastMillis = nowMillis;
  }
  if (gpsSignalReader.readOnce(gpsSignal)) {
    if (waited) {
      terminal->println();
      waited = false;
    }
    terminal->print("- utc: ");
    terminal->print(gpsSignal.utc_time);
    terminal->print(" ... ");
    if (gpsSignal.position_fixed) {
      terminal->print("position fixed -- ");
      terminal->print("lat:");
      terminal->print(gpsSignal.latitude);
      terminal->print(" long:");
      terminal->print(gpsSignal.longitude);
      terminal->print(" alt:");
      terminal->print(gpsSignal.altitude);
    } else {
      terminal->print("position NOT fixed");
    }
    terminal->println();
    graphical->clear();
    graphical->setCursor(0, 0);
    graphical->println("UTC -- " + String(gpsSignal.utc_time));
    graphical->print("LOC -- ");
    if (gpsSignal.position_fixed) {
      graphical->println("LAT:" + String(gpsSignal.latitude, 4) + " / LONG:" + String(gpsSignal.longitude, 4));
      tomtommap->zoomTo(gpsSignal.latitude, gpsSignal.longitude);
    } else {
      graphical->print("not fixed");
    }
    if (((nowMillis - startMillis) > 20000) && gpsSignal.position_fixed) {
      // show TomTom map ... notice the "20000 millis from start" requirement
      terminal->visible(false);
      tomtommap->visible(true);
    }  
    lastMillis = nowMillis;
  }
}

#include <SoftwareSerial.h>


#define TX 6
#define RX 5

struct GpsSignal {
  char utc_time[10];
  bool position_fixed = false;
  float latutude;
  float longitude;
  float altitude;
};

class GpsSignalReader {
public:
  GpsSignalReader(SoftwareSerial& gpsSerial)
    : gpsSerial(gpsSerial) {}
  bool readOnce(GpsSignal& gpsSignal, bool logToSerial = false);
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
  float latutude;
  float longitude;
  float altitude;
};

bool GpsSignalReader::readOnce(GpsSignal& gpsSignal, bool logToSerial) {
  if (gpsSerial.available()) {
    char c = gpsSerial.read();
    //Serial.write(c);
    if (reading) {
      if (reading_header_idx < HEADER_LEN) {
        if (c != HEADER[reading_header_idx++]) {
          reading = false;
          return false;
        }
      } else {
        //Serial.write(c);
        if (c == '\r') {
          if (logToSerial) {
            Serial.print(field_buffer);  // check sum field
            Serial.println();
          }
          reading = false;
          // got all values
          if (utc_time[0] != 0) {
            if (logToSerial) {
              Serial.print("* UTC: ");
              Serial.print(utc_time);
              Serial.print(" ... ");
            }
            if (position_fixed) {
              if (logToSerial) {
                Serial.print("position fixed -- ");
                Serial.print("LAT:");
                Serial.print(latutude);
                Serial.print(" LONG:");
                Serial.print(longitude);
                Serial.print(" ALT:");
                Serial.print(altitude);
              }
            } else {
              if (logToSerial) {
                Serial.print("position NOT fixed");
              }
            }
            if (logToSerial) {
              Serial.println();
            }
            memcpy(gpsSignal.utc_time, utc_time, sizeof(utc_time));
            gpsSignal.position_fixed = position_fixed;
            gpsSignal.latutude = latLongConvert(latutude);
            gpsSignal.longitude = latLongConvert(longitude);  // the "longitude" read is actually 100 times bigger
            gpsSignal.altitude = altitude;
            return true;
          }
        }
        if (c == ',') {
          if (logToSerial) {
            Serial.print(field_buffer);
            Serial.print("|");
          }
          if (reading_field_idx > 0) {
            if (read_field_count == 0) {
              // just read UTC time
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
              latutude = atof(field_buffer);
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
        latutude = 0;
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


SoftwareSerial gpsSerial(RX, TX);
GpsSignalReader gpsSignalReader(gpsSerial);



void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);
}

GpsSignal gpsSignal;
void loop() {
  if (gpsSignalReader.readOnce(gpsSignal, true)) {
    Serial.print("- utc: ");
    Serial.print(gpsSignal.utc_time);
    Serial.print(" ... ");
    if (gpsSignal.position_fixed) {
      Serial.print("position fixed -- ");
      Serial.print("lat:");
      Serial.print(gpsSignal.latutude);
      Serial.print(" long:");
      Serial.print(gpsSignal.longitude);
      Serial.print(" alt:");
      Serial.print(gpsSignal.altitude);
    } else {
      Serial.print("position NOT fixed");
    }
    Serial.println();
  }
}

// const char* HEADER = "$GPGGA,";
// const int HEADER_LEN = strlen(HEADER);

// void loop() {
//   bool reading = false;
//   int reading_header_idx = 0;
//   int reading_field_idx = 0;
//   char field_buffer[32];
//   int read_field_count = 0;
//   char utc_time[10];
//   bool position_fixed = false;
//   float latutude;
//   float longitude;
//   float altitude;
//   while (1) {
//     if (swSerial.available()) {
//       char c = swSerial.read();
//       //Serial.write(c);
//       if (reading) {
//         if (reading_header_idx < HEADER_LEN) {
//           if (c != HEADER[reading_header_idx++]) {
//             reading = false;
//             continue;
//           }
//         } else {
//           //Serial.write(c);
//           if (c == '\r') {
//             Serial.print(field_buffer);  // check sum field
//             Serial.println();
//             reading = false;
//             // got all values
//             if (utc_time[0] != 0) {
//               Serial.print("* UTC: ");
//               Serial.print(utc_time);
//               Serial.print(" ... ");
//               if (position_fixed) {
//                 Serial.print("position fixed -- ");
//                 Serial.print("LAT:");
//                 Serial.print(latutude);
//                 Serial.print(" LONG:");
//                 Serial.print(longitude);
//                 Serial.print(" ALT:");
//                 Serial.print(altitude);
//               } else {
//                 Serial.print("position NOT fixed");
//               }
//               Serial.println();
//             }
//             continue;
//           }
//           if (c == ',') {
//             Serial.print(field_buffer);
//             Serial.print("|");
//             if (reading_field_idx > 0) {
//               if (read_field_count == 0) {
//                 // just read UTC time
//                 utc_time[0] = field_buffer[0];
//                 utc_time[1] = field_buffer[1];
//                 utc_time[2] = ':';
//                 utc_time[3] = field_buffer[2];
//                 utc_time[4] = field_buffer[3];
//                 utc_time[5] = ':';
//                 utc_time[6] = field_buffer[4];
//                 utc_time[7] = field_buffer[5];
//                 utc_time[8] = 0;
//               } else if (read_field_count == 1) {
//                 latutude = atof(field_buffer);
//               } else if (read_field_count == 3) {
//                 longitude = atof(field_buffer);
//               } else if (read_field_count == 5) {
//                 position_fixed = field_buffer[0] == '1' || field_buffer[0] == '2';
//               } else if (read_field_count == 8) {
//                 altitude = atof(field_buffer);
//               }
//             }
//             reading_field_idx = 0;
//             field_buffer[0] = 0;
//             read_field_count++;
//           } else {
//             field_buffer[reading_field_idx++] = c;
//             field_buffer[reading_field_idx] = 0;
//           }
//         }
//       } else {
//         if (c == '\n') {
//           reading = true;
//           reading_header_idx = 0;
//           reading_field_idx = 0;
//           field_buffer[0] = 0;
//           read_field_count = 0;
//           utc_time[0] = 0;
//           position_fixed = false;
//           latutude = 0;
//           longitude = 0;
//         }
//       }
//     }
//   }
//   //
//   //    while (swSerial.available()) {
//   //      char c = swSerial.read();
//   //      Serial.write(c); // pro výpis přijatých dat odkomentujte tento řádek
//   //    }
// }
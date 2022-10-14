

// --------------------------------------

// *****
// * code for reading GPS signal from NEO-7M U-BLOX module
// * . simply read and interpret fixed position data
// * . not very interesting
// *****


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






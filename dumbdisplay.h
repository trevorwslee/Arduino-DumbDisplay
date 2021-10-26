#ifndef dumbdisplay_h
#define dumbdisplay_h


#ifdef DD_4_ESP32
#include <esp_spp_api.h>
#include "HardwareSerial.h"
#endif


#define DUMBDISPLAY_BAUD 115200
#define DD_SERIAL_BAUD DUMBDISPLAY_BAUD
#define DD_WIFI_PORT 10201


#define DD_CONDENSE_COMMAND

#ifdef DD_CONDENSE_COMMAND
#define DD_RGB_COLOR(r, g, b) ("#" + String(((((r) << 8) + (g)) << 8) + (b), 16))
#else
#define DD_RGB_COLOR(r, g, b) (String(r<0?0:(r>255?255:r)) + "-" + String(g<0?0:(g>255?255:g)) + "-" + String(b<0?0:(b>255?255:b)))
#endif

#define DD_HEX_COLOR(color) ("#" + String(color, 16))

#define DD_AP_SPACER(w, h) (String("<") + String(w) + "x" + String(h) + String(">")) 
#define DD_AP_HORI "H(*)"
#define DD_AP_VERT "V(*)"
#define DD_AP_HORI_2(id1, id2) ("H(" + id1 + "+" + id2 + ")")
#define DD_AP_VERT_2(id1, id2) ("V(" + id1 + "+" + id2 + ")")
#define DD_AP_HORI_3(id1, id2, id3) ("H(" + id1 + "+" + id2 + "+" + id3 + ")")
#define DD_AP_VERT_3(id1, id2, id3) ("V(" + id1 + "+" + id2 + "+" + id3 + ")")
#define DD_AP_HORI_4(id1, id2, id3, id4) ("H(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + ")")
#define DD_AP_VERT_4(id1, id2, id3, id4) ("V(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + ")")
#define DD_AP_HORI_5(id1, id2, id3, id4, id5) ("H(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")")
#define DD_AP_VERT_5(id1, id2, id3, id4, id5) ("V(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")")
#define DD_AP_HORI_6(id1, id2, id3, id4, id5, id6) ("H(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")" + "+" + id6 + ")")
#define DD_AP_VERT_6(id1, id2, id3, id4, id5, id6) ("V(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")" + "+" + id6 + ")")


#define DD_SUPPORT_FEEDBACK_TEXT


class DDSerial {
  public:
    virtual void begin(unsigned long serialBaud) {
      Serial.begin(serialBaud);
    }
    virtual bool available() {
      return Serial.available();
    }
    virtual char read() {
      return Serial.read();
    }
    virtual void print(const String &s) {
      Serial.print(s);
    }
    virtual void print(const char *p) {
      Serial.print(p);
    }
    virtual void flush() {
      Serial.flush();
    }
};

extern DDSerial* _The_DD_Serial;


class DDInputOutput {
  public:
    DDInputOutput(unsigned long serialBaud = DD_SERIAL_BAUD): DDInputOutput(serialBaud, false, true) {
    }
    DDInputOutput* newForSerialConnection() {
      return new DDInputOutput(serialBaud, false, true);
    }
    virtual ~DDInputOutput() {
    }
    virtual bool available() {
      //return Serial.available();
      return _The_DD_Serial != NULL && _The_DD_Serial->available(); 
    }
    virtual char read() {
      //return Serial.read();
      return _The_DD_Serial != NULL ? _The_DD_Serial->read() : 0;
    }
    virtual void print(const String &s) {
      //Serial.print(s);
      if (_The_DD_Serial != NULL) _The_DD_Serial->print(s);
    }
    virtual void print(const char *p) {
      //Serial.print(p);
      if (_The_DD_Serial != NULL) _The_DD_Serial->print(p);
    }
    virtual void flush() {
      //Serial.flush();
      if (_The_DD_Serial != NULL) _The_DD_Serial->flush();
    }
    virtual void keepAlive() {
    }
    virtual void validConnection() {
    }
    virtual void preConnect() {
      if (setupForSerial) {
        if (_The_DD_Serial != NULL) _The_DD_Serial->begin(serialBaud);
        //Serial.begin(serialBaud);
      }
    }
  public:  
    bool isSerial() {
      return !backupBySerial && setupForSerial;
    }
    bool isBackupBySerial() {
      return backupBySerial;
    }
  protected:
    DDInputOutput(unsigned long serialBaud, bool backupBySerial, bool setupForSerial) {
      this->serialBaud = serialBaud;
      this->backupBySerial = backupBySerial;
      this->setupForSerial = setupForSerial;
    }
  protected:
    unsigned long serialBaud;
    bool backupBySerial;
    bool setupForSerial;
};


struct DDFeedback {
  int x;
  int y;
#ifdef DD_SUPPORT_FEEDBACK_TEXT
  String text;
#endif
};


class DDFeedbackManager {
  public: 
    DDFeedbackManager(int bufferSize);
    ~DDFeedbackManager();
    const DDFeedback* getFeedback();
#ifdef DD_SUPPORT_FEEDBACK_TEXT
    void pushFeedback(int x, int y, const String& text);
#else
    void pushFeedback(int x, int y);
#endif
  private:
    DDFeedback* feedbackArray;
    int arraySize;
    int nextArrayIdx;
    int validArrayIdx;
};


class DDLayer;


enum DDFeedbackType { CLICK, DOUBLECLICK, LONGPRESS };

/* pLayer -- pointer to the DDLayer of which "feedback" received */
/* type -- currently, only possible value if CLICK */
/* x, y -- (x, y) is the "area" on the layer where was clicked */
#ifdef DD_SUPPORT_FEEDBACK_TEXT
typedef void (*DDFeedbackHandler)(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback);
#else
typedef void (*DDFeedbackHandler)(DDLayer* pLayer, DDFeedbackType type, int x, int y);
#endif

class DDObject {
};

class DDLayer: public DDObject {
  public:
    /* set layer visibility */
    void visibility(bool visible);
    /* set layer opacity */
    /* - 0 to 255 */
    void opacity(int opacity);
    /* size unit is pixel: */
    /* - LcdLayer; each character is composed of pixels */
    /* - 7SegmentRowLayer; each 7-segment is composed of fixed 220 x 320 pixels */
    /* - LedGridLayer; a LED is considered as a pixel */  
    /* shape -- can be "flat", "round", "raised" or "sunken" */  
    /* extraSize just added to size; however if shape is "round", it affect the "roundness" */
    void border(float size, const String& color, const String& shape = "flat", float extraSize = 0);
    void noBorder();
    /* size unit ... see border() */
    void padding(float size);
    /* size unit ... see border() */
    void padding(float left, float top, float right, float bottom);
    void noPadding();
    /* clear the layer */
    void clear();
    /* set layer background color with common "color name" */
    void backgroundColor(const String& color);
    /* set no layer background color */
    void noBackgroundColor();
    /* normally used for "feedback" -- flash the default way (layer + border) */
    void flash();
    /* normally used for "feedback" -- flash the area (x, y) where the layer is clicked */
    void flashArea(int x, int y);
    const String& getLayerId() { return layerId; }
    void writeComment(const String& comment);
    /* rely on getFeedback() being called */ 
    /* autoFeedbackMethod: */
    /* . "" -- no auto feedback */
    /* . "f" -- flash the default way (layer + border) */
    /* . "fl" -- flash the layer */
    /* . "fa" -- flash the area where the layer is clicked */
    /* . "fas" -- flash the area (as a spot) where the layer is clicked */
    /* . "fs" -- flash the spot where the layer is clicked (regardless of any area boundary) */
    void enableFeedback(const String& autoFeedbackMethod = "");
    /** disable "feedback" */
    void disableFeedback();
    /** get "feedback" ... NULL if no pending "feedback" */
    const DDFeedback* getFeedback();
    /* set explicit (and more responsive) "feedback" handler (and enable feedback) */
    /* autoFeedbackMethod ... see enableFeedback() */
    void setFeedbackHandler(DDFeedbackHandler handler, const String& autoFeedbackMethod = "");
  public:
    DDFeedbackManager* getFeedbackManager() { return pFeedbackManager; }
    DDFeedbackHandler getFeedbackHandler() { return feedbackHandler; }
  protected:
    DDLayer(int layerId);
  public:
    ~DDLayer();
  protected:
    String layerId;  
    DDFeedbackManager *pFeedbackManager;
    DDFeedbackHandler feedbackHandler;
};

enum MbArrow { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };
enum MbIcon { Heart, SmallHeart, Yes, No, Happy, Sad, Confused, Angry, Asleep, Surprised,
              Silly, Fabulous, Meh, TShirt, Rollerskate, Duck, House, Tortoise, Butterfly, StickFigure,
              Ghost, Sword, Giraffe, Skull, Umbrella, Snake, Rabbit, Cow, QuarterNote, EigthNote,
              Pitchfork, Target, Triangle, LeftTriangle, Chessboard, Diamond, SmallDiamond, Square, SmallSquare, Scissors };

class MbImage {
  public:
    MbImage(int imageId) {
      this->imageId = String(imageId);
    }
    inline const String& getImageId() { return this->imageId; }  
  private:
    String imageId;
};


class MbDDLayer: public DDLayer {
  public:
    MbDDLayer(int layerId): DDLayer(layerId) {
    }
    /* show Microbit icon */
    void showIcon(MbIcon icon);
    /* show Microbit arrow */
    void showArrow(MbArrow arrow);
    /* show number; scroll if more than a single digit; but you get to control timing by using delay() */
    void showNumber(int num);
    /* show string; scroll if more than a single character; but you get to control timing by using delay() */
    void showString(const String& str);
    /* turn on LED @ (x, y) */
    void plot(int x, int y);
    /* turn off LED @ (x, y) */
    void unplot(int x, int y);
    /* toggle LED @ (x, y) on / off */
    void toggle(int x, int y);
    /* turn LEDs on by "pattern" */
    /* - '.': off */
    /* - '#': on */
    /* - '|': delimit a row */
    /* - e.g. "#|.#|..#" -- 3 rows */
    void showLeds(const String& ledPattern);
    /* set layer LED color with common "color name" */
    void ledColor(const String& color);
    MbImage* createImage(const String& ledPattern);
    void releaseImage(MbImage *pImage);
    void showImage(MbImage *pImage, int xOff);
    void scrollImage(MbImage *pImage, int xOff, long interval);
};


class TurtleDDLayer: public DDLayer {
  public:
    TurtleDDLayer(int layerId): DDLayer(layerId) {
    }
    /* forward; with pen or not */
    void forward(int distance, bool withPen = true);
    /* backward; with pen or not */
    void backward(int distance, bool withPen = true);
    /* left turn */
    void leftTurn(int angle);
    /* right turn */
    void rightTurn(int angle);
    /* go home (0, 0); with pen or not */
    void home(bool withPen = true);
    /* go to (x, y); with pen or not */
    void goTo(int x, int y, bool withPen = true);
    /* go by (byX, byY); with pen or not */
    void goBy(int byX, int byY, bool withPen = true);
    /* set heading angle */
    void setHeading(int angle);
    /* pen up */
    void penUp();
    /* pen down */
    void penDown();
    /* set pen size */
    void penSize(int size);
    /* set pen color */
    void penColor(const String& color);
    /* set fill color */
    void fillColor(const String& color);
    /* set no fill color */
    void noFillColor();
    /* set pen filled or not; if filled, shape drawn will be filled */
    void penFilled(bool filled);
    /* set text size */
    void setTextSize(int size);
    /* set font */
    /* - fontName */
    /* - textSize: 0 means default */
    void setTextFont(const String& fontName, int textSize = 0);
    /* draw a dot */
    void dot(int size, const String& color);
    /* draw circle; centered or not */
    void circle(int radius, bool centered = false);
    /* draw oval; centered or not */
    void oval(int width, int height, bool centered = false);
    /* draw triangle (SAS) */
    void triangle(int side1, int angle, int side2);
    /* draw isosceles triangle; given size and angle */
    void isoscelesTriangle(int side, int angle);
    /* draw rectangle; centered or not */
    void rectangle(int width, int height, bool centered = false);
    /* draw polygon given side and vertex count */
    void polygon(int side, int vertexCount);
    /* draw polygon enclosed in an imaginary centered circle */
    /* - given circle radius and vertex count */
    /* - whether inside the imaginary circle or outside of it */ 
    void centeredPolygon(int radius, int vertexCount, bool inside = false);
    /* write text; draw means draw the text (honor heading) */
    void write(const String& text, bool draw = false);
};


class LedGridDDLayer: public DDLayer {
  public:
    LedGridDDLayer(int layerId): DDLayer(layerId) {
    }
    /* turn on LED @ (x, y) */
    void turnOn(int x = 0, int y = 0);
    /* turn off LED @ (x, y) */
    void turnOff(int x = 0, int y = 0);
    /* toggle LED @ (x, y) */
    void toggle(int x = 0, int y = 0);
    /* turn on LEDs to form a horizontal "bar" */
    void horizontalBar(int count, bool rightToLeft = false);
    /* turn on LEDs to form a vertical "bar" */ 
    void verticalBar(int count, bool bottomToTop = true);
    /* set LED on color */ 
    void onColor(const String& color);
    /* set LED off color */ 
    void offColor(const String& color);
    /* set no LED off color */ 
    void noOffColor();
};

class LcdDDLayer: public DDLayer {
  public:
    LcdDDLayer(int layerId): DDLayer(layerId) {
    }
    void print(const String& text);
    void home();
    void setCursor(int x, int y);
    void cursor();
    void noCursor();
    void autoscroll();
    void noAutoscroll();
    void display();
    void noDisplay();
    void scrollDisplayLeft();
    void scrollDisplayRight();
    /* write text as a line, with alignment 'L', 'C', or 'R' */
    void writeLine(const String& text, int y = 0, const String& align = "L");
    /* write text as a line, with align "centered" */
    void writeCenteredLine(const String& text, int y = 0);
    /* set pixel color */
    void pixelColor(const String &color);
    /* set "background" (off) pixel color */
    void bgPixelColor(const String &color);
    /* set no "background" (off) pixel color */
    void noBgPixelColor();
};

class GraphicalDDLayer: public DDLayer {
  public:
    GraphicalDDLayer(int layerId): DDLayer(layerId) {
    }
    /* set cursor */
    void setCursor(int x, int y);
    /* move cursor by ... */
    void moveCursorBy(int byX, int byY);
    /* set text color and text background color */
    /* . empty background color means no background color */
    void setTextColor(const String& color, const String& bgColor = "");
    /* set text size */
    void setTextSize(int size);
    /* set font */
    /* - fontName */
    /* - textSize: 0 means default */
    void setTextFont(const String& fontName, int textSize = 0);
    /* set whether "print" will auto wrap or not */
    void setTextWrap(bool wrapOn);
    /* fill screen with color */
    void fillScreen(const String& color);
    void print(const String& text);
    void println(const String& text = "");
    /* draw char */
    /* - empty color means text color */
    /* - empty background color means no background color */
    /* - size: 0 means default */
    void drawChar(int x, int y, char c, const String& color = "", const String& bgColor = "", int size = 0);
    /* draw string */
    /* - empty color means text color */
    /* . empty background color means no background color */
    /* - size: 0 means default */
    void drawStr(int x, int y, const String& string, const String& color = "", const String& bgColor = "", int size = 0);
    /* draw a pixel */
    /* - empty color means text color */
    void drawPixel(int x, int y, const String& color = "");
    /* draw a line */
    /* - empty color means text color */
    void drawLine(int x1, int y1, int x2, int y2, const String& color = "");
    /* - empty color means text color */
    void drawCircle(int x, int y, int r, const String& color = "", bool filled = false);
    /* - empty color means text color */
    inline void fillCircle(int x, int y, int r, const String& color = "") {
      drawCircle(x, y, r, color, true);
    }
    /* - empty color means text color */
    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color = "", bool filled = false);
    /* - empty color means text color */
    inline void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color = "") {
      drawTriangle(x1, y1, x2, y2, x3, y3, color, true);
    }
    /* - empty color means text color */
    void drawRect(int x, int y, int w, int h, const String& color = "", bool filled = false);
    /* - empty color means text color */
    inline void fillRect(int x, int y, int w, int h, const String& color = "") {
      drawRect(x, y, w, h, color, true);
    }
    /* - empty color means text color */
    void drawRoundRect(int x, int y, int w, int h, int r, const String& color = "", bool filled = false);
    /* - empty color means text color */
    inline void fillRoundRect(int x, int y, int w, int h, int r, const String& color = "") {
      drawRoundRect(x, y, w, h, r, color, true);
    }
    /* forward (relative to cursor) */
    void forward(int distance);
    /* left turn */
    void leftTurn(int angle);
    /* right turn */
    void rightTurn(int angle);
    /* set heading angle */
    void setHeading(int angle);
    /* set pen size */
    void penSize(int size);
    /* set pen color (i.e. text color) */
    void penColor(const String& color);
    /* set fill color (for shape) */
    void fillColor(const String& color);
    /* set no fill color (for shape) */
    void noFillColor();
    /* draw circle; centered or not */
    void circle(int radius, bool centered = false);
    /* draw oval; centered or not */
    void oval(int width, int height, bool centered = false);
    /* draw triangle (SAS) */
    void triangle(int side1, int angle, int side2);
    /* draw isosceles triangle; given size and angle */
    void isoscelesTriangle(int side, int angle);
    /* draw rectangle; centered or not */
    void rectangle(int width, int height, bool centered = false);
    /* draw polygon given side and vertex count */
    void polygon(int side, int vertexCount);
    /* draw polygon enclosed in an imaginary centered circle */
    /* - given circle radius and vertex count */
    /* - whether inside the imaginary circle or outside of it */ 
    void centeredPolygon(int radius, int vertexCount, bool inside = false);
    /* write text; will not auto wrap */
    /* - draw means draw the text (honor heading direction) */
    void write(const String& text, bool draw = false);
};


class SevenSegmentRowDDLayer: public DDLayer {
  public:
    SevenSegmentRowDDLayer(int layerId): DDLayer(layerId) {
    }
    /* set segment color */
    void segmentColor(const String& color);
    /* reset segment off color; note that this will clear all digits */
    void resetSegmentOffColor(const String& color);
    /* reset segment off color to no color; note that this will clear all digits */
    void resetSegmentOffNoColor();
    /* turn on one or more segments */
    /* - segments: each character represents a segment to turn on */
    /*   . 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' */
    void turnOn(const String& segments, int digitIdx = 0);
    /* turn off one or more segments */ 
    /* - segments: each character represents a segment to turn off */
    /*   . 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' */
    void turnOff(const String& segments, int digitIdx = 0);
    /* like turnOn(), exception that the digit will be cleared first */
    /* - empty segments basically means turn all segments of the digit off */
    void setOn(const String& segments = "", int digitIdx = 0);
    /* show number */
    void showNumber(float number, const String& padding = " ");
    /* show HEX number */
    void showHexNumber(int number);
    /* show formatted number (even number with hex digits) */
    /* e.g. "12.00", "00.34", "-.12", "0ff" */
    void showFormatted(const String& formatted);
};

class PlotterDDLayer: public DDLayer {
  public:
    PlotterDDLayer(int layerId): DDLayer(layerId) {
    }
    void set(const String& key, float value);  
    void set2(const String& key1, float value1, const String& key2, float value2);  
    void set3(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3);  
};



class DDTunnel: public DDObject {
  public:
    DDTunnel(const String& type, int tunnelId, const String& endPoint, bool connectNow, int bufferSize);
    virtual ~DDTunnel();
    virtual void release();
    virtual void reconnect();
    const String& getTunnelId() { return tunnelId; }
  protected:
    //int _count();
    virtual bool _eof();
    //void _readLine(String &buffer);
    void _writeLine(const String& data);
  public:
    virtual void handleInput(const String& data, bool final);
  protected:
    String type;
    String tunnelId;
    String endPoint;
    long connectMillis;
    // int arraySize;
    // String* dataArray;
    // int nextArrayIdx;
    // int validArrayIdx;
  private:
    bool done;
};


class DDBufferedTunnel: public DDTunnel {
  public:
    DDBufferedTunnel(const String& type, int tunnelId, const String& endPoint, bool connectNow, int bufferSize);
    virtual ~DDBufferedTunnel();
    virtual void release();
    virtual void reconnect();
    //const String& getTunnelId() { return tunnelId; }
  protected:
    int _count();
    virtual bool _eof();
    bool _readLine(String &buffer);
    //void _writeLine(const String& data);
  public:
    virtual void handleInput(const String& data, bool final);
  private:
    // String endPoint;
    // String tunnelId;
    int arraySize;
    String* dataArray;
    int nextArrayIdx;
    int validArrayIdx;
    //bool done;
};


/**
 * support basic "text based line oriented" socket communication ... e.g.
 * pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17")
 */ 
class BasicDDTunnel: public DDBufferedTunnel {
  public:
    BasicDDTunnel(const String& type, int tunnelId, const String& endPoint, bool connectNow, int bufferSize): DDBufferedTunnel(type, tunnelId, endPoint, connectNow, bufferSize) {
    }
    /* count buffer ready to be read */
    inline int count() { return _count(); }
    /* reached EOF? */
    inline bool eof() { return _eof(); }
    /* read a line from buffer */
    String readLine();
    /* read a line from buffer, in to the buffer passed in */
    inline void readLine(String &buffer) { _readLine(buffer); }
    /* write a line */
    inline void writeLine(const String& data) { _writeLine(data); }
};

/**
 * support simple REST GET api .. e.g.
 * pTunnel = dumbdisplay.createJsonTunnel("http://worldtimeapi.org/api/timezone/Asia/Hong_Kong") 
 * . read() will read JSON one piece at a time ... e.g.
 *   { 
 *     "full_name": "Bruce Lee",
 *     "name":
 *     {
 *       "first": "Bruce",
 *       "last": "Lee"
 *     },
 *     "gender":"Male",
 *     "age":32
 *   }
 *   ==>
 *   `full_name` = `Bruce Lee`
 *   `name.first` = `Bruce`
 *   `name.last` = `Lee`
 *   `gender` = `Male`
 *   `age` = `32`
 */
class JsonDDTunnel: public DDBufferedTunnel {
  public:
    JsonDDTunnel(const String& type, int tunnelId, const String& endPoint, bool connectNow, int bufferSize): DDBufferedTunnel(type, tunnelId, endPoint, connectNow, bufferSize) {
    }
    /* count buffer ready (pieces of JSON) to be read */
    inline int count() { return _count(); }
    /* reached EOF? */
    inline bool eof() { return _eof(); }
    /* read a piece of JSON data */
    bool read(String& fieldId, String& fieldValue);
};

/** will not delete "tunnels" passed in */
class JsonDDTunnelMultiplexer {
  public:
    JsonDDTunnelMultiplexer(JsonDDTunnel** tunnels, int tunnelCount);
    ~JsonDDTunnelMultiplexer();
    int count();
    bool eof();
    /** return the index of the tunnel the field read from; -1 if non ready to read */
    int read(String& fieldId, String& fieldValue);
    void release();
    void reconnect();
  private:
     int tunnelCount;
     JsonDDTunnel** tunnels;
};

class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO) {
#ifndef DD_NO_SERIAL      
      if (pIO->isSerial() || pIO->isBackupBySerial()) {
        _The_DD_Serial = new DDSerial();
      }
#endif      
      initialize(pIO);
    }
    //DumbDisplay(DDInputOutput* pIO, DDSerialProxy* pDDSerialProxy);
    /* explicitly make connection -- blocking */
    /* - implicitly called when configure or create a layer */
    void connect();
    /** note that when reconnect, the connect version will be bumped up */
    int getConnectVersion();
    /* configure "pin frame" to be x-units by y-units (default 100x100) */
    void configPinFrame(int xUnitCount = 100, int yUnitCount = 100);
    /* configure "auto pinning of layers" with the layer spec provided */
    /* - horizontal: H(*) */
    /* - vertical: V(*) */
    /* - or nested, like H(0+V(1+2)+3)*/
    /* - where 0/1/2/3 are the layer ids  */
    /* - consider using the macros DD_AP_XXX */
    void configAutoPin(const String& layoutSpec);
    /* create a Microbit layer; 1st time will block waiting for connection */
    MbDDLayer* createMicrobitLayer(int width = 5, int height = 5);
    /* create a Turtle layer; 1st time will block waiting for connection */
    TurtleDDLayer* createTurtleLayer(int width, int height);
    /* create a LED-grid layer; given col count and row count */
    /* - a LED can be formed by sub-LED-grid; given sub-col count and sub-row count */
    LedGridDDLayer* createLedGridLayer(int colCount = 1, int rowCount = 1, int subColCount = 1, int subRowCount = 1);
    /* create a LCD layer */
    LcdDDLayer* createLcdLayer(int colCount = 16, int rowCount = 2, int charHeight = 0, const String& fontName = "");
    /* create a graphical [LCD] layer */
    GraphicalDDLayer* createGraphicalLayer(int width, int height);
    SevenSegmentRowDDLayer* create7SegmentRowLayer(int digitCount = 1);
    PlotterDDLayer* createPlotterLayer(int width, int height, int pixelsPerSecond = 10);
    /* create a 'tunnel' to interface with Internet (similar to socket) */
    /* note the 'tunnel' is ONLY supported with DumbDisplayWifiBridge -- https://www.youtube.com/watch?v=0UhRmXXBQi8 */
    /* MUST delete the 'tunnel' after use, by calling deleteTunnel()  */
    /* if not connect now, need to connect via reconnect() */
    BasicDDTunnel* createBasicTunnel(const String& endPoint, bool connectNow = true, int bufferSize = 4);
    /* if not connect now, need to connect via reconnect() */
    JsonDDTunnel* createJsonTunnel(const String& endPoint, bool connectNow = true, int bufferSize = 4);
    //void reconnectTunnel(DDTunnel *pTunnel, const String& endPoint);
    void deleteTunnel(DDTunnel *pTunnel);
    /* set DD background color with common "color name" */
    void backgroundColor(const String& color);
    /* basically, functions the same as recordLayerCommands() */
    void recordLayerSetupCommands();
    /* basically, functions the same as playbackLayerCommands() */
    /* additionally: */
    /* 1. save and persiste the layer commands */
    /* 2. enable DumbDisplay reconnect feature -- */ 
    /*    tells the layer setup commands to use when DumbDisplay reconnects */ 
    void playbackLayerSetupCommands(const String persist_id);
    /* start recording layer commands (of any layers) */
    /* and sort of freeze the display, until playback */
    void recordLayerCommands();
    /* playback recorded commands (unfreeze the display) */
    void playbackLayerCommands();
    /* stop recording commands (and forget what have recorded) */
    void stopRecordLayerCommands();
    /* save the recorded commands (and continue recording) */
    /* - id: identifier of the recorded commands, overwriting and previous one; */
    /*       if not recording, will delete previous recorded commands */
    /* - persist: store it to your phone or not */
    void saveLayerCommands(const String& id, bool persist = false);
    /* load saved commands (as if recording those commands) */
    /* - recording started or not, will add the commands to the buffer */
    /* - afterward, will keep recording */
    /* - use playbackLayerCommands() to playback loaded commands */
    /* - if not recording commands, this basically remove saved commands */
    void loadLayerCommands(const String& id);
    /* write out a comment to DD */
    void writeComment(const String& comment);
    /* pin a layer @ some position of an imaginary grid of units */
    /* - the imaginary grid size can be configured when calling connect() -- default is 100x100 */  
    /* - align (e.g. "LB"): left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align */
    void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    void deleteLayer(DDLayer *pLayer);
    void debugSetup(int debugLedPin, bool enableEchoFeedback = false);
    /* log line to serial making sure not affecting DD */
    void logToSerial(const String& logLine) {
      if (canLogToSerial()) {
        //Serial.println(logLine);
        if (_The_DD_Serial != NULL) {
          _The_DD_Serial->print(logLine);
          _The_DD_Serial->print("\n");
        }
      } else {
        writeComment(logLine);
      }
    }
  private:
    void initialize(DDInputOutput* pIO);
    bool canLogToSerial();
};


class DDConnectVersionTracker {
  public:
    DDConnectVersionTracker() {
      this->version = 0;
    }
    bool checkChanged(DumbDisplay& dumbdisplay) {
      int oldVersion = this->version;
      this->version = dumbdisplay.getConnectVersion();
      return this->version != oldVersion;
    }
  private:
    int version;  
};


// /* log line to serial making sure not affect DD */
// void DDLogToSerial(const String& logLine);
/* the same usage as standard delay(), but it allows DD chances to handle feedback */
void DDDelay(unsigned long ms);
/* give DD a chance to handle feedback */
void DDYield();


#endif

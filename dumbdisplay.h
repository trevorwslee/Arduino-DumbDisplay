#ifndef dumbdisplay_h
#define dumbdisplay_h


#ifdef DD_4_ESP32
#include <esp_spp_api.h>
#include "HardwareSerial.h"
#endif


#define SUPPORT_TUNNEL


#define DUMBDISPLAY_BAUD 115200
#define DD_SERIAL_BAUD DUMBDISPLAY_BAUD
#define DD_WIFI_PORT 10201


#define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
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


class DDInputOutput {
  public:
    /* Serial IO mechanism (i.e. connecting via USB) */ 
    DDInputOutput(unsigned long serialBaud = DD_SERIAL_BAUD): DDInputOutput(serialBaud, false, true) {
    }
    bool isSerial() {
      return !backupBySerial && setupForSerial;
    }
    bool isBackupBySerial() {
      return backupBySerial;
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
    virtual void keepAlive() {
    }
    virtual void validConnection() {
    }
    virtual void preConnect() {
      if (setupForSerial)
        Serial.begin(serialBaud);
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
};


class DDFeedbackManager {
  public: 
    DDFeedbackManager(int bufferSize);
    ~DDFeedbackManager();
    const DDFeedback* getFeedback();
    void pushFeedback(int x, int y);
  private:
    DDFeedback* feedbackArray;
    int arraySize;
    int nextArrayIdx;
    int validArrayIdx;
};


class DDLayer;


enum DDFeedbackType { CLICK };

/* pLayer -- pointer to the DDLayer of which "feedback" received */
/* type -- currently, only possible value if CLICK */
/* x, y -- (x, y) is the "area" on the layer where was clicked */
typedef void (*DDFeedbackHandler)(DDLayer* pLayer, DDFeedbackType type, int x, int y);

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
    void border(float size, const String& color, const String& shape = "flat");
    void noBorder();
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
    /* . empty background color means no background color */
    /* - size: 0 means default */
    void drawChar(int x, int y, char c, const String& color, const String& bgColor = "", int size = 0);
    /* draw a pixel */
    void drawPixel(int x, int y, const String& color);
    /* draw a [end to end] line */
    void drawLine(int x1, int y1, int x2, int y2, const String& color);
    void drawCircle(int x, int y, int r, const String& color);
    void fillCircle(int x, int y, int r, const String& color);
    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color);
    void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color);
    void drawRect(int x, int y, int w, int h, const String& color);
    void fillRect(int x, int y, int w, int h, const String& color);
    void drawRoundRect(int x, int y, int w, int h, int r, const String& color);
    void fillRoundRect(int x, int y, int w, int h, int r, const String& color);
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
    /* turn on one or more segments */
    /* - segments: each character represents a segment to turn on */
    /*   . 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' */
    void turnOn(const String& segments, int digitIdx = 0);
    /* turn off one or more segments */ 
    /* - segments: each character represents a segment to turn off */
    /*   . 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' */
    void turnOff(const String& segments, int digitIdx = 0);
    /* show number */
    void showNumber(float number);
    /* show HEX number */
    void showHexNumber(int number);
    /* show formatted number (even number with hex digits) */
    /* e.g. "12.00", "00.34", "-.12", "0ff" */
    void showFormatted(const String& formatted);
};


#ifdef SUPPORT_TUNNEL

class DDTunnel: public DDObject {
  public:
    DDTunnel(int tunnelId, int bufferSize = 4);
    ~DDTunnel();
    void close();
    const String& getTunnelId() { return tunnelId; }
    int count();
    bool eof();
    const String& readLine();
    void writeLine(const String& data);
  public:
    void handleInput(const String& data, bool final);
  protected:
    String tunnelId;
    int arraySize;
    String* dataArray;
    int nextArrayIdx;
    int validArrayIdx;
    bool done;
};

class BasicDDTunnel: public DDTunnel {
  public:
    BasicDDTunnel(int tunnelId/*, const String& endPoint*/): DDTunnel(tunnelId/*, endPoint*/) {
    }
    // int count() { return arraySize; }
    // String readLine();
    // void writeLine(const String& lineData);
    // bool eof();
};

#endif


class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO);
    /* explicitly make connection -- blocking */
    /* - implicitly called when configure or create a layer */
    void connect();
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
#ifdef SUPPORT_TUNNEL
    BasicDDTunnel* createBasicTunnel(const String& endPoint);
#endif
    /* set DD background color with common "color name" */
    void backgroundColor(const String& color);
    /* write out a comment to DD */
    void writeComment(const String& comment);
    /* pin a layer @ some position of an imaginary grid of units */
    /* - the imaginary grid size can be configured when calling connect() -- default is 100x100 */  
    /* - align (e.g. "LB"): left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align */
    void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    void deleteLayer(DDLayer *pLayer);
    void debugSetup(int debugLedPin, bool enableEchoFeedback = false);
};

/* log line to serial making sure not affect DD */
void DDLogToSerial(const String& logLine);
/* the same usage as standard delay(), but it allows DD chances to handle feedback */
void DDDelay(unsigned long ms);
/* give DD a chance to handle feedback */
void DDYield();


#endif

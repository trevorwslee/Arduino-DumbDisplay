#ifndef dumbdisplay_h
#define dumbdisplay_h

#define DUMBDISPLAY_BAUD 115200

#define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
#define DD_HEX_COLOR(color) ("#" + String(color, 16))

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


class DDInputOutput {
  public:
    /* Serial IO mechanism (i.e. connecting via USB) */ 
    DDInputOutput(): DDInputOutput(false, true) {
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
    virtual void print(const char *p) {
        Serial.print(p);
    }
    virtual void flush() {
      Serial.flush();
    }
    virtual void preConnect() {
      if (setupForSerial)
        Serial.begin(DUMBDISPLAY_BAUD);
    }
  protected:
    DDInputOutput(bool backupBySerial, bool setupForSerial) {
      this->backupBySerial = backupBySerial;
      this->setupForSerial = setupForSerial;
    }
  protected:
    bool backupBySerial;
    bool setupForSerial;
};

 

class DDLayer {
  public:
    /* set layer visibility */
    void visibility(bool visible);
    /* set layer opacity */
    /* - 0 to 255 */
    void opacity(int opacity);
    void padding(int left, int top, int right, int bottom);
    void noPadding();
    /* clear the layer */
    void clear();
    /* set layer background color with common "color name" */
    void backgroundColor(const String& color);
    /* set no layer background color */
    void noBackgroundColor();
    const String& getLayerId() { return layerId; }
  protected:
    DDLayer(int layerId);
  protected:
    String layerId;  
};


enum MbArrow { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };
enum MbIcon { Heart, SmallHeart, Yes, No, Happy, Sad, Confused, Angry, Asleep, Surprised,
              Silly, Fabulous, Meh, TShirt, Rollerskate, Duck, House, Tortoise, Butterfly, StickFigure,
              Ghost, Sword, Giraffe, Skull, Umbrella, Snake, Rabbit, Cow, QuarterNote, EigthNote,
              Pitchfork, Target, Triangle, LeftTriangle, Chessboard, Diamond, SmallDiamond, Square, SmallSquare, Scissors,
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
    /* - whether inside the circle or not */ 
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
    /* set pixel color */
    void pixelColor(const String &color);
    /* set "background" (off) pixel color */
    void bgPixelColor(const String &color);
    /* set no "background" (off) pixel color */
    void noBgPixelColor();
};



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
    /* pin a layer @ some position of an imaginary grid of units */
    /* - the imaginary grid size can be configured when calling connect() -- default is 100x100 */  
    /* - align (e.g. "LB"): left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align */
    void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    void deleteLayer(DDLayer *pLayer);
};


#endif

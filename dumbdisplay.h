#ifndef dumbdisplay_h
#define dumbdisplay_h

#define DUMBDISPLAY_BAUD 115200
#define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
#define DD_HEX_COLOR(color) ("#" + String(color, 16))

class DDInputOutput {
  public:
    /* Serial IO mechanism */ 
    DDInputOutput(): DDInputOutput(true) {
    }
    inline bool allowSerial() {
      return enableSerial;
    }
    virtual bool available() {
      return enableSerial && Serial.available();
    }
    virtual char read() {
      return enableSerial ? Serial.read() : 0;
    }
    virtual void print(const char *p) {
      if (enableSerial)
        Serial.print(p);
    }
  protected:
    DDInputOutput(bool enableSerial) {
      this->enableSerial = enableSerial;
      if (enableSerial)
        Serial.begin(DUMBDISPLAY_BAUD);
    }
  protected:
    bool enableSerial;
};


class DDLayer {
  public:
    /* set layer visibility */
    void visibility(bool visible);
    /* set layer opacity */
    /* - 0 to 255 */
    void opacity(int opacity);
    /* clear the layer */
    void clear();
    // /* set layer background color with RGB */
    // void backgroundColor(long color);
    /* set layer background color with common "color name" */
    void backgroundColor(const String& color);
    /* set no layer background color */
    void noBackgroundColor();
    String& getLayerId() { return layerId; }
  protected:
    DDLayer(int layerId);
    // DDLayer(int layerId) {
    //   this->layerId = String(layerId);
    // }
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
    // /* clear layer screen */
    // void clearScreen();
    // /* set layer LED color with RGB */
    // void ledColor(long color);
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
    // /* set pen color */
    // void penColor(long color);
    /* set pen color */
    void penColor(const String& color);
    // /* set fill color */
    // void fillColor(long color);
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
    void verticalBar(int count, bool bottomToTop = false);
    // /* set LED on color */ 
    // void onColor(long color);
    /* set LED on color */ 
    void onColor(const String& color);
    // /* set LED off color */ 
    // void offColor(long color);
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
    /* write text as a line, width alignment 'L', 'C', or 'R' */
    void writeLine(const String& text, int col = 1, const String& align = "L");
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
    /* create a Microbit layer; 1st time will block waiting for connection */
    MbDDLayer* createMicrobitLayer(int width = 5, int height = 5);
    /* create a Turtle layer; 1st time will block waiting for connection */
    TurtleDDLayer* createTurtleLayer(int width, int height);
    /* create a LED-grid layer; given col count and row count */
    /* - a LED can be formed by sub-LED-grid; given sub-col count and sub-row count */
    LedGridDDLayer* createLedGridLayer(int colCount = 1, int rowCount = 1, int subColCount = 1, int subRowCount = 1);
    LcdDDLayer* createLcdLayer(int colCount, int rowCount, int charHeight = 9, const String& fontName = "");
    void deleteLayer(DDLayer *pLayer);
};


#endif

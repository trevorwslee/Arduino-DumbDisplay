#ifndef dumbdisplay_h
#define dumbdisplay_h

#define DUMBDISPLAY_BAUD 115200

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
    /* set layer background color with RGB */
    void backgroundColor(long color);
    /* set layer background color with common "color name" */
    void backgroundColor(const String& color);
    /* set no layer background color */
    void noBackgroundColor();
  protected:
    DDLayer(int layerId);
    // DDLayer(int layerId) {
    //   this->layerId = String(layerId);
    // }
  protected:
    String layerId;  
};


enum MBArrow { North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };
enum MBIcon { Heart, SmallHeart, Yes, No, Happy, Sad, Confused, Angry, Asleep, Surprised,
              Silly, Fabulous, Meh, TShirt, Rollerskate, Duck, House, Tortoise, Butterfly, StickFigure,
              Ghost, Sword, Giraffe, Skull, Umbrella, Snake, Rabbit, Cow, QuarterNote, EigthNote,
              Pitchfork, Target, Triangle, LeftTriangle, Chessboard, Diamond, SmallDiamond, Square, SmallSquare, Scissors,
        };

class MicroBitLayer: public DDLayer {
  public:
    MicroBitLayer(int layerId): DDLayer(layerId) {
    }
    /* show Microbit icon */
    void showIcon(MBIcon icon);
    /* show Microbit arrow */
    void showArrow(MBArrow arrow);
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
    void ledColor(long color);
    /* set layer LED color with common "color name" */
    void ledColor(const String& color);
};


class TurtleLayer: public DDLayer {
  public:
    TurtleLayer(int layerId): DDLayer(layerId) {
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
    void penColor(long color);
    /* set pen color */
    void penColor(const String& color);
    /* set fill color */
    void fillColor(long color);
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
    /* draw rectangle; centered or not */
    void rectangle(int width, int height, bool centered = false);
    /* draw polygon given side and vertex count */
    void polygon(int side, int vertexCount);
    /* draw polygon "enclosed" in a virtual centered cirtle; given circle radius and vertex count */
    void enclosedPolygon(int radius, int vertexCount);
    /* write text; draw means draw the text (honor heading) */
    void write(const String& text, bool draw = false);
};

class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO);
    /* create a Microbit layer; 1st time will block waiting for connection */
    MicroBitLayer* createMicroBitLayer(int width, int height);
    /* create a Turtle layer; 1st time will block waiting for connection */
    TurtleLayer* createTurtleLayer(int width, int height);
};



#endif

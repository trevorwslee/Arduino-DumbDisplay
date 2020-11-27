#ifndef dumbdisplay_h
#define dumbdisplay_h

#define DUMBDISPLAY_BAUD 115200

class DDInputOutput {
  public:
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
    void visibility(bool visible);
    /* opacity: 0 - 255 */
    void opacity(int opacity);
    void backgroundColor(long color);
    void backgroundColor(const String& color);
    void noBackgroundColor();
  protected:
    DDLayer(const String& layerId) {
      this->layerId = String(layerId);
    }
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
    MicroBitLayer(const String& layerId, int width, int height): DDLayer(layerId) {
      this->width = width;
      this->height = height;
    }
    void showIcon(MBIcon icon);
    void showArrow(MBArrow arrow);
    void showNumber(int num);
    void showString(const String& str);
    void plot(int x, int y);
    void unplot(int x, int y);
    void toggle(int x, int y);
    void showLeds(const String& ledPattern);
    void clearScreen();
    void ledColor(long color);
    void ledColor(const String& color);
  private:
    int width;
    int height;  
};

class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO);
    void connect();
    MicroBitLayer* createMicroBitLayer(int width, int height);
};



#endif

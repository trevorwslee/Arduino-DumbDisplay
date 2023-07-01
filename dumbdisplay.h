/*

============================

if want to disable int parameter encoding, define DD_DISABLE_PARAM_ENCODEING before includeing dumbdisplay.h, like

#define DD_DISABLE_PARAM_ENCODEING

=============================

*/



#ifndef dumbdisplay_h
#define dumbdisplay_h


// not used???
// #ifdef DD_4_ESP32
// #include <esp_spp_api.h>
// #include "HardwareSerial.h"
// #endif


#define DUMBDISPLAY_BAUD  115200
#define DD_SERIAL_BAUD    DUMBDISPLAY_BAUD
#define DD_WIFI_PORT      10201


#define DD_DEF_SEND_BUFFER_SIZE 128


#include "_dd_util.h"



#define DD_CONDENSE_COMMAND


#define DD_HEX_COLOR(color) ("#" + String(color, 16))


#ifdef DD_CONDENSE_COMMAND
  #define DD_INT_COLOR(color) ("+" + DDIntEncoder(color).encoded())
  #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) ||  defined(DD_DISABLE_PARAM_ENCODEING)
    #define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
  #else
    #define DD_RGB_COLOR(r, g, b) ("+" + DDIntEncoder(0xffffff & ((((((int32_t) (r)) << 8) + ((int32_t) (g))) << 8) + ((int32_t) (b)))).encoded())
  #endif
  // #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
  //   #define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
  //   //#define DD_RGB_COLOR(r, g, b) (String(r<0?0:(r>255?255:r)) + "-" + String(g<0?0:(g>255?255:g)) + "-" + String(b<0?0:(b>255?255:b)))
  // #else
  //   #if defined(DD_DISABLE_PARAM_ENCODEING)
  //     #define DD_RGB_COLOR(r, g, b) ("#" + String(0xffffff & ((((((int32_t) (r)) << 8) + ((int32_t) (g))) << 8) + ((int32_t) (b))), 16))
  //   #else  
  //     #define DD_RGB_COLOR(r, g, b) ("+" + DDIntEncoder(0xffffff & ((((((int32_t) (r)) << 8) + ((int32_t) (g))) << 8) + ((int32_t) (b)))).encoded())
  //   #endif
  // #endif
#else
  #define DD_RGB_COLOR(r, g, b) (String(r<0?0:(r>255?255:r)) + "-" + String(g<0?0:(g>255?255:g)) + "-" + String(b<0?0:(b>255?255:b)))
  #define DD_INT_COLOR(color) ("+" + String(color))
#endif



#define DD_AP_HORI "H(*)"
#define DD_AP_VERT "V(*)"
#define DD_AP_STACK "S(*)"
#define DD_AP_HORI_2(id1, id2) ("H(" + id1 + "+" + id2 + ")")
#define DD_AP_VERT_2(id1, id2) ("V(" + id1 + "+" + id2 + ")")
#define DD_AP_STACK_2(id1, id2) ("S(" + id1 + "+" + id2 + ")")
#define DD_AP_HORI_3(id1, id2, id3) ("H(" + id1 + "+" + id2 + "+" + id3 + ")")
#define DD_AP_VERT_3(id1, id2, id3) ("V(" + id1 + "+" + id2 + "+" + id3 + ")")
#define DD_AP_STACK_3(id1, id2, id3) ("S(" + id1 + "+" + id2 + "+" + id3 + ")")
#define DD_AP_HORI_4(id1, id2, id3, id4) ("H(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + ")")
#define DD_AP_VERT_4(id1, id2, id3, id4) ("V(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + ")")
#define DD_AP_STACK_4(id1, id2, id3, id4) ("S(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + ")")
#define DD_AP_HORI_5(id1, id2, id3, id4, id5) ("H(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")")
#define DD_AP_VERT_5(id1, id2, id3, id4, id5) ("V(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")")
#define DD_AP_STACK_5(id1, id2, id3, id4, id5) ("S(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")")
#define DD_AP_HORI_6(id1, id2, id3, id4, id5, id6) ("H(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")" + "+" + id6 + ")")
#define DD_AP_VERT_6(id1, id2, id3, id4, id5, id6) ("V(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")" + "+" + id6 + ")")
#define DD_AP_STACK_6(id1, id2, id3, id4, id5, id6) ("S(" + id1 + "+" + id2 + "+" + id3 + "+" + id4 + "+" + id5 + ")" + "+" + id6 + ")")

#define DD_AP_PADDING(lp, tp, rp, bp, id) ("S/" + String(lp) + "-" + String(tp) + "-" + String(rp) + "-" + String(bp) + "(" + id + ")") 

#define DD_AP_SPACER(w, h) (String("<") + String(w) + "x" + String(h) + String(">")) 

#define DD_PROGRAM_SPACE_COMPRESS_BA_0 '!'
#define DD_COMPRESS_BA_0 '0'

#define DD_TUNNEL_DEF_BUFFER_SIZE 3


#include "_dd_serial.h"
#include "_dd_io.h"
#include "_dd_feedback.h"
#include "_dd_colors.h"


class DDLayer;

/// @struct DDFeedbackHandler
/// @brief
/// Type signature for "feedback" handler. See DDLayer::setFeedbackHandler()
/// @param pLayer pointer to the DDLayer from which "feedback" is for 
/// @param type type of the "feedback" 
/// @param feedback data concerning the "feedback"
typedef void (*DDFeedbackHandler)(DDLayer* pLayer, DDFeedbackType type, const DDFeedback& feedback);


const int8_t DD_OBJECT_TYPE_LAYER  = 0;
const int8_t DD_OBJECT_TYPE_TUNNEL = 1;


/// Base class for DD objects.
struct DDObject {
    /// object type -- DD_OBJECT_TYPE_LAYER or DD_OBJECT_TYPE_TUNNEL
    int8_t objectType;
    /// custom data
    String customData;
public:
   // since 20230601
    virtual ~DDObject() {}
};


/// Base class for the different layers support by DumbDisplay; created with various layer creation methods of DumbDispaly, DumbDisplay::createLedGridLayer()
class DDLayer: public DDObject {
  public:
    /// set border for each size
    /// @param size size unit is pixel:
    ///             - LcdLayer -- each character is composed of pixels
    ///             - 7SegmentRowLayer -- each 7-segment is composed of fixed 220 x 320 pixels
    ///             - LedGridLayer -- a LED is considered as a pixel  
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    /// @param shape can be "flat", "hair", "round", "raised" or "sunken"  
    /// @param extraSize simply add to size; however if shape is "round", it affects the "roundness"
    void border(float size, const String& color, const String& shape = "flat", float extraSize = 0);
    /// set no border
    void noBorder();
    /// set padding for all sides
    /// @param size for size unit, see border()
    void padding(float size);
    /// set padding for each side;
    /// for unit, see border()
    void padding(float left, float top, float right, float bottom);
    /// set no padding
    void noPadding();
    /// set margin for all sides
    /// @param size for size unit, see border()
    void margin(float size);
    /// set margin for each side;
    /// for unit, see border()
    void margin(float left, float top, float right, float bottom);
    /// set no margin
    void noMargin();
    /// clear the layer
    void clear();
    /// set layer background color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void backgroundColor(const String& color);
    /// set no layer background color
    void noBackgroundColor();
    /// set whether layer visible (not visible means hidden)
    void visible(bool visible);
    /// set whether layer transparent
    void transparent(bool transparent);
    /// set layer disabled or not; if disabled, layer will not have "feedback", and its appearance will be like disabled 
    void disabled(bool disabled);
    /// set layer opacity percentage
    /// @param opacity 0 - 100
    void opacity(int opacity);
    /// set layer's alpha channel
    /// @param alpha 0 - 255
    void alpha(int alpha);
    /// normally used for "feedback" -- flash the default way (layer + border)
    void flash();
    /// normally used for "feedback" -- flash the area (x, y) where the layer is clicked
    void flashArea(int x, int y);
    const String& getLayerId() const { return layerId; }
    /// set explicit (and more responsive) "feedback" handler (and enable feedback)
    /// autoFeedbackMethod:
    /// - "" -- no auto feedback
    /// - "f" -- flash the default way (layer + border)
    /// - "fl" -- flash the layer
    /// - "fa" -- flash the area where the layer is clicked
    /// - "fas" -- flash the area (as a spot) where the layer is clicked
    /// - "fs" -- flash the spot where the layer is clicked (regardless of any area boundary)
    /// @param handler "feedback" handler; see DDFeedbackHandler
    void setFeedbackHandler(DDFeedbackHandler handler, const String& autoFeedbackMethod = "");
    /// rely on getFeedback() being called
    /// autoFeedbackMethod:
    /// - "" -- no auto feedback (the default)
    /// - "f" -- flash the standard way (layer + border)
    /// - "fl" -- flash the layer
    /// - "fa" -- flash the area where the layer is clicked
    /// - "fas" -- flash the area (as a spot) where the layer is clicked
    /// - "fs" -- flash the spot where the layer is clicked (regardless of any area boundary)
    void enableFeedback(const String& autoFeedbackMethod = "");
    /// disable "feedback"
    void disableFeedback();
    /// get "feedback" DDFeedback
    /// @return NULL if no pending "feedback"
    const DDFeedback* getFeedback();
    void debugOnly(int i);
  public:
    /// @attention used internally
    DDFeedbackManager* getFeedbackManager() const { return pFeedbackManager; }
    /// @attention used internally
    DDFeedbackHandler getFeedbackHandler() const { return feedbackHandler; }
  protected:
    DDLayer(int8_t layerId);
  public:
    // made virtual since 20230601
    virtual ~DDLayer();
  protected:
    void _enableFeedback();
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
    MbImage(int8_t imageId) {
      this->imageId = String(imageId);
    }
    inline const String& getImageId() const { return this->imageId; }  
  private:
    String imageId;
};


/// Class for Microbit-like DD layer; created with DumbDisplay::createMicrobitLayer()
class MbDDLayer: public DDLayer {
  public:
    /// for internal use only
    MbDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// show Microbit icon
    void showIcon(MbIcon icon);
    /// show Microbit arrow
    void showArrow(MbArrow arrow);
    /// show number; scroll if more than a single digit; but you get to control timing by using delay()
    void showNumber(int num);
    /// show string; scroll if more than a single character; but you get to control timing by using delay()
    void showString(const String& str);
    /// turn on LED @ (x, y)
    void plot(int x, int y);
    /// turn off LED @ (x, y)
    void unplot(int x, int y);
    /// toggle LED @ (x, y) on / off
    void toggle(int x, int y);
    /// turn LEDs on by "pattern"
    /// - '.': off
    /// - '#': on
    /// - '|': delimit a row
    /// - e.g. "#|.#|..#" -- 3 rows
    void showLeds(const String& ledPattern);
    /// set LED color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void ledColor(const String& color);
    MbImage* createImage(const String& ledPattern);
    void releaseImage(MbImage *pImage);
    void showImage(MbImage *pImage, int xOff);
    void scrollImage(MbImage *pImage, int xOff, long interval);
};


/// Class for Turtle-like DD layer; created with DumbDisplay::createTurtleLayer()
class TurtleDDLayer: public DDLayer {
  public:
    /// for internal use only
    TurtleDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// forward; with pen or not
    void forward(int distance, bool withPen = true);
    /// backward; with pen or not
    void backward(int distance, bool withPen = true);
    /// left turn
    void leftTurn(int angle);
    /// right turn
    void rightTurn(int angle);
    /// go home (0, 0); with pen or not
    void home(bool withPen = true);
    /// go to (x, y); with pen or not
    void goTo(int x, int y, bool withPen = true);
    /// go by (byX, byY); with pen or not
    void goBy(int byX, int byY, bool withPen = true);
    /// set heading angle (degree)
    void setHeading(int angle);
    /// set pen size
    void penSize(int size);
    /// set pen color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void penColor(const String& color);
    /// set fill color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void fillColor(const String& color);
    /// set no fill color
    void noFillColor();
    /// set pen filled or not; if filled, shape drawn will be filled
    void penFilled(bool filled);
    /// set text size
    void setTextSize(int size);
    /// set font
    /// @param textSize: 0 means default
    void setTextFont(const String& fontName = "", int textSize = 0);
    /// pen up
    void penUp();
    /// pen down
    void penDown();
    void beginFill();
    void endFill();
    /// draw a dot
    void dot(int size, const String& color);
    /// draw circle; centered or not
    void circle(int radius, bool centered = false);
    /// draw oval; centered or not
    void oval(int width, int height, bool centered = false);
    /// draw arc; centered or not
    void arc(int width, int height, int startAngle, int sweepAngle, bool centered = false);
    /// draw triangle (SAS)
    void triangle(int side1, int angle, int side2);
    /// draw isosceles triangle; given size and angle
    void isoscelesTriangle(int side, int angle);
    /// draw rectangle; centered or not
    void rectangle(int width, int height, bool centered = false);
    /// draw polygon given side and vertex count
    void polygon(int side, int vertexCount);
    /// draw polygon enclosed in an imaginary centered circle
    /// - given circle radius and vertex count
    /// - whether inside the imaginary circle or outside of it
    void centeredPolygon(int radius, int vertexCount, bool inside = false);
    /// write text; draw means draw the text (honor heading)
    void write(const String& text, bool draw = false);
};


/// Class for LED grid layer; created with DumbDisplay::createLedGridLayer()
class LedGridDDLayer: public DDLayer {
  public:
    /// for internal use only
    LedGridDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// turn on LED @ (x, y)
    void turnOn(int x = 0, int y = 0);
    /// turn off LED @ (x, y)
    void turnOff(int x = 0, int y = 0);
    /// toggle LED @ (x, y)
    void toggle(int x = 0, int y = 0);
    /// turn on LED @ (x, y)
    /// @param onColor LED on color (common color name); empty string means what already set
    void turnOnEx(int x = 0, int y = 0, const String& onColor = "");
    /// turn on/off LEDs based on bits
    /// @param bits least significant bit maps to right-most LED
    /// @param y row
    void bitwise(unsigned long bits, int y = 0);
    /// turn on/off two rows of LEDs by bits
    /// @param y starting row
    void bitwise2(unsigned long bits_0, unsigned long bits_1, int y = 0);
    /// turn on/off three rows of LEDs by bits
    /// @param y starting row
    void bitwise3(unsigned long bits_0, unsigned long bits_1, unsigned long bits_2, int y = 0);
    /// turn on/off four rows of LEDs by bits
    /// @param y starting row
    void bitwise4(unsigned long bits_0, unsigned long bits_1, unsigned long bits_2, unsigned long bits_3, int y = 0);
    /// turn on LEDs to form a horizontal "bar"
    /// @param rightToLeft true means right to left
    void horizontalBar(int count, bool rightToLeft = false);
    /// turn on LEDs to form a vertical "bar" 
    /// @param bottomToTop true means bottom to top
    void verticalBar(int count, bool bottomToTop = true);
    /// turn on LEDs to form a horizontal "bar"
    /// @param count can be negative (if it makes sense)
    /// @param startX normally 0
    /// @param color LED on color; DD_RGB_COLOR(...) or common color name; empty string means what already set
    void horizontalBarEx(int count, int startX = 0, const String& color = "");
    /// turn on LEDs to form a vertical "bar"
    /// @param count can be negative (if it makes sense)
    /// @param startY normally 0
    /// @param color LED on color; DD_RGB_COLOR(...) or common color name; empty string means what already set
    void verticalBarEx(int count, int startY = 0, const String& color = "");
    /// set LED on color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void onColor(const String& color);
    /// set LED off color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void offColor(const String& color);
    /// set no LED off color 
    void noOffColor();
};

/// @brief
/// Class for LCD layer; created with DumbDisplay::createLcdLayer()
/// @note with "feedback" enabled, can be used as a button
/// @note with "feedback" enabled, can be used as checkbox; consider using these emojis for checkbox --
/// ☒☐✅❎🟩✔️ ☑️⬛✔✖
class LcdDDLayer: public DDLayer {
  public:
    /// for internal use only
    LcdDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// print text, moving cursor
    void print(const String& text);
    /// move cursor to home
    void home();
    /// set cursor position
    /// @param x horizontal position
    /// @param y vertical position
    void setCursor(int x, int y);
    /// show cursor
    void cursor();
    /// hide cursor
    void noCursor();
    /// enable auto-scroll
    void autoscroll();
    /// disable auto-scroll
    void noAutoscroll();
    void display();
    void noDisplay();
    void scrollDisplayLeft();
    void scrollDisplayRight();
    /// write text as a line
    /// @param align 'L', 'C', or 'R'
    void writeLine(const String& text, int y = 0, const String& align = "L");
    /// write text as a line, with align "centered"
    void writeCenteredLine(const String& text, int y = 0);
    /// set pixel color
    /// @param color DD_RGB_COLOR(...) or common color name
    void pixelColor(const String &color);
    /// set "background" (off) pixel color
    /// @param color DD_RGB_COLOR(...) or common color name
    void bgPixelColor(const String &color);
    /// set no "background" (off) pixel color
    void noBgPixelColor();
};

/// Class for graphical LCD layer; created with DumbDisplay::createGraphicalLayer()
class GraphicalDDLayer: public DDLayer {
  public:
    /// for internal use only
    GraphicalDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// rotate the screen
    void setRotation(int8_t rotationType);
    /// set cursor to position
    void setCursor(int x, int y);
    /// move cursor by some amount
    void moveCursorBy(int byX, int byY);
    /// set text color and text background color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    /// @param bgColor empty background color means no background color
    void setTextColor(const String& color, const String& bgColor = "");
    /// set text size
    void setTextSize(int size);
    /// set font
    /// @param fontName empty means default
    /// @param textSize 0 means default
    void setTextFont(const String& fontName = "", int textSize = 0);
    /// set whether "print" will auto wrap or not
    void setTextWrap(bool wrapOn);
    /// fill screen with color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void fillScreen(const String& color);
    /// print text
    void print(const String& text);
    /// print text with line feed
    void println(const String& text = "");
    /// draw char
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    /// @param backgroundColor DD_RGB_COLOR(...) or common color name; empty background color means no background color
    /// @param size 0 means default
    void drawChar(int x, int y, char c, const String& color = "", const String& bgColor = "", int size = 0);
    /// draw string
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    /// @param backgroundColor DD_RGB_COLOR(...) or common color name; empty background color means no background color
    /// @param size 0 means default
    void drawStr(int x, int y, const String& string, const String& color = "", const String& bgColor = "", int size = 0);
    /// draw a pixel
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawPixel(int x, int y, const String& color = "");
    /// draw a line
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawLine(int x1, int y1, int x2, int y2, const String& color = "");
    /// draw a circle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawCircle(int x, int y, int r, const String& color = "", bool filled = false);
    // draw filled circle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    inline void fillCircle(int x, int y, int r, const String& color = "") {
      drawCircle(x, y, r, color, true);
    }
    /// draw a triangle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color = "", bool filled = false);
    /// draw filled triangle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    inline void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const String& color = "") {
      drawTriangle(x1, y1, x2, y2, x3, y3, color, true);
    }
    /// draw a rectangle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawRect(int x, int y, int w, int h, const String& color = "", bool filled = false);
    /// draw filled rectangle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    inline void fillRect(int x, int y, int w, int h, const String& color = "") {
      drawRect(x, y, w, h, color, true);
    }
    /// draw a rounded rectangle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawRoundRect(int x, int y, int w, int h, int r, const String& color = "", bool filled = false);
    /// draw filled rounded rectangle
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    inline void fillRoundRect(int x, int y, int w, int h, int r, const String& color = "") {
      drawRoundRect(x, y, w, h, r, color, true);
    }
    /// draw an ellipse (oval)
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawOval(int x, int y, int w, int h, const String& color = "", bool filled = false);
    /// draw filled ellipse (oval)
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    inline void fillOval(int x, int y, int w, int h, const String& color = "") {
      drawOval(x, y, w, h, color, true);
    }
    /// draw an arc
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    void drawArc(int x, int y, int w, int h, int startAngle, int sweepAngle, bool useCenter, const String& color = "", bool filled = false);
    /// draw filled arc
    /// @param color DD_RGB_COLOR(...) or common color name; empty color means text color
    inline void fillArc(int x, int y, int w, int h, int startAngle, int sweepAngle, bool useCenter, const String& color = "") {
      drawArc(x, y, w, h, startAngle, sweepAngle, useCenter, color, true);
    }
    /// move forward (relative to cursor)
    void forward(int distance);
    /// turn left
    void leftTurn(int angle);
    /// turn right
    void rightTurn(int angle);
    /// set heading angle (degree)
    void setHeading(int angle);
    /// set pen size
    void penSize(int size);
    /// set pen color (i.e. text color)
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void penColor(const String& color);
    /// set fill color (for shape)
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void fillColor(const String& color);
    /// set no fill color (for shape)
    void noFillColor();
    /// draw a circle; centered or not
    void circle(int radius, bool centered = false);
    /// draw an oval; centered or not
    void oval(int width, int height, bool centered = false);
    /// draw arc; centered or not
    void arc(int width, int height, int startAngle, int sweepAngle, bool centered = false);
    /// draw triangle (SAS)
    void triangle(int side1, int angle, int side2);
    /// draw isosceles triangle; given size and angle
    void isoscelesTriangle(int side, int angle);
    /// draw rectangle; centered or not
    void rectangle(int width, int height, bool centered = false);
    /// draw polygon given side and vertex count
    void polygon(int side, int vertexCount);
    /// draw polygon "enclosed" in an imaginary centered circle
    /// @param radius circle radius
    /// @param vertexCount number of vertices
    /// @param inside whether inside the imaginary circle or outside of it
    void centeredPolygon(int radius, int vertexCount, bool inside = false);
    /// write text; will not auto wrap
    /// @param draw means draw the text (in the heading direction)
    void write(const String& text, bool draw = false);
    /// load image file to cache
    /// - w / h: image size to scale to; if both 0, will not scale, if any 0, will scale keeping aspect ratio
    /// - asImageFileNmae: better provide a different name for the scaled cached
    void loadImageFile(const String& imageFileName, int w = 0, int h = 0, const String& asImageFileName = "");
    /// load image file to cache cropped
    /// @see loadImageFile()
    void loadImageFileCropped(const String& imageFileName, int x, int y, int w, int h, const String& asImageFileName = "");
    /// unload image file from cache
    void unloadImageFile(const String& imageFileName);
    /// draw image file in cache (if not already loaded to cache, load it) 
    /// - x / y: position of the left-top corne
    /// - w / h: image size to scale to; if both 0, will not scale, if any 0, will scale keeping aspect ratio
    void drawImageFile(const String& imageFileName, int x = 0, int y = 0, int w = 0, int h = 0);
    /// draw image file in cache (if not already loaded to cache, load it)
    /// - x / y / w / h: aread to draw the image; 0 means the default value
    /// - align (e.g. "LB"): left align "L"; right align "R"; top align "T"; bottom align "B"; default to fit centered
    void drawImageFileFit(const String& imageFileName, int x = 0, int y = 0, int w = 0, int h = 0, const String& align = "");
    /// cache image; not saved
    void cacheImage(const String& imageName, const uint8_t *bytes, int byteCount, char compressionMethod = 0);
    /// cache single-bit "pixel" image; not saved
    void cachePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color = "", char compressionMethod = 0);
    /// cache 16-bit "pixel" image; not saved
    void cachePixelImage16(const String& imageName, const uint16_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// cache greyscale "pixel" image; as if image saved and loaded
    void cachePixelImageGS(const String& imageName, const uint8_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// saved cached image
    /// @param imageName cachedImageName
    void saveCachedImageFile(const String& imageName);
    /// saved cached image
    /// @param stitchAsImageName if not empty, will stitch all cached images to one image file of the given name
    void saveCachedImageFiles(const String& stitchAsImageName = "");
};


/// Class for 7-segment row layer; created with DumbDisplay::create7SegmentRowLayer()
class SevenSegmentRowDDLayer: public DDLayer {
  public:
    /// for internal use only
    SevenSegmentRowDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// set segment color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void segmentColor(const String& color);
    /// reset segment off color; note that this will clear all digits
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void resetSegmentOffColor(const String& color);
    /// reset segment off color to no color; note that this will clear all digits
    void resetSegmentOffNoColor();
    /// turn on one or more segments
    /// @param segments each character represents a segment to turn on -- 
    ///                 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' 
    void turnOn(const String& segments, int digitIdx = 0);
    /// turn off one or more segments
    /// @param segments each character represents a segment to turn on -- 
    ///                 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.' 
    void turnOff(const String& segments, int digitIdx = 0);
    /// like turnOn(), exception that the digit will be cleared first
    /// @param segments empty segments basically means turn all segments of the digit off
    void setOn(const String& segments = "", int digitIdx = 0);
    /// show a digit
    void showDigit(int digit, int digitIdx = 0);
    /// show number (can be float)
    void showNumber(float number, const String& padding = " ");
    /// show HEX number
    void showHexNumber(int number);
    /// show formatted number (even number with hex digits);
    /// e.g. "12.00", "00.34", "-.12", "0ff"
    void showFormatted(const String& formatted, bool completeReplace = true, int startIdx = 0);
};


/// @brief
/// Class for virtual joystick layer; created with DumbDisplay::createJoystickLayer()
/// @since v0.9.7-r2
class JoystickDDLayer: public DDLayer {
  public:
    /// for internal use only  
    JoystickDDLayer(int8_t layerId): DDLayer(layerId) {
      _enableFeedback();
    }
    /// set joystick auto-recenter on / off; if auto recenter, after user releases the joystick, the joystick will move back to the center position automatically
    /// note that it will not affect the current position; use moveToCenter() to move to the center
    void autoRecenter(bool autoRecenter = true);
    /// set the colors of the stick UI
    void colors(const String& stickColor, const String& stickOutlineColor, const String& socketColor, const String& socketOutlineColor);
    /// move joystick position (if joystick is single directional, will only move in the movable direction)
    /// @param x x to move to
    /// @param x y to move to
    /// @param sendFeedback if true, will send "feedback" for the move (regardless of the current position)
    void moveToPos(int x, int y, bool sendFeedback = false);
    /// move joystick to the center
    /// @param sendFeedback if true, will send "feedback" for the move (regardless of the current position)
    void moveToCenter(bool sendFeedback = false);
};

/// Class for plotter layer; created with DumbDisplay::createPlotterLayer() or DumbDisplay::createFixedRatePlotterLayer()
class PlotterDDLayer: public DDLayer {
  public:
    /// for internal use only
    PlotterDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /* set label of value with certain key */
    /* if key has no label, the key will be the label */
    void label(const String& key, const String& lab);
    inline void label(const String& lab) { label("", lab); }
    // template<class... Args>
    // void setEx(Args... keyOrValue);
    /* set value of certain key */
    /* note that key can be empty */
    void set(const String& key, float value);  
    /* set value with empty key */
    inline void set(float value) { set("", value); }  
    void set(const String& key1, float value1, const String& key2, float value2);  
    void set(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3);  
    void set(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3, const String& key4, float value4);  
};


/// @brief
/// Class for TomTom map "device dependent view" layer, which means that it is solely rendered by the Android view that it hosts; 
/// created with DumbDisplay::createTomTomMapLayer()
class TomTomMapDDLayer: public DDLayer {
  public:
    /// for internal use only
    TomTomMapDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    void goTo(float latitude, float longitude, const String& label = "");
    void zoomTo(float latitude, float longitude, float zoomLevel = 15.0, const String& label = "");
    void zoom(float zoomLevel);
};


/// Class for a terminal-like "device dependent view" layer, for logging etc; created with DumbDisplay::createTerminalLayer()
class TerminalDDLayer: public DDLayer {
  public:
    /// for internal use only
    TerminalDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    void print(const String& val);
    void println(const String& val);
    inline void print(int intVal) { 
      String val(intVal);
      print(val);
    }
    inline void println(int intVal) { 
      String val(intVal);
      print(val);
    }
    inline void print(float floatVal) { 
      String val(floatVal);
      print(val);
    }
    inline void println(float floatVal) { 
      String val(floatVal);
      print(val);
    }
    inline void println() {
      println(""); 
    }
};


/// Helper class for constructing "tunnel" endpoint, if the endpoint is not a simple URL. Can be used for DDTunnel::reconnectToEndpoint()
class DDTunnelEndpoint {
  public:
    DDTunnelEndpoint(const String& endPoint) {
      this->endPoint = endPoint;
      this->headers = "";
      this->params = "";
    }
    /// reset the endpoint
    void resetEndpoint(const String& endPoint) {
      this->endPoint = endPoint;
    }
    /// reset the sound (saved or cached) to be attached with the request
    /// @param soundName name of the sound to be attached (saved or cached), empty means no sound attachment
    void resetSoundAttachment(const String& soundName) {
      if (soundName == "") {
        this->attachmentId = "";
      } else {
        this->attachmentId = "sound:" + soundName;
      }
    }
    /// reset headers
    void resetHeaders() {
      this->headers = "";
    }
    /// reset params (to the endpoint query string)
    void resetParams() {
      this->params = "";
    }
    /// add param (to the endpoint query string)
    void addParam(const String& param) {
      if (true) {
        if (this->params.length() > 0) {
          this->params.concat(',');
        }
        this->params.concat(param);
      } else {
        if (this->params == "") {
          this->params = param;
        } else {
          this->params = this->params + "," + param;
        }
      }
    }
    /// add named param (to the endpoint query string)
    void addNamedParam(const String& paramName, const String& paramValue) {
      addParam(paramName + "=" + paramValue);
    }
    /// add header to the request
    void addHeader(const String& headerKey, const String& headerValue) {
      if (true) {
        if (this->headers.length() > 0) {
          this->headers.concat('|');
        }
        this->headers.concat(headerKey);
        this->headers.concat(":");
        this->headers.concat(headerValue);
      } else {
        if (this->headers == "") {
          this->headers = String(headerKey) + ":" + headerValue;
        } else {
          this->headers = this->headers + "|" + headerKey + ":" + headerValue;
        }
      }
    }
  public:
    String endPoint;
    String headers;
    String attachmentId;
    String params;
};


/// Base class for DD "tunnel"
class DDTunnel: public DDObject {
  public:
    /// for internal use only
    DDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/);
    void afterConstruct(bool connectNow);
    virtual ~DDTunnel();
  public:
    virtual void release();
    virtual void reconnect();
    void reconnectTo(const String& endPoint) {
      this->endPoint = endPoint;
      reconnect();
    }
    /// reconnect to specified endpoint with parameters
    /// @param endPoint endpoint to connect to
    /// @param params parameters to to end point; empty if nont
    void reconnectToSetParams(const String& endPoint, const String& params) {
      this->endPoint = endPoint;
      this->params = params;
      reconnect();
    }
    /// reconnect to specified endpoint. See DDTunnelEndpoint.
    void reconnectToEndpoint(const DDTunnelEndpoint endpoint) {
      this->endPoint = endpoint.endPoint;
      this->headers = endpoint.headers;
      this->attachmentId = endpoint.attachmentId;
      this->params = endpoint.params;
      reconnect();
    }
    const String& getTunnelId() const { return tunnelId; }
  protected:
    //int _count();
    virtual bool _eof();
    //void _readLine(String &buffer);
    void _writeLine(const String& data);
    void _writeSound(const String& soundName);
  public:
    virtual void handleInput(const String& data, bool final);
  protected:
    String type;
    String tunnelId;
    String endPoint;
    String headers;
    String attachmentId;
    String params;
    long connectMillis;
    // int arraySize;
    // String* dataArray;
    // int nextArrayIdx;
    // int validArrayIdx;
  private:
    bool done;
};


/// Class for DD "tunnel", with buffering support; created with DumbDisplay::createBasicTunnel()
class DDBufferedTunnel: public DDTunnel {
  public:
    /// for internal use only
    DDBufferedTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize);
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
    /// @attention for internal use only
    virtual void handleInput(const String& data, bool final);
  private:
    // String endPoint;
    // String tunnelId;
    int8_t arraySize;
    String* dataArray;
    int8_t nextArrayIdx;
    int8_t validArrayIdx;
    //bool done;
  public:
    /// count buffer ready  read
    inline int count() { return _count(); }
    /// reached EOF?
    inline bool eof() { return _eof(); }
    /// read a line from buffer
    String readLine();
    /// read a line from buffer, in to the buffer passed in
    inline bool readLine(String &buffer) { return _readLine(buffer); }
    /// write a line
    inline void writeLine(const String& data) { _writeLine(data); }
    /// read a piece of JSON data
    bool read(String& fieldId, String& fieldValue);
};


/// support basic "text based line oriented" socket communication ... e.g.
/// ```
/// pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17")
/// ```
typedef DDBufferedTunnel BasicDDTunnel;


/// support simple REST GET api .. e.g.
/// ```
///  pTunnel = dumbdisplay.createJsonTunnel("http://worldtimeapi.org/api/timezone/Asia/Hong_Kong") 
/// ```
/// . read() will read JSON one piece at a time ... e.g.
/// ```
///  { 
///    "full_name": "Bruce Lee",
///    "name":
///    {
///      "first": "Bruce",
///      "last": "Lee"
///    },
///    "gender":"Male",
///    "age":32
///  }
/// ```
///   ==>
///   - `full_name` = `Bruce Lee`
///   - `name.first` = `Bruce`
///   -  `name.last` = `Lee`
///   -  `gender` = `Male`
///   -  `age` = `32`
///
typedef BasicDDTunnel JsonDDTunnel;

/// Class for basic tool "tunnel"
class SimpleToolDDTunnel: public BasicDDTunnel {
  public:
    /// @attention constructed via DumbDisplay object
    SimpleToolDDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int bufferSize):
        BasicDDTunnel(type, tunnelId, params, endPoint/*, connectNow*/, bufferSize) {
      this->result = 0;
    }
  public:
    virtual void reconnect();
    /// @return 0: not done; 1: done; -1: failed
    int checkResult(); 
  private:
    int result; 
};

/// Output struct of GpsServiceDDTunnel
struct DDLocation {
  float latitude;
  float longitude;
};
/// Class for GPS service "tunnel"
class GpsServiceDDTunnel: public BasicDDTunnel {
  public:
    /// @attention constructed via DumbDisplay object
    GpsServiceDDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize):
        BasicDDTunnel(type, tunnelId, params, endPoint/*, connectNow*/, bufferSize) {
    }
  public:
    /// @param repeat  how often (seconds) data will be sent back; -1 means no repeat
    void reconnectForLocation(int repeat = -1);
    bool readLocation(DDLocation& location);  
};


/// Output struct of ObjectDetetDemoServiceDDTunnel
struct DDObjectDetectDemoResult {
  int left;
  int top;
  int right;
  int bottom;
  String label;
};
/// Class for "object detection demo" service "tunnel"
class ObjectDetetDemoServiceDDTunnel: public BasicDDTunnel {
  public:
    /// @attention constructed via DumbDisplay object
    ObjectDetetDemoServiceDDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize):
        BasicDDTunnel(type, tunnelId, params, endPoint/*, connectNow*/, bufferSize) {
    }
  public:
    void reconnectForObjectDetect(const String& imageName);
    void reconnectForObjectDetectFrom(GraphicalDDLayer* pGraphicalLayer, const String& imageName);
    bool readObjectDetectResult(DDObjectDetectDemoResult& objectDetectResult);  
};



/// Class for "tunnel" multiplexer
/// @attention will not delete "tunnels" passed in
class JsonDDTunnelMultiplexer {
  public:
    /// @attention constructed via DumbDisplay object
    JsonDDTunnelMultiplexer(JsonDDTunnel** tunnels, int8_t tunnelCount);
    ~JsonDDTunnelMultiplexer();
    int count();
    bool eof();
    /// @return the index of the tunnel the field read from; -1 if non ready to read
    int read(String& fieldId, String& fieldValue);
    void release();
    void reconnect();
  private:
     int8_t tunnelCount;
     JsonDDTunnel** tunnels;
};

/// @struct DDIdleConnectionState
/// @brief
/// See DDIdleCallback
enum DDIdleConnectionState { IDLE_NOT_CONNECTED, IDLE_CONNECTING, IDLE_RECONNECTING };
/// @struct DDIdleCallback
/// @brief
/// Type signature for callback function that will be called when idle. See DumbDisplay::setIdleCallback()
/// @param idleForMillis  how long (millis) the connection has been idle
/// @param connectionState  the connection state
typedef void (*DDIdleCallback)(long idleForMillis, DDIdleConnectionState connectionState);
/// @struct DDConnectVersionChangedCallback
/// @brief
/// Type signature for callback function that will be called when connect version (counting up) changed. See DumbDisplay::setConnectVersionChangedCallback()
typedef void (*DDConnectVersionChangedCallback)(int connectVersion);


/// @struct DDDebugConnectionState
/// @brief
/// See DDDebugInterface
enum DDDebugConnectionState { DEBUG_NOT_CONNECTED, DEBUG_CONNECTING, DEBUG_CONNECTED, DEBUG_RECONNECTING, DEBUG_RECONNECTED };

/// Base class for debug callback set by calling DumbDisplay::debugSetup()
class DDDebugInterface {
  public:
    /// See DDDebugConnectionState
    virtual void logConnectionState(DDDebugConnectionState connectionState) {}
    /// @param state 1: start senging; 0: stop sending
    virtual void logSendCommand(int state) {}
};


/// Struct for the status values of calling DumbDisplay::connectPassive()
struct DDConnectPassiveStatus {
  /// connection made or not -- same as the return value of DumbDisplay::connectPassive() 
  bool connected;
  /// connecting: when not connected; starting to establish connection by sending hand-shake messages
  bool connecting;
  /// reconnecting: when connected; detected reconnecting (after lost of connection) 
  bool reconnecting;
};



extern bool _DDDisableParamEncoding;
inline void DDDebugDisableParamEncoding() { _DDDisableParamEncoding = true; }


/// @brief
/// The core class for DumbDisplay; everything starts here. The most important argument to DumbDisplay is an DDInputOutput object.
/// For an example, please refer to [Blink Test With Virtual Display, DumbDisplay](https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/)
class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO, uint16_t sendBufferSize = DD_DEF_SEND_BUFFER_SIZE/*, bool enableDoubleClick = true*/) {
#ifdef DD_DISABLE_PARAM_ENCODEING
    DDDebugDisableParamEncoding();
#endif      
#ifndef DD_NO_SERIAL      
      if (pIO->isForSerial() || pIO->isBackupBySerial()) {
        _The_DD_Serial = new DDSerial();
      }
#endif      
      initialize(pIO, sendBufferSize/*, enableDoubleClick*/);
    }
    /// explicitly make connection (blocking);
    /// implicitly called in situations like create a layer
    void connect();
    /// @return connected or not
    bool connected() const;
    /// @return the version of the connection, which when reconnected will be bumped up
    int getConnectVersion() const;
    /// @return compatibility version
    /// @note only meaningful after connection
    int getCompatibilityVersion() const;  
    /// by default, "long press feedback" and "double click feedback" is enabled; however, this makes "click feedback" detection less responsive;
    /// one remedy is to set for "single click feedback" only
    void setFeedbackSingleClickOnly(bool singleClickOnly = true);
    /// configure "auto pinning of layers" with the layer spec provided
    /// - horizontal: H(*)
    /// - vertical: V(*)
    /// - or nested, like H(0+V(1+2)+3);  where 0/1/2/3 are the layer ids
    /// - consider using the macros DD_AP_XXX
    void configAutoPin(const String& layoutSpec);
    /// add the "auto pin" config for layers not included in "auto pin" set by configAutoPin()
    /// @param remainingLayoutSpec 
    void addRemainingAutoPinConfig(const String& remainingLayoutSpec);
    /// configure "pin frame" to be x-units by y-units (default 100x100)
    /// @see pinLayer()
    void configPinFrame(int xUnitCount = 100, int yUnitCount = 100);
    /// pin a layer @ some position of an imaginary grid of "pin grame"
    /// - the imaginary grid size can be configured when calling connect() -- default is 100x100  
    /// - the input align (e.g. "LB") -- left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align
    void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    /// pin "auto pin" layers @ some position, like pinLayer()
    /// @param layoutSpec the "auto pin" layout specification to pin; see configAutoPin() for how spec is constructed
    /// @param align (e.g. "LB") -- left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align */
    void pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    /// create a Microbit-like layer
    MbDDLayer* createMicrobitLayer(int width = 5, int height = 5);
    /// create a Turtle-like layer
    TurtleDDLayer* createTurtleLayer(int width, int height);
    /// create a LED-grid layer
    LedGridDDLayer* createLedGridLayer(int colCount = 1, int rowCount = 1, int subColCount = 1, int subRowCount = 1);
    /// create a LCD layer
    LcdDDLayer* createLcdLayer(int colCount = 16, int rowCount = 2, int charHeight = 0, const String& fontName = "");
    /// create a graphical LCD layer
    /// @see GraphicalDDLayer
    GraphicalDDLayer* createGraphicalLayer(int width, int height);
    /// create a 7-segment layer 
    /// @param digitCount show how many digits; 1 by default
    /// @see SevenSegmentRowDDLayer
    SevenSegmentRowDDLayer* create7SegmentRowLayer(int digitCount = 1);
    /// create a joystick layer:
    /// - will send joystick positions as "feedback", and hence "feedback" is automatically enabled;
    /// - initial position is (0, 0)
    /// @param maxStickValue the max value of the stick; e.g. 255 or 1023 (the default); min is 15
    /// @param directions "lr" or "hori": left-to-right; "tb" or "vert": top-to-bottom; "rl": right-to-left; "bt": bottom-to-top;
    ///                   use "+" combines the above like "lr+tb" to mearn both directions; "" the same as "lr+tb" 
    /// @param stickLookScaleFactor the scaling factor of the stick (UI); 1 by default 
    /// @see JoystickDDLayer
    JoystickDDLayer* createJoystickLayer(int maxStickValue = 1023, const String& directions = "", float stickLookScaleFactor = 1.0);
    /// create a plotter layer
    PlotterDDLayer* createPlotterLayer(int width, int height, int pixelsPerSecond = 10);
    /// create a fixed-rate plotter layer
    /// i.e. it will assume fixe-rate of PlotterDDLayer::set()
    /// @see PlotterDDLayer
    PlotterDDLayer* createFixedRatePlotterLayer(int width, int height, int pixelsPerScale = 5);
    /// create a TomTom map layer
    /// @param mapKey should be provided; plesae visit TomTom's website to get one of your own
    ///               if pass in "" as mapKey, will use my testing one
    /// @see TomTomMapDDLayer
    TomTomMapDDLayer* createTomTomMapLayer(const String& mapKey, int width, int height);
    /// create a terminal layer
    /// @see TerminalDDLayer
    TerminalDDLayer* createTerminalLayer(int width, int height);
    /// create a "tunnel" for accessing the Web
    /// @note if not connect now, need to connect via reconnect()
    /// @see BasicDDTunnel
    BasicDDTunnel* createBasicTunnel(const String& endPoint, bool connectNow = true, int8_t bufferSize = DD_TUNNEL_DEF_BUFFER_SIZE);
    /// create a JSON 'tunnel' for thing like making RESTful calls
    /// @note if not connect now, need to connect via reconnect()
    /// @see BasicDDTunnel
    JsonDDTunnel* createJsonTunnel(const String& endPoint, bool connectNow = true, int8_t bufferSize = DD_TUNNEL_DEF_BUFFER_SIZE);
    /// create a JSON 'tunnel' for thing like making RESTful calls; with result filtered
    /// @note if not connect now, need to connect via reconnect()
    /// @param fieldNames comma-delimited list of field names to accept; note that matching is "case-insensitive containment match" 
    /// @see JsonDDTunnel
    JsonDDTunnel* createFilteredJsonTunnel(const String& endPoint, const String& fileNames, bool connectNow = true, int8_t bufferSize = DD_TUNNEL_DEF_BUFFER_SIZE);
    /// create a "tunnel" to download image from the web, and save the downloaded image to phone;
    /// you will get result in JSON format: ```{"result":"ok"}``` or ```{"result":"failed"}```
    /// for simplicity, use SimpleToolDDTunnel.checkResult() to check the result
    /// @see SimpleToolDDTunnel
    SimpleToolDDTunnel* createImageDownloadTunnel(const String& endPoint, const String& imageName, boolean redownload = true);
    /// create a "service tunnel" for getting date-time info from phone;
    /// use reconnectTo() with commands like
    /// - now
    /// - now-millis
    /// @see BasicDDTunnel
    BasicDDTunnel* createDateTimeServiceTunnel();
    /// create a "service tunnel" for getting GPS info from phone
    /// @see GpsServiceDDTunnel
    GpsServiceDDTunnel* createGpsServiceTunnel();
    /// create a "service tunnel" for getting object detection info from phone; model used is the demo model `mobilenetv1.tflite`
    /// @see ObjectDetetDemoServiceDDTunnel
    ObjectDetetDemoServiceDDTunnel* createObjectDetectDemoServiceTunnel(int scaleToWidth = 0, int scaleToHeight = 0);
    /// if finished using a "tunnel", delete it to release resource
    void deleteTunnel(DDTunnel *pTunnel);
    /// set DD background color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void backgroundColor(const String& color);
    /// basically, functions the same as recordLayerCommands()
    void recordLayerSetupCommands();
    /// basically, functions the same as playbackLayerCommands().
    /// additionally:
    /// - save and persiste the layer commands
    /// - enable DumbDisplay reconnect feature -- tells the layer setup commands to use when DumbDisplay reconnects
    void playbackLayerSetupCommands(const String& persist_id);
    /// start recording layer commands (of any layers);
    /// and sort of freeze the display, until playback
    void recordLayerCommands();
    /// playback recorded commands (unfreeze the display)
    void playbackLayerCommands();
    /// stop recording commands (and forget what have recorded)
    void stopRecordLayerCommands();
    /// save the recorded commands (and continue recording)
    /// @param id identifier of the recorded commands, overwriting and previous one;
    ///           if not recording, will delete previous recorded commands
    /// @param persist store it to your phone or not
    void saveLayerCommands(const String& id, bool persist = false);
    /// load saved commands (as if recording those commands)
    /// - recording started or not, will add the commands to the buffer
    /// - afterward, will keep recording
    /// - use playbackLayerCommands() to playback loaded commands
    /// - if not recording commands, this basically remove saved commands
    void loadLayerCommands(const String& id);
    /// capture and save display as image
    /// @param imageFileName: name of image file; if it ends with ".png", saved image format will be PNG; other, saved image format will be JPEG */
    /// @param width width of the display on which to render the layers
    /// @param height height of the display on which to render the layers
    /// @attention old file with the same name will be  replaced
    void capture(const String& imageFileName, int width, int height);
    /// send "no-op" command
    void sendNoOp();
    /// write out a comment to DD app
    void writeComment(const String& comment);
    /// make DD app sound a tone
    void tone(uint32_t freq, uint32_t duration);
    void notone();
    /// make DD app play the sound of the given sound file
    void playSound(const String& soundName);
    void stopSound();
    /// save 8-bit sound with the given sound samples
    void saveSound8(const String& soundName, const int8_t *bytes, int sampleCount, int sampleRate, int numChannels = 1);
    /// save 16-bit sound with the given sound samples
    void saveSound16(const String& soundName, const int16_t *data, int sampleCount, int sampleRate, int numChannels = 1);
    /// cache 8-bit sound with the given sound samples
    void cacheSound8(const String& soundName, const int8_t *bytes, int sampleCount, int sampleRate, int numChannels = 1);
    /// cache 16-bit sound with the given sound samples
    void cacheSound16(const String& soundName, const int16_t *data, int sampleCount, int sampleRate, int numChannels = 1);
    /// save the cached sound
    void saveCachedSound(const String& soundName);
    /// save the cached sound as C header file (.h)
    /// @warn this is experimental
    void saveCachedSoundAsH(const String& soundName);
    /// stream sound 8-bit sample (for playing sound)
    int streamSound8(int sampleRate, int numChannels = 1); 
    /// stream sound 16-bit sample (for playing sound)
    int streamSound16(int sampleRate, int numChannels = 1); 
    /// initiate saving of 8-bit sound sound chunked;
    /// use sendSoiundChunk8() to send sound data 
    int saveSoundChunked8(const String& soundName/*, const int8_t *bytes, int sampleCount*/, int sampleRate, int numChannels = 1);
    /// initiate saving of 16-bit sound sound chunked;
    /// use sendSoiundChunk16() to send sound data 
    int saveSoundChunked16(const String& soundName/*, const int16_t *data, int sampleCount*/, int sampleRate, int numChannels = 1);
    /// initiate caching of 8-bit sound sound chunked;
    /// use sendSoiundChunk8() to send sound data 
    int cacheSoundChunked8(const String& soundName/*, const int8_t *bytes, int sampleCount*/, int sampleRate, int numChannels = 1);
    /// initiate caching of 16-bit sound sound chunked;
    /// use sendSoiundChunk16() to send sound data 
    int cacheSoundChunked16(const String& soundName/*, const int16_t *data, int sampleCount*/, int sampleRate, int numChannels = 1);
    /// send 8-bit sound data chunk for saving/caching after calling saveSoundChunked8() or cacheSoundChunked8()
    void sendSoundChunk8(int chunkId, const int8_t *bytes, int sampleCount, bool isFinal = false);
    /// send 16-bit sound data chunk for saving/caching after calling saveSoundChunked16() or cacheSoundChunked16()
    void sendSoundChunk16(int chunkId, const int16_t *data, int sampleCount, bool isFinal = false);
    /// svae image with the given image data
    void saveImage(const String& imageName, const uint8_t *bytes, int byteCount);
    // save single-bit "pixel" image with the given image data
    void savePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color = "", char compressMethod = 0);
    // save 16-bit "pixel" image with the given image data
    void savePixelImage16(const String& imageName, const uint16_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    // save greyscale "pixel" image with the given image data
    void savePixelImageGS(const String& imageName, const uint8_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// stitch images together
    /// @param imageNames '+' delimited
    /// @param asImageName name for the stitched image
    void stitchImages(const String& imageNames, const String& asImageName);
    /// reorder the layer (by moving one layer in the z-order plane)
    /// @param how  can be "T" for top; or "B" for bottom; "U" for up; or "D" for down
    void reorderLayer(DDLayer *pLayer, const String& how);
    /// if layer is no longer used; delete it to release resources
    void deleteLayer(DDLayer *pLayer);
    /// loop through all the existing layers calling the function passed in
    void walkLayers(void (*walker)(DDLayer *));
    /// set 'idle callback', which will be called repeatedly in 2 situations:
    /// - no connection response while connecting
    /// - detected no 'keep alive' signal (i.e. reconnecting)
    /// @param idleCallback the callback function; see DDIdleCallback
    void setIdleCallback(DDIdleCallback idleCallback); 
    /// set callback when version changed (e.g. reconnected after disconnect)
    /// @param connectVersionChangedCallback the callback function; see DDConnectVersionChangedCallback
    void setConnectVersionChangedCallback(DDConnectVersionChangedCallback connectVersionChangedCallback);
    /// check if it is safe to print to Serial
    bool canPrintToSerial();
    /// log line to Serial; if it is not safe to output to Serial, will write comment with DumbDisplay::writeComment() instead
    void logToSerial(const String& logLine, bool force = false);
  public:
    /// @brief
    /// make connection passively; i.e. will not block, but will require continuous calling for making connection
    /// @return connection made or not (note that even if connection lost and requires reconnecting, it is still considered connected)
    /// @since 0.9.8-r1
    bool connectPassive(DDConnectPassiveStatus* pStatus = NULL);
    // // EXPERIMENTAL
    // void savePassiveConnectState(DDSavedConnectPassiveState& state);
    // // EXPERIMENTAL
    // void restorePassiveConnectState(DDSavedConnectPassiveState& state);
    /// EXPERIMENTAL; "master reset" to be just like uninitialized;
    /// "master reset" will:
    /// . disconnect from DD app (if connected)
    /// . delete all created layers and tunnels; hence, DO NOT use the pointers to them after "master reset"
    /// . DumbDisplay object will be just like at initial state; it will *not* be deleted
    /// @since 0.9.8-r1
    void masterReset();  
    // /// EXPERIMENTAL; like connectPassive();
    // /// if detected reconnecting, will "master reset", which ...
    // /// . disconnect from DD app (if connected)
    // /// . delete all created layers and tunnels (hence, DO NOT use the pointer to them)
    // /// . DumbDisplay object will be just like at initial state
    // bool connectPassiveReset();
  public:
    /// set debug use callback
    /// @param debugInterface a concrete implementation of DDDebugInterface 
    void debugSetup(DDDebugInterface *debugInterface);
    void debugOnly(int i);
  private:
    void initialize(DDInputOutput* pIO, uint16_t sendBufferSize/*, bool enableDoubleClick*/);
    //bool canLogToSerial();
};

#include "_dd_misc.h"

#endif

/*

============================

if want to disable int parameter encoding, define DD_DISABLE_PARAM_ENCODING before including dumbdisplay.h, like

#define DD_DISABLE_PARAM_ENCODING

=============================

*/

#ifdef DD_DISABLE_PARAM_ENCODING
  #pragma "message ***** DD_DISABLE_PARAM_ENCODING *****"
#endif




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
#define DD_DEF_IDLE_TIMEOUT     5000
#define DD_DEF_TUNNEL_TIMEOUT   150000


#include "_dd_util.h"


#define DD_CONDENSE_COMMAND


#define DD_HEX_COLOR(color) ("#" + String(color, 16))


#ifdef DD_CONDENSE_COMMAND
  #define DD_INT_COLOR(color) ("+" + DDIntEncoder(color).encoded())
  #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO) ||  defined(DD_DISABLE_PARAM_ENCODING)
    #define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
  #else
    #define DD_RGB_COLOR(r, g, b) ("+" + DDIntEncoder(0xffffff & ((((((int32_t) (r)) << 8) + ((int32_t) (g))) << 8) + ((int32_t) (b)))).encoded())
  #endif
  // #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
  //   #define DD_RGB_COLOR(r, g, b) (String(r) + "-" + String(g) + "-" + String(b))
  //   //#define DD_RGB_COLOR(r, g, b) (String(r<0?0:(r>255?255:r)) + "-" + String(g<0?0:(g>255?255:g)) + "-" + String(b<0?0:(b>255?255:b)))
  // #else
  //   #if defined(DD_DISABLE_PARAM_ENCODING)
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

#ifndef DD_TUNNEL_DEF_BUFFER_SIZE
  #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
    #define DD_TUNNEL_DEF_BUFFER_SIZE 2
  #else
    #define DD_TUNNEL_DEF_BUFFER_SIZE 4
  #endif
#endif
#if DD_TUNNEL_DEF_BUFFER_SIZE < 2
  #error "DD_TUNNEL_DEF_BUFFER_SIZE must be at least 2"
#endif 


#define DDIO_USE_DD_SERIAL

#include "_dd_serial.h"
#ifdef DDIO_USE_DD_SERIAL
  extern DDSerial* _The_DD_Serial;
#endif

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
#ifdef DD_NO_CUSTOM_DATA
  #warning ??? DD_NO_CUSTOM_DATA set ???
#else
    /// custom data
    String customData;
#endif
public:
    // since 20230601
    virtual ~DDObject() {}
};


/// experimental: _h is for internal use only
struct DDLayerHandle {
  int _h;
};


/// Base class for the different layers support by DumbDisplay; created with various layer creation methods of DumbDisplay, DumbDisplay::createLedGridLayer()
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
    /// @note layer property
    void border(float size, const String& color, const String& shape = "flat", float extraSize = 0);
    /// set no border
    /// @note layer property
    void noBorder();
    /// set padding for all sides
    /// @param size for size unit, see border()
    /// @note layer property
    void padding(float size);
    /// set padding for each side;
    /// for unit, see border()
    /// @note layer property
    void padding(float left, float top, float right, float bottom);
    /// set no padding
    /// @note layer property
    void noPadding();
    /// set margin for all sides
    /// @param size for size unit, see border()
    /// @note layer property
    void margin(float size);
    /// set margin for each side;
    /// for unit, see DDLayer::border()
    /// @note layer property
    void margin(float left, float top, float right, float bottom);
    /// set no margin
    /// @note layer property
    void noMargin();
    /// set layer background color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    /// @param opacity background opacity (0 - 100); it combines with layer opacity and alpha
    /// @note layer property
    void backgroundColor(const String& color, int opacity = 100);
    /// set no layer background color
    /// @note layer property
    void noBackgroundColor();
    /// set layer background image (on top of background color; follow opacity of background color)
    /// @param backgroundImageName name of the image
    ///                            can be a series of images like dumbdisplay_##0-7##.png (for dumbdisplay_0.png to dumbdisplay_7.png)
    ///                            which can be used for animation with animateBackgroundImage()
    /// @param drawBackgroundOptions options for drawing the background; same means as the option param of GraphicalDDLayer::drawImageFiler()
    /// @param refImageWidth the reference width of the image to scale the image (while keeping the aspect ration); 0 means no scaling
    void backgroundImage(const String& backgroundImageName, const String& drawBackgroundOptions, int refImageWidth = 0);
    /// set no layer background image
    void noBackgroundImage();
    /// export the layer as background image
    /// @param replace replace any existing background image; for add as an item of background image series that can be used for animation with animateBackgroundImage()
    /// @param noDrawBackground during export, do not draw background
    /// @param exportAsWidth the width of the image; 0 means the default (should be good enough)
    void exportAsBackgroundImage(bool replace = true, bool noDrawBackground = true, int exportAsWidth = 0);
    /// experimental:
    /// start animate background image series
    /// @param fps frames per second which is used to calculate the interval between the series of images
    /// @param reset reset to the first image in the series (before start animation)
    /// @param options can be "r" to reverse the order of the series of images
    void animateBackgroundImage(float fps, bool reset = true, const String& options = "");
    /// stop animate background image
    /// @param reset reset to the first image in the series
    void stopAnimateBackgroundImage(bool reset = true);
    /// set whether layer visible (not visible means hidden)
    /// @note layer property
    void visible(bool visible);
    /// set whether layer is transparent
    /// @note layer property
    void transparent(bool transparent);
    /// set layer disabled or not; if disabled, layer will not have "feedback", and its appearance will be like disabled 
    /// @note layer property
    void disabled(bool disabled = true);
    /// set layer opacity percentage
    /// @param opacity 0 - 100
    /// @note layer property
    void opacity(int opacity);
    /// set layer's alpha channel; it combines with opacity
    /// @param alpha 0 - 255
    /// @note layer property
    void alpha(int alpha);
    /// blending with "film" of color over the layer
    /// @param color 
    /// @param alpha 
    /// @param mode xfermode "darken" / "lighten" / "screen" / "overlay" / "" (no xfermode)
    /// @note layer property
    void blend(const String& color, int alpha = 255, const String& mode = "darken");
    /// no blending
    /// @note layer property
    void noblend();
    /// clear the layer
    void clear();
    /// normally used for "feedback" -- flash the default way (layer + border)
    void flash();
    /// normally used for "feedback" -- flash the area (x, y) where the layer is clicked
    void flashArea(int x, int y);
    /// trigger explicit "feedback" to the layer (similar to implicit "feedback" when layer is clicked) 
    /// @param type other than CLICK etc, can be CUSTOM (which is only possible with explicit "feedback");
    /// @param option can be "" / "keys" / "numkeys" / "confirm";
    ///               - in case of "keys" / "numkeys", input box will be popped up for user to enter the "text" of the "feedback";
    ///                 and the "text" in the parameter will be the "hint"
    ///               - in case of "confirm", a confirmation dialog will be popped up with "text" as the message,
    ///                 and the "feedback" "text" will be "Yes" or "No";
    /// @note feedback must be enabled for this to work
    void explicitFeedback(int16_t x = 0, int16_t y = 0, const String& text = "", DDFeedbackType type = DDFeedbackType::CLICK, const String& option = "");
    inline const String& getLayerId() const { return layerId; }
    /// set explicit (and more responsive) "feedback" handler (and enable feedback)
    /// @param handler "feedback" handler; see DDFeedbackHandler
    /// @param autoFeedbackMethod see DDLayer::enableFeedback()
    /// @param allowedFeedbackType can be comma-delimited list of "CLICK", "LONGPRESS" and "DOUBLECLICK"
    /// @note if you will not be making use of "feedback", you can disable it by defining DD_NO_FEEDBACK in order to reduce footprint 
    void setFeedbackHandler(DDFeedbackHandler handler, const String& autoFeedbackMethod = "", const String& allowFeedbackTypes = "");
    /// rely on getFeedback() being called
    /// @param autoFeedbackMethod
    /// - "" -- no auto feedback flash (the default)
    /// - "f" -- flash the standard way (layer + border)
    /// - "fl" -- flash the layer
    /// - "fa" -- flash the area where the layer is clicked
    /// - "fas" -- flash the area (as a spot) where the layer is clicked
    /// - "fs" -- flash the spot where the layer is clicked (regardless of any area boundary)
    /// @param allowedFeedbackType can be comma-delimited list of "CLICK", "LONGPRESS" and "DOUBLECLICK"
    /// @note if you will not be making use of "feedback", you can disable it by defining DD_NO_FEEDBACK in order to reduce footprint 
    void enableFeedback(const String& autoFeedbackMethod = "", const String& allowFeedbackTypes = "");
    /// disable "feedback"
    void disableFeedback();
    /// get "feedback" DDFeedback
    /// @return NULL if no pending "feedback"
    const DDFeedback* getFeedback();
    /// for debug use
    void debugOnly(int i);
  public:
#ifndef DD_NO_FEEDBACK
    /// @attention used internally
    inline DDFeedbackManager* getFeedbackManager() const { return pFeedbackManager; }
    /// @attention used internally
    inline DDFeedbackHandler getFeedbackHandler() const { return feedbackHandler; }
#endif    
  protected:
    DDLayer(int8_t layerId);
  public:
    // made virtual since 20230601
    virtual ~DDLayer();
  protected:
    void _enableFeedback();
  protected:
    String layerId;  
#ifndef DD_NO_FEEDBACK
    // either feedbackManager or feedbackHandler is used
    DDFeedbackManager *pFeedbackManager;
    DDFeedbackHandler feedbackHandler;
#endif    
};

#define DD_DEF_LAYER_LEVEL_ID "_"

/// Base class for a multi-level layer.
/// A layer (single-level or multi-level) will have at least one level (DD_DEF_LAYER_LEVEL_ID). 
/// A multi-level layer can have other named levels, which act like separate sub-layers with separate sets of non-layer-specific properties.
class MultiLevelDDLayer: public DDLayer {
  public:
    /// add a level, optionally change its "opening" size
    /// @param levelId level ID; cannot be DD_DEF_LAYER_LEVEL_ID
    /// @param width width of the level "opening"; 0 means the maximum width (the width of the layer)
    /// @param height height of the level "opening"; 0 means the maximum height (the height of the layer)
    void addLevel(const String& levelId, float width, float height, bool switchToIt = false);
    /// another version of addLevel()
    inline void addLevel(const String& levelId, bool switchToIt = false) {
      addLevel(levelId, 0, 0, switchToIt);
    }
    /// like addLevel() but add to the top (i.e. will be drawn last)
    void addTopLevel(const String& levelId, float width = 0, float height = 0, bool switchToIt = false);
    /// another version of addTopLevel()
    inline void addTopLevel(const String& levelId, bool switchToIt) {
      addTopLevel(levelId, 0, 0, switchToIt);
    }
    /// switch to a different level (which is like a sub-layer), making it the current level
    /// @param levelId level ID; use DD_DEF_LAYER_LEVEL_ID for the default level
    /// @param addIfMissing if true, add the level if it is missing
    void switchLevel(const String& levelId, bool addIfMissing = true);
    /// push the current level onto the level stack, to be pop with popLevel()
    void pushLevel(); 
    /// push the current level onto the level stack, to be pop with popLevel()
    /// @param switchTolevelId switch to level ID (after pushing current level)
    void pushLevelAndSwitchTo(const String& switchTolevelId, bool addIfMissing = true); 
    /// pop a level from the level stack and make it the current level
    void popLevel();
    /// set the opacity of the current level 
    /// @param opacity background opacity (0 - 100)
    void levelOpacity(int opacity);
    /// set whether level is transparent
    void levelTransparent(bool transparent);
    /// set the anchor of the level; note that level anchor is the top-left corner of the level "opening"
    void setLevelAnchor(float x, float y, long reachInMillis = 0);
    /// move the level anchor
    void moveLevelAnchorBy(float byX, float byY, long reachInMillis = 0);
    /// register an image for setting as level's background
    /// @param backgroundId id to identify the background -- see setLevelBackground()
    /// @param backgroundImageName name of the image
    ///                            can be a series of images like dumbdisplay_##0-7##.png (for dumbdisplay_0.png to dumbdisplay_7.png)
    ///                            which can be used for animation with animateLevelBackground()
    /// @param drawBackgroundOptions options for drawing the background; same means as the option param of GraphicalDDLayer::drawImageFiler()
    void registerLevelBackground(const String& backgroundId, const String& backgroundImageName, const String& drawBackgroundOptions = "");
    /// experimental:
    /// export the current level as a registered background image -- see exportLevelsAsImage() and registerLevelBackground()
    /// @param backgroundId id to identify the background -- see setLevelBackground()
    /// @param replace if true (default), replace the existing registered background image with the same id;
    ///                if false, will add as an item of background image series that can be used for animation with animateLevelBackground()
    void exportLevelAsRegisteredBackground(const String& backgroundId, bool replace = true);
    /// set a registered background image as the current level's background
    /// @param backgroundId 
    /// @param backgroundImageName if not registered, the name of the image to register;
    ///                            can be a series of images like dumbdisplay_##0-7##.png (for dumbdisplay_0.png to dumbdisplay_7.png)
    ///                            which can be used for animation with animateLevelBackground()
    /// @param drawBackgroundOptions if not registered, the options for drawing the background
    void setLevelBackground(const String& backgroundId, const String& backgroundImageName = "", const String& drawBackgroundOptions = "");
    /// set that the current level uses no background image
    void setLevelNoBackground();
    /// start animate level background (if level background has a series of images)
    /// @param fps frames per second which is used to calculate the interval between the series of images
    /// @param reset reset to the first image in the series (before start animation)
    /// @param options can be "r" to reverse the order of the series of images
    void animateLevelBackground(float fps, bool reset = true, const String& options = "");
    /// stop animate level background
    /// @param reset reset to the first image in the series
    void stopAnimateLevelBackground(bool reset = true);
    /// reorder the specified level (by moving it in the z-order plane)
    /// @param how  can be "T" for top; or "B" for bottom; "U" for up; or "D" for down
    void reorderLevel(const String& levelId, const String& how);
    /// export (and save) the levels as an image (without the decorations of the layer like border)
    void exportLevelsAsImage(const String& imageFileName, bool cacheItNotSave = false);
    /// delete the specified level
    void deleteLevel(const String& levelId);
  protected:
    MultiLevelDDLayer(int8_t layerId): DDLayer(layerId) {}
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
class TurtleDDLayer: public MultiLevelDDLayer {
  public:
    /// for internal use only
    TurtleDDLayer(int8_t layerId): MultiLevelDDLayer(layerId) {
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

struct LedGridDDLayerHandle: DDLayerHandle {};


/// Class for LED grid layer; created with DumbDisplay::createLedGridLayer()
class LedGridDDLayer: public DDLayer {
  public:
    /// experimental: construct a "transient" LedGridDDLayer from LedGridDDLayerHandle
    /// created using DUmbDisplay::createLcdLayerHandle()
    LedGridDDLayer(LedGridDDLayerHandle layerHandle): DDLayer(layerHandle._h) {
    }
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
    /// @param bits most significant bit (bit 0) maps to left-most LED
    /// @param y row
    /// @param reverse true means reversed ... i.e. least significant bit (bit 31) maps to left-most LED
    void bitwise(unsigned int bits, int y = 0, bool reverse = false);
    /// turn on/off two rows of LEDs by bits
    /// @see bitwise()
    void bitwise2(unsigned int bits_0, unsigned int bits_1, int y = 0, bool reverse = false);
    /// turn on/off three rows of LEDs by bits
    /// @see bitwise()
    void bitwise3(unsigned int bits_0, unsigned int bits_1, unsigned int bits_2, int y = 0, bool reverse = false);
    /// turn on/off four rows of LEDs by bits
    /// @see bitwise()
    void bitwise4(unsigned int bits_0, unsigned int bits_1, unsigned int bits_2, unsigned int bits_3, int y = 0, bool reverse = false);
    /// turn on LEDs to form a horizontal "bar"
    /// @see bitwise()
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


struct LcdDDLayerHandle: DDLayerHandle {};

/// @brief
/// Class for LCD layer; created with DumbDisplay::createLcdLayer()
/// @note with "feedback" enabled, can be used as a button
/// @note with "feedback" enabled, can be used as checkbox; consider using these emojis for checkbox --
/// ☒☐✅❎🟩✔️ ☑️⬛✔✖
class LcdDDLayer: public DDLayer {
  public:
    /// experimental: construct a "transient" LcdDDLayer from LcdDDLayerHandle
    /// created using DUmbDisplay::createLcdLayerHandle()
    LcdDDLayer(LcdDDLayerHandle layerHandle): DDLayer(layerHandle._h) {
    }
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
    /// write text as a line (of y-th row)
    /// @param align 'L', 'C', or 'R'
    void writeLine(const String& text, int y = 0, const String& align = "L");
    /// write text as a right-aligned line
    void writeRightAlignedLine(const String& text, int y = 0);
    /// write text as a line, with align "centered"
    void writeCenteredLine(const String& text, int y = 0);
    /// set pixel color
    /// @param color DD_RGB_COLOR(...) or common color name
    void pixelColor(const String &color);
    /// set "background" (off) pixel color
    /// @param color DD_RGB_COLOR(...) or common color name
    void bgPixelColor(const String &color, bool sameForBackgroundColor = false, int backgroundOpacity = 100);
    /// set no "background" (off) pixel color
    void noBgPixelColor();
};


class SelectionBaseDDLayer: public DDLayer {
  public:
    /// for internal use only
    SelectionBaseDDLayer(int8_t layerId): DDLayer(layerId) {
      _enableFeedback();
    }
    /// set pixel color (of bot selected and unselected "selection" units)
    /// @param color DD_RGB_COLOR(...) or common color name
    /// please use the other version of pixelColor() to select different colors for selected and unselected
    void pixelColor(const String &color);
    /// set pixel color (when whether selected or not have different colors)
    /// @param color DD_RGB_COLOR(...) or common color name
    /// please use the other version of pixelColor() to select a single color for both selected and unselected
    void pixelColor(const String &color, bool selected);
    // /// set background pixel color
    // /// @param color DD_RGB_COLOR(...) or common color name
    // void bgPixelColor(const String &color, bool sameForBackgroundColor = false, int backgroundOpacity = 100);
    /// select all "selection" units
    void selectAll();
    /// deselect all "selection" units
    void deselectAll();
    /// set selected / unselected "selection" unit border characteristics 
    /// @param borderColor DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"; "" means default
    /// @param borderShape can be "flat", "hair", "round", "raised" or "sunken"; "" means default  
    void highlightBorder(bool forSelected, const String& borderColor = "", const String& borderShape = "");
};

/// @brief
/// Class for "selection" layer of multiple LCD-like "selection" units; created with DumbDisplay::createSelectionLayer()
/// @note by default, it has "feedback" enabled to indicate which "selection" unit is clicked
/// @since v0.9.9-r10
class SelectionDDLayer: public SelectionBaseDDLayer {
  public:
    /// for internal use only
    SelectionDDLayer(int8_t layerId): SelectionBaseDDLayer(layerId) {}
    /// set a "selection" unit text (of y-th row)
    /// @param align 'L', 'C', or 'R'
    void text(const String& text, int y = 0, int horiSelectionIdx = 0, int vertSelectionIdx = 0, const String& align = "L");
    /// set a "selection" unit centered text (of y-th row)
    void textCentered(const String& text, int y = 0, int horiSelectionIdx = 0, int vertSelectionIdx = 0);
    /// set a "selection" unit right-aligned text (of y-th row)
    void textRightAligned(const String& text, int y = 0, int horiSelectionIdx = 0, int vertSelectionIdx = 0);
    /// set a "selection" unit text (of y-th row) when unselected (it defaults to the same text as selected)
    /// @param align 'L', 'C', or 'R'
    void unselectedText(const String& text, int y = 0, int horiSelectionIdx = 0, int vertSelectionIdx = 0, const String& align = "L");
    /// set a "selection" unit centered text (of y-th row) when unselected (it defaults to the same text as selected)
    void unselectedTextCentered(const String& text, int y = 0, int horiSelectionIdx = 0, int vertSelectionIdx = 0);
     /// set a "selection" unit right-aligned text (of y-th row) when unselected (it defaults to the same text as selected)
   void unselectedTextRightAligned(const String& text, int y = 0, int horiSelectionIdx = 0, int vertSelectionIdx = 0);
    /// select a "selection" unit
    void select(int horiSelectionIdx = 0, int vertSelectionIdx = 0, bool deselectTheOthers = true);
    /// deselect a "selection" unit
    void deselect(int horiSelectionIdx = 0, int vertSelectionIdx = 0, bool selectTheOthers = false);
    /// @deprecated
    void setSelected(bool selected, int horiSelectionIdx = 0, int vertSelectionIdx = 0) {
      if (selected) {
        select(horiSelectionIdx, vertSelectionIdx, false);
      } else {
        deselect(horiSelectionIdx, vertSelectionIdx, false);
      }
    }
    /// set a "selection" unit selected or not (combination of select() and deselect())
    void selected(bool selected, int horiSelectionIdx = 0, int vertSelectionIdx = 0, bool reverseTheOthers = false);
};

/// @brief
/// Class for "selection list" layer, like SelectionDDLayer but "selections" can be added and removed dynamically; it can be created with DumbDisplay::createListSelectionLayer()
/// Consider using SelectionListLayerHelper / SelectionListLayerWrapper for richer tracking of the "list"
/// @note by default, it has "feedback" enabled to indicate which "selection" unit is clicked
/// @since v0.9.9-r41
class SelectionListDDLayer: public SelectionBaseDDLayer {
  public:
    /// for internal use only
    SelectionListDDLayer(int8_t layerId): SelectionBaseDDLayer(layerId) {}
    /// add (insert) a "selection" unit
    /// @param selectionIdx add (insert) the selection to index
    void add(int selectionIdx);
    /// remove a "selection" unit
    /// @param selectionIdx remove the selection at index
    void remove(int selectionIdx);
    /// set the offset to the "selection" unit start showing
    void offset(int offset);
    /// set a "selection" unit text (of y-th row)
    /// @param align 'L', 'C', or 'R'
    void text(int selectionIdx, const String& text, int y = 0, const String& align = "L");
    /// set a "selection" unit centered text (of y-th row)
    void textCentered(int selectionIdx, const String& text, int y = 0);
    /// set a "selection" unit right-aligned text (of y-th row)
    void textRightAligned(int selectionIdx, const String& text, int y = 0);
    /// set a "selection" unit text (of y-th row) when unselected (it defaults to the same text as selected)
    /// @param align 'L', 'C', or 'R'
    void unselectedText(int selectionIdx, const String& text, int y = 0, const String& align = "L");
    /// set a "selection" unit centered text (of y-th row) when unselected (it defaults to the same text as selected)
    void unselectedTextCentered(int selectionIdx, const String& text, int y = 0);
     /// set a "selection" unit right-aligned text (of y-th row) when unselected (it defaults to the same text as selected)
   void unselectedTextRightAligned(int selectionIdx, const String& text, int y = 0);
    /// select a "selection" unit
    void select(int selectionIdx, bool deselectTheOthers = true);
    /// deselect a "selection" unit
    void deselect(int selectionIdx, bool selectTheOthers = false);
    /// set a "selection" unit selected or not (combination of select() and deselect())
    void selected(int selectionIdx, bool selected, bool reverseTheOthers = false);
};


struct GraphicalDDLayerHandle: DDLayerHandle {};


/// Class for graphical LCD layer; created with DumbDisplay::createGraphicalLayer()
class GraphicalDDLayer: public MultiLevelDDLayer {
  public:
    /// experimental: construct a "transient" LcdDDLayer from LcdDDLayerHandle
    /// created using DUmbDisplay::createGraphicalLayerHandle()
    GraphicalDDLayer(GraphicalDDLayerHandle layerHandle): MultiLevelDDLayer(layerHandle._h) {
    }
  public:
    /// for internal use only
    GraphicalDDLayer(int8_t layerId): MultiLevelDDLayer(layerId) {
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
    void setTextFont(const String& fontName);
    /// reset text font and size
    void setTextFont();
    /// @deprecated
    inline void setTextFont(const String& fontName, int textSize) {
      setTextFont(fontName);
      if (textSize > 0) {
        setTextSize(textSize);
      }
    }
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
    /// similar to drawStr(), but draw string as a text line at (0, y) with alignment option 
    /// @param align 'L', 'C', or 'R'
    void drawTextLine(const String& text, int y, const String& align = "L", const String& color = "", const String& bgColor = "", int size = 0);
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
    void forward(int distance, bool withPen = true);
    /// move backward (relative to cursor)
    void backward(int distance, bool withPen = true);
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
    /// @param w,h: image size to scale to; if both 0, will not scale, if any 0, will scale keeping aspect ratio
    /// @param asImageFileName: better provide a different name for the scaled cached
    void loadImageFile(const String& imageFileName, int w = 0, int h = 0, const String& asImageFileName = "");
    /// load image file to cache cropped
    /// @param x,y,w,h: rect to crop the image
    /// @param asImageFileName: since image cropped, should provide a different name for the scaled cached
    /// @param
    /// @see loadImageFile()
    inline void loadImageFileScaled(const String& imageFileName, int w, int h, const String& asImageFileName) {
      loadImageFile(imageFileName, w, h, asImageFileName);
    }
    void loadImageFileCropped(const String& imageFileName, int x, int y, int w, int h, const String& asImageFileName, int scaleW = 0, int scaleH = 0);
    /// unload image file from cache
    void unloadImageFile(const String& imageFileName);
    /// unload all image files from cache
    void unloadAllImageFiles();
    /// draw image file in cache (if not already loaded to cache, load it) 
    /// @param x,y: position of the left-top corner
    /// @param w,h: image size to scale to; if both 0, will not scale, if any 0, will scale keeping aspect ratio
    void drawImageFile(const String& imageFileName, int x = 0, int y = 0, int w = 0, int h = 0, const String& options = "");
    /// another version of drawImageFile() with options
    inline void drawImageFile(const String& imageFileName, const String& options) {
      drawImageFile(imageFileName, 0, 0, 0, 0, options);
    }
    /// draw image file in cache scaled, like calling drawImageFile(imageFileName, 0, 0, w, h, options) with w and h
    /// @see drawImageFile()
    inline void drawImageFileScaled(const String& imageFileName, int w, int h, const String& options = "") {
      drawImageFile(imageFileName, 0, 0, w, h, options);
    }
    /// draw image file in cache (if not already loaded to cache, load it)
    /// @param x,y,w,h: rect to draw the image; 0 means the default value
    /// @param options (e.g. "LB"): left align "L"; right align "R"; top align "T"; bottom align "B"; default to fit centered
    void drawImageFileFit(const String& imageFileName, int x = 0, int y = 0, int w = 0, int h = 0, const String& options = "");
    /// another version of drawImageFileFit() with options
    inline void drawImageFileFit(const String& imageFileName, const String& options) {
      drawImageFileFit(imageFileName, 0, 0, 0, 0, options);
    }
    /// cache image; not saved
    /// @param imageName cachedImageName
    void cacheImage(const String& imageName, const uint8_t *bytes, int byteCount, char compressionMethod = 0);
    /// cache image with specified timestamp; not saved
    /// @param imageName cachedImageName
    void cacheImageWithTS(const String& imageName, const uint8_t *bytes, int byteCount, long imageTimestamp, char compressionMethod = 0);
    /// cache single-bit "pixel" image (i.e. B&W image); not saved
    /// @param imageName cachedImageName
    void cachePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color = "", char compressionMethod = 0);
    /// cache 16-bit "pixel" image (i.e. 565 RGB image); not saved
    /// @param imageName cachedImageName
    void cachePixelImage16(const String& imageName, const uint16_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// cache grayscale "pixel" image; not saved
    /// @param imageName cachedImageName
    void cachePixelImageGS(const String& imageName, const uint8_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// saved cached image
    /// @param imageName cachedImageName
    void saveCachedImageFile(const String& imageName, const String& asImageName = "");
#ifdef ESP32
    /// saved cached image with timestamp
    /// @param imageName cachedImageName
    void saveCachedImageFileWithTS(const String& imageName, const String& asImageName, int64_t imageTimestamp);
#endif
    /// saved cached image (async / non-blocking)
    /// @param imageName cachedImageName
    void saveCachedImageFileAsync(const String& imageName, const String& asImageName = "");
#ifdef ESP32
    /// saved cached image (async / non-blocking)
    /// @param imageName cachedImageName
    void saveCachedImageFileWithTSAsync(const String& imageName, const String& asImageName, int64_t imageTimestamp);
#endif
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
    /// like turnOn(), except that the digit will be cleared first
    /// @param segments empty segments basically means turn all segments of the digit off
    void setOn(const String& segments = "", int digitIdx = 0);
    /// show a digit
    void showDigit(int digit, int digitIdx = 0);
    /// show number (can be float)
    void showNumber(float number, const String& padding = " ");
    /// show HEX number
    void showHexNumber(int16_t number);
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
    void colors(const String& stickColor, const String& stickOutlineColor, const String& socketColor = "", const String& socketOutlineColor = "");
    /// move joystick position (if joystick is single directional, will only move in the movable direction)
    /// @param x x to move to
    /// @param x y to move to
    /// @param sendFeedback if true, will send "feedback" for the move (regardless of the current position)
    void moveToPos(int16_t x, int16_t y, bool sendFeedback = false);
    /// move joystick to the center
    /// @param sendFeedback if true, will send "feedback" for the move (regardless of the current position)
    void moveToCenter(bool sendFeedback = false);
    /// set stick max value; will also move the joystick position to "home" -- center if auto-recenter else (0, 0)
    /// @param minValue the min value of the stick
    /// @param maxValue the max value of the stick
    /// @param sendFeedback if true, will send "feedback" for the move (regardless of the current position)
    void valueRange(int16_t minValue, int16_t maxValue, int valueStep = 1, bool sendFeedback = false);
    /// set 'snappy' makes stick snaps to closest value when moved
    void snappy(bool snappy = true);
    /// show value on top of the stick 
    void showValue(bool show = true, const String& color = "");
};

/// Class for plotter layer; created with DumbDisplay::createPlotterLayer() or DumbDisplay::createFixedRatePlotterLayer()
class PlotterDDLayer: public DDLayer {
  public:
    /// for internal use only
    PlotterDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    /// set label of value with certain key
    /// if key has no label, the key will be the label
    void label(const String& key, const String& lab);
    inline void label(const String& lab) { label("", lab); }
    /// set value of certain key
    /// note that key can be empty
    void set(const String& key, float value);  
    /// set value with empty key
    inline void set(float value) { set("", value); }  
    void set(const String& key1, float value1, const String& key2, float value2);  
    void set(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3);  
    void set(const String& key1, float value1, const String& key2, float value2, const String& key3, float value3, const String& key4, float value4);  
};


/// Class for TomTom map "device dependent view" layer, which means that it is solely rendered by the Android view that it hosts; 
/// see created with DumbDisplay::createTomTomMapLayer()
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

/// Class for a WebView "device dependent view" layer;
/// see created with DumbDisplay::createWebViewLayer()
class WebViewDDLayer: public DDLayer {
  public:
    /// for internal use only
    WebViewDDLayer(int8_t layerId): DDLayer(layerId) {
      _enableFeedback();
    }
    void loadUrl(const String& url);
    void loadHtml(const String& html);
    void execJs(const String& js);
};


/// Class for DumbDisplay "window" "device dependent view" layer, which creates a "window" for connecting to other device's DumbDisplay
/// see created with DumbDisplay::createDumbDisplayWindowLayer()
class DumbDisplayWindowDDLayer: public DDLayer {
  public:
    /// for internal use only
    DumbDisplayWindowDDLayer(int8_t layerId): DDLayer(layerId) {
    }
    void connect(const String& deviceType, const String& deviceName, const String& deviceAddress);
    void disconnect();
};

/// Class for a RTSP-client "device dependent view" layer;
/// see created with DumbDisplay::createRtspClient()
class RtspClientDDLayer: public DDLayer {
  public:
    /// for internal use only
    RtspClientDDLayer(int8_t layerId): DDLayer(layerId) {}
    /// @param url e.g. rtsp://192.168.0.154 
    void start(const String& url);
    void stop();
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
    const String& getEndpoint() { return endPoint; }   
  public:
    virtual void release();
  protected:
    virtual void _reconnect(const String& extraParams = "");
    void _reconnectTo(const String& endPoint, const String& extraParams = "");
  public:
    const String& getTunnelId() const { return tunnelId; }
    /// reconnect to specified endpoint with parameters
    /// @param endPoint endpoint to connect to
  public:
    inline void reconnectTo(const String& endPoint) { _reconnectTo(endPoint); }
  protected:
    // /// reconnect to specified endpoint with parameters
    // /// @param endPoint endpoint to connect to
    // /// @param params parameters to to end point; empty if none
    // void reconnectToSetParams(const String& endPoint, const String& params) {
    //   this->endPoint = endPoint;
    //   this->params = params;
    //   _reconnect();
    // }
  public:
    /// reconnect to specified endpoint. See DDTunnelEndpoint.
    void reconnectToEndpoint(const DDTunnelEndpoint endpoint) {
      this->endPoint = endpoint.endPoint;
      this->headers = endpoint.headers;
      this->attachmentId = endpoint.attachmentId;
      this->params = endpoint.params;
      _reconnect();
    }
  protected:
    inline bool _pending() { return doneState == 0; }
    //int _count();
    inline bool _timedOut() { return /*timedOut*/doneState == -1; }
    virtual bool _eof(long timeoutMillis);
    //void _readLine(String &buffer);
    void _writeLine(const String& data);
    void _writeSound(const String& soundName);
  public:
    virtual void handleInput(const String& data, uint8_t* fbBytes, bool final) { doneHandleInput(final); }
  protected:
    void doneHandleInput(bool final);   
  protected:
    String type;
    String tunnelId;
    String endPoint;
    String headers;
    String attachmentId;
    String params;
    unsigned long connectMillis;
    // int arraySize;
    // String* dataArray;
    // int nextArrayIdx;
    // int validArrayIdx;
  private:
    //bool done;
    //bool timedOut;
    int8_t doneState;  // 0 not done; 1 done; -1 timed out
};


/// Class for DD "tunnel", with buffering support; created with DumbDisplay::createBasicTunnel()
class DDBufferedTunnel: public DDTunnel {
  public:
    /// for internal use only
    DDBufferedTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize);
    virtual ~DDBufferedTunnel();
  public:
    virtual void release();
  protected:
    virtual void _reconnect(const String& extraParams = "");
    //const String& getTunnelId() { return tunnelId; }
  public:
    inline void reconnect() { _reconnect(); }
  protected:
    int _count();
    virtual bool _eof(long timeoutMillis);
    bool _readLine(String &buffer, uint8_t** pFBBytes = NULL);
    //void _writeLine(const String& data);
  public:
    /// @attention for internal use only
    virtual void handleInput(const String& data, uint8_t* fbBytes, bool final);
  private:
    // String endPoint;
    // String tunnelId;
    int8_t arraySize;
    String* dataArray;
    uint8_t** fbByesArray;
    int8_t nextArrayIdx;
    int8_t validArrayIdx;
    //bool done;
  public:
    bool pending();
    /// count buffer ready read
    inline int count() { return _count(); }
    /// reached EOF?
    /// @param timeoutMillis timeout in millis; see DDTunnel::timedOut()
    /// @return true if EOF (or timed out)
    inline bool eof(long timeoutMillis = DD_DEF_TUNNEL_TIMEOUT) { return _eof(timeoutMillis); }
    /// check whether EOF caused by timeout or not; note that timeout is only due to check of EOF with DDBufferedTunnel::eof() with timeoutMillis set
    inline bool timedOut() { return _timedOut(); }
    /// read a line from buffer
    String readLine();
    /// read a line from buffer, in to the buffer passed in
    inline bool readLine(String &buffer) { return _readLine(buffer); }
    /// write a line
    inline void writeLine(const String& data) { _writeLine(data); }
    /// read a piece of JSON data
    bool read(String& fieldId, String& fieldValue);
};


/// @struct BasicDDTunnel
/// support basic "text based line oriented" socket communication ... e.g.
/// ```
/// pTunnel = dumbdisplay.createBasicTunnel("djxmmx.net:17")
/// ```
typedef DDBufferedTunnel BasicDDTunnel;

/// @struct JsonDDTunnel
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
  protected:
    virtual void _reconnect(const String& extraParams = "");
  public:
    //inline void reconnect() { _reconnect(); }
    /// @return 0: not done; 1: done; -1: failed
    int checkResult(); 
  private:
    int result; 
};

/// Image download tool "tunnel" created with DumbDisplay::createImageDownloadTunnel()
class ImageDownloadDDTunnel: public SimpleToolDDTunnel {
  public:
    /// @attention constructed via DumbDisplay object
    ImageDownloadDDTunnel(int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int bufferSize):
      SimpleToolDDTunnel("dddownloadimage", tunnelId, params, endPoint/*, connectNow*/, bufferSize) {}
  public:
  /// @param cropUIConfig if not empty, enable crop UI after the download, and the config can be like "120x240"  
  void reconnectTo(const String& endPoint, const String& cropUIConfig = "");
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


/// Output struct of ObjectDetectDemoServiceDDTunnel
struct DDObjectDetectDemoResult {
  int left;
  int top;
  int right;
  int bottom;
  String label;
};
/// Class for "object detection demo" service "tunnel"
class ObjectDetectDemoServiceDDTunnel: public BasicDDTunnel {
  public:
    /// @attention constructed via DumbDisplay object
    ObjectDetectDemoServiceDDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize):
        BasicDDTunnel(type, tunnelId, params, endPoint/*, connectNow*/, bufferSize) {
    }
  public:
    void reconnectForObjectDetect(const String& imageName);
    void reconnectForObjectDetectFrom(GraphicalDDLayer* pGraphicalLayer, const String& imageName);
    bool readObjectDetectResult(DDObjectDetectDemoResult& objectDetectResult);  
};

struct DDImageData {
  DDImageData(): bytes(NULL) {};
  ~DDImageData();
  bool isValid() { return width > 0 && height > 0; }
  void release();
  int width;
  int height;
  int byteCount;
  uint8_t* bytes;
protected:
  void transferTo(DDImageData& imageData);
};
struct DDPixelImage: public DDImageData {
  void transferDataTo(DDPixelImage& pixelImage) { DDImageData::transferTo(pixelImage); }
};
struct DDPixelImage16 {
  DDPixelImage16(): data(NULL) {};
  ~DDPixelImage16();
  bool isValid() { return width > 0 && height > 0; }
  void transferTo(DDPixelImage16& pixelImage16);
  void release();
  int width;
  int height;
  int byteCount;
  uint16_t* data;
};
struct DDJpegImage: public DDImageData {
  void transferTo(DDJpegImage& jpegImage) { DDImageData::transferTo(jpegImage); }
};
/// Class service "tunnel" for retrieving image data (in format like JPEG / 565RGB) saved in DumbDisplay app storage via DumbDisplay::saveCachedImageFile(), DDLayer::saveImage() etc.
/// When "reconnect" to retrieve image data, the dimension, say the TFT screen dimension, will be passed as parameters.
/// Note that the image will be scaled down when needed.
/// To read the mage data retrieved, call readPixelImage(), readPixelImage16(), readJpegImage() etc; in case of detected corruption of the
/// data, the image width and height will be zeros.
/// @since v0.9.9-r3
class ImageRetrieverDDTunnel: public BasicDDTunnel {
  public:
    /// @attention constructed via DumbDisplay object
    ImageRetrieverDDTunnel(const String& type, int8_t tunnelId, const String& params, const String& endPoint/*, bool connectNow*/, int8_t bufferSize):
        BasicDDTunnel(type, tunnelId, params, endPoint/*, connectNow*/, bufferSize) {
    }
  public:
    /// reconnect to retrieve single-bit "pixel" image (i.e. B&W image)
    /// @param fit whether to fit the image to the given width and height, scaling up if necessary
    void reconnectForPixelImage(const String& imageName, int width, int height, bool fit = false);
    /// reconnect to retrieve 16-bit "pixel" image (i.e. 565 RGB image)
    /// @param grayscale whether to convert the image to to grayscale
    void reconnectForPixelImage16(const String& imageName, int width, int height, bool fit = false, bool grayscale = false);
    void reconnectForPixelImageGS(const String& imageName, int width, int height, bool fit = false);
    /// reconnect to retrieve JPEG image
    /// @param quality 0-100
    void reconnectForJpegImage(const String& imageName, int width, int height, int quality=100, bool fit = false);
    /// get single-bit image data retrieved with reconnectForPixelImage
    bool readPixelImage(DDPixelImage& pixelImage);  
    /// get 16-bit image data retrieved with reconnectForPixelImage16  
    bool readPixelImage16(DDPixelImage16& pixelImage16);  
    bool readPixelImageGS(DDPixelImage& pixelImage);
    /// get grayscale 16-bit image data retrieved with reconnectForPixelImageGS  
    bool readPixelImageGS16(DDPixelImage16& pixelImage16); 
    /// get JPEG image data retrieved with reconnectForJpegImage
    bool readJpegImage(DDJpegImage& jpeg);  
  private:  
    bool _readImageData(DDImageData& imageData, short type);  
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

#ifndef DD_NO_IDLE_CALLBACK
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
#endif

#ifndef DD_NO_CONNECT_VERSION_CHANGED_CALLBACK
/// @struct DDConnectVersionChangedCallback
/// @brief
/// Type signature for callback function that will be called when connect version (counting up) changed. See DumbDisplay::setConnectVersionChangedCallback()
typedef void (*DDConnectVersionChangedCallback)(int connectVersion);
#endif

#ifndef DD_NO_DEBUG_INTERFACE
/// @struct DDDebugConnectionState
/// @brief
/// See DDDebugInterface
enum DDDebugConnectionState { DEBUG_NOT_CONNECTED, DEBUG_CONNECTING, DEBUG_CONNECTED, DEBUG_RECONNECTING, DEBUG_RECONNECTED };
/// Base class for debug callback set by calling DumbDisplay::debugSetup()
class DDDebugInterface {
  public:
    /// See DDDebugConnectionState
    virtual void logConnectionState(DDDebugConnectionState connectionState) {}
    /// @param state 1: start sending; 0: stop sending
    virtual void logSendCommand(int state) {}
    virtual void logError(const String& errMsg) {}
};
#endif


#ifndef DD_NO_PASSIVE_CONNECT
/// Struct for the status values of calling DumbDisplay::connectPassive()
struct DDConnectPassiveStatus {
  /// connection made or not -- same as the return value of DumbDisplay::connectPassive() 
  bool connected;
  /// connecting: when not connected; starting to establish connection by sending hand-shake messages
  bool connecting;
  /// reconnecting: when connected; detected reconnecting (after lost of connection) 
  bool reconnecting;
};
#endif


extern bool _DDDisableParamEncoding;
inline void DDDebugDisableParamEncoding() { _DDDisableParamEncoding = true; }


/// @brief
/// The core class for DumbDisplay; everything starts here. The most important argument to DumbDisplay is an DDInputOutput object.
/// For an example, please refer to [Blink Test With Virtual Display, DumbDisplay](https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/)
class DumbDisplay {
  public:
    DumbDisplay(DDInputOutput* pIO, uint16_t sendBufferSize = DD_DEF_SEND_BUFFER_SIZE, long idleTimeout = DD_DEF_IDLE_TIMEOUT/*, bool enableDoubleClick = true*/) {
#ifdef DD_DISABLE_PARAM_ENCODING
    DDDebugDisableParamEncoding();
#endif      
#ifdef DD_NO_SERIAL      
  #warning ***** DD_NO_SERIAL defined *****
#else
  #ifdef DDIO_USE_DD_SERIAL
      if (pIO->isForSerial() || pIO->isBackupBySerial()) {
        _The_DD_Serial = new DDSerial();
      }
  #endif    
#endif      
      initialize(pIO, sendBufferSize, idleTimeout/*, enableDoubleClick*/);
    }
    /// explicitly make connection (blocking);
    /// implicitly called in situations like create a layer
    void connect();
    /// @return connected or not
    bool connected() const;
    /// @return the version of the connection, which when reconnected will be bumped up; note that if not connected, version will be 0
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
    /// @param layoutSpec the layout specification
    /// @param autoControlLayerVisible auto set layer visible (visibility) according whether the layer is specified in the layoutSpec or not; false by default
    void configAutoPin(const String& layoutSpec = DD_AP_VERT, bool autoControlLayerVisible = false);
    // /// in addition to DumbDisplay::configAutoPin(), also configure the remaining layout spec for the remaining layers not mentioned in the layoutSpec 
    // /// @see configAutoPin
    // /// @see addRemainingAutoPinConfig
    // void configAutoPinEx(const String& layoutSpec = DD_AP_VERT, const String& remainingLayoutSpec);
    /// add the "auto pin" config for layers not included in "auto pin" set by configAutoPin()
    void addRemainingAutoPinConfig(const String& remainingLayoutSpec);
    /// configure "pin frame" to be x-units by y-units (default 100x100)
    /// @param autoControlLayerVisible auto set layer visible (visibility) according whether the layer is pinned or not; false by default
    /// @see pinLayer()
    void configPinFrame(int xUnitCount = 100, int yUnitCount = 100, bool autoControlLayerVisible = false);
    /// pin a layer @ some position of an imaginary grid of "pin grame"
    /// - the imaginary grid size can be configured when calling connect() -- default is 100x100  
    /// - the input align (e.g. "LB") -- left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align
    void pinLayer(DDLayer *pLayer, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    /// pin "auto pin" layers @ some position, like pinLayer()
    /// @param layoutSpec the "auto pin" layout specification to pin; see configAutoPin() for how spec is constructed
    /// @param align (e.g. "LB") -- left align "L"; right align "R"; top align "T"; bottom align "B"; default is center align */
    void pinAutoPinLayers(const String& layoutSpec, int uLeft, int uTop, int uWidth, int uHeight, const String& align = "");
    /// rest pinning of layers, as if they are not pinned
    void resetPinLayers();
    /// experimental support of a "root" layer (GraphicalDDLayer) that contain all other created layers;
    /// note that the "root" will always be placed as the container, and hence don't need be pined;
    /// @param containedAlignment the alignment of the contained layers; "L" / "T" / "LT"; "" means centered 
    /// currently, "container" layer does not support "feedback"
    /// @since v0.9.9-r50
    GraphicalDDLayer* setRootLayer(int width, int height, const String& containedAlignment = "");
    /// create a Microbit-like layer
    MbDDLayer* createMicrobitLayer(int width = 5, int height = 5);
    /// create a Turtle-like layer
    TurtleDDLayer* createTurtleLayer(int width, int height);
    /// create a LED-grid layer
    LedGridDDLayer* createLedGridLayer(int colCount = 1, int rowCount = 1, int subColCount = 1, int subRowCount = 1);
    /// create a LCD layer
    LcdDDLayer* createLcdLayer(int colCount = 16, int rowCount = 2, int charHeight = 0, const String& fontName = "");
    /// create a "selection" layer
    SelectionDDLayer* createSelectionLayer(int colCount = 16, int rowCount = 2,
                                           int horiSelectionCount = 1, int vertSelectionCount = 1,
                                           int charHeight = 0, const String& fontName = "",
                                           bool canDrawDots = true, float selectionBorderSizeCharHeightFactor = 0.3);
    /// create a list "selection" layer
    SelectionListDDLayer* createSelectionListLayer(int colCount = 16, int rowCount = 2,
                                                   int horiSelectionCount = 1, int vertSelectionCount = 1,
                                                   int charHeight = 0, const String& fontName = "",
                                                   bool canDrawDots = true, float selectionBorderSizeCharHeightFactor = 0.3);
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
    ///                   use "+" combines the above like "lr+tb" to mean both directions; "" the same as "lr+tb" 
    /// @param stickSizeFactor the size factor of the stick (UI); 1 by default 
    /// @see JoystickDDLayer
    JoystickDDLayer* createJoystickLayer(int maxStickValue = 1023, const String& directions = "", float stickSizeFactor = 1.0/*, int stickValueDivider = 1*/);
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
    /// create a WebView layer
    /// @see WebViewDDLayer
    WebViewDDLayer* createWebViewLayer(int width, int height, const String& jsObjectName = "DD");
    /// create a DumbDisplay "window" layer
    /// @see DumbDisplayDDLayer
    DumbDisplayWindowDDLayer* createDumbDisplayWindowLayer(int width, int height);
    /// create a RTSP-client layer
    /// @see RtspClientDDLayer
    RtspClientDDLayer* createRtspClient(int width, int height);
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
    JsonDDTunnel* createFilteredJsonTunnel(const String& endPoint, const String& fieldNames, bool connectNow = true, int8_t bufferSize = DD_TUNNEL_DEF_BUFFER_SIZE);
    /// create a "tunnel" to download image from the web, and save the downloaded image to phone;
    /// you will get result in JSON format: ```{"result":"ok"}``` or ```{"result":"failed"}```
    /// for simplicity, use SimpleToolDDTunnel.checkResult() to check the result
    /// @see SimpleToolDDTunnel
    ImageDownloadDDTunnel* createImageDownloadTunnel(const String& endPoint, const String& imageName, bool redownload = true);
    /// @deprecated use createGeneralServiceTunnel() instead
    BasicDDTunnel* createDateTimeServiceTunnel();
    /// create a general "service tunnel" for purposes like getting date-time info from phone;
    /// use reconnectTo() with commands like
    /// - `now` -- e.g `now:yyyy-MM-dd-hh-mm-ss` where `yyyy-MM-dd-hh-mm-ss` is the format
    /// - `now-millis`
    /// - `confirm` -- pop up a dialog to confirm; e.g. `confirm?title=Confirm&message=Are%20you%20sure%3F&ok=Yes&cancel=No`
    /// @see BasicDDTunnel
    BasicDDTunnel* createGeneralServiceTunnel();
    /// create a "service tunnel" for getting GPS info from phone
    /// @see GpsServiceDDTunnel
    GpsServiceDDTunnel* createGpsServiceTunnel();
    /// create a "service tunnel" for getting object detection info from phone; model used is the demo model `mobilenetv1.tflite`
    /// @see ObjectDetectDemoServiceDDTunnel
    ObjectDetectDemoServiceDDTunnel* createObjectDetectDemoServiceTunnel(int scaleToWidth = 0, int scaleToHeight = 0, int maxNumObjs = 1);
    ImageRetrieverDDTunnel* createImageRetrieverTunnel();
    /// experimental: create a visual-only graphical LCD layer handle which you can be used where GraphicalDDLayerHandle is accepted, like construction of GraphicalDDLayer;
    /// @since v0.9.9-r50
    GraphicalDDLayerHandle createGraphicalLayerHandle(int width, int height);
    /// experimental: create a visual-only LCD layer handle which you can be used where LcdDDLayerHandle is accepted, like construction of LcdDDLayer;
    /// @since v0.9.9-r50
    LcdDDLayerHandle createLcdLayerHandle(int colCount = 16, int rowCount = 2, int charHeight = 0, const String& fontName = "");
    /// experimental: create a visual-only LED-grid layer handle which you can be used where LedGridDDLayerHandle is accepted, like construction of LedGridDDLayer;
    /// @since v0.9.9-r51
    LedGridDDLayerHandle createLedGridLayerHandle(int colCount = 1, int rowCount = 1, int subColCount = 1, int subRowCount = 1);
    /// if finished using a "tunnel", delete it to release resource
    void deleteTunnel(DDTunnel *pTunnel);
    /// set DD background color
    /// @param color DD_COLOR_XXX; DD_RGB_COLOR(...); can also be common "color name"
    void backgroundColor(const String& color);
    /// basically, functions the same as recordLayerCommands()
    void recordLayerSetupCommands();
    /// basically, functions the same as playbackLayerCommands().
    /// additionally:
    /// - save and persist the layer commands
    /// - enable DumbDisplay reconnect feature -- tells the layer setup commands to use when DumbDisplay reconnects
    void playbackLayerSetupCommands(const String& layerSetupPersistId);
    /// start recording layer commands (of any layers);
    /// and sort of freeze the display, until playback
    void recordLayerCommands();
    /// playback recorded commands (unfreeze the display)
    void playbackLayerCommands();
    /// stop recording commands (if not saved, will forget what recorded)
    /// @param saveId if provided, save like calling saveLayerCommands(); otherwise, forget what have been recording
    /// @param persistSave if save, store it to your phone as well
    void stopRecordLayerCommands(const String& saveId = "", bool persistSave = false);
    /// save the recorded commands (continue recording or not depends on parameter passed in );
    /// any pre-existed saved commands with the same id will be replaced with the recording commands (delete if not recording)
    /// @param id identifier of the recorded commands, overwriting and previous one;
    ///           if not recording, will delete previous recorded commands
    /// @param persist store it to your phone as well
    /// @param stopAfterSave stop recording after saving
    void saveLayerCommands(const String& id, bool persist = false, bool stopAfterSave = false);
    /// load saved commands (as if recording those commands)
    /// - recording started or not, will add the commands to the buffer
    /// - afterward, will keep recording
    /// - use playbackLayerCommands() to playback loaded commands
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
    /// save single-bit "pixel" image (i.e. B&W image) with the given image data
    void savePixelImage(const String& imageName, const uint8_t *bytes, int width, int height, const String& color = "", char compressMethod = 0);
    /// save 16-bit "pixel" image (i.e. 565 RGB image) with the given image data
    void savePixelImage16(const String& imageName, const uint16_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// save grayscale "pixel" image with the given image data
    void savePixelImageGS(const String& imageName, const uint8_t *data, int width, int height, const String& options = "", char compressMethod = 0);
    /// stitch images together
    /// @param imageNames '+' delimited
    /// @param asImageName name for the stitched image
    void stitchImages(const String& imageNames, const String& asImageName);
    /// pop up a dialog to alert
    void alert(const String& message, const String& title = "");
    /// reorder the layer (by moving one layer in the z-order plane)
    /// @param how  can be "T" for top; or "B" for bottom; "U" for up; or "D" for down
    void reorderLayer(DDLayer *pLayer, const String& how);
    /// if layer is no longer used; delete it to release resources
    void deleteLayer(DDLayer *pLayer);
    /// if layer is no longer used; delete it to release resources
    void deleteLayer(DDLayerHandle layerHandle);
    /// loop through all the existing layers calling the function passed in
    void walkLayers(void (*walker)(DDLayer *));
 #ifndef DD_NO_IDLE_CALLBACK
   /// set 'idle callback', which will be called repeatedly in 2 situations:
    /// - no connection response while connecting
    /// - detected no 'keep alive' signal (i.e. reconnecting)
    /// @param idleCallback the callback function; see DDIdleCallback
    void setIdleCallback(DDIdleCallback idleCallback); 
#endif
#ifndef DD_NO_CONNECT_VERSION_CHANGED_CALLBACK
    /// set callback when version changed (e.g. reconnected after disconnect)
    /// @param connectVersionChangedCallback the callback function; see DDConnectVersionChangedCallback
    void setConnectVersionChangedCallback(DDConnectVersionChangedCallback connectVersionChangedCallback);
#endif
    /// check if it is safe to print to Serial
    bool canPrintToSerial();
    /// log line to Serial; if it is not safe to output to Serial, will write comment with DumbDisplay::writeComment() instead
    void logToSerial(const String& logLine, bool force = false);
    /// like to Serial (if safe to do so); and if connected,  will log as comment to DD as well  
    void log(const String& logLine, bool isError = false);
 public:
 #ifndef DD_NO_PASSIVE_CONNECT
   /// @brief
    /// make connection passively; i.e. will not block, but will require continuous calling for making connection
    /// @return connection made or not (note that even if connection lost and requires reconnecting, it is still considered connected)
    /// @since 0.9.8-r1
    /// @note if you will not be making use of "passive" connection, you can disable it by defining DD_NO_PASSIVE_CONNECT in order to reduce footprint 
    bool connectPassive(DDConnectPassiveStatus* pStatus = NULL);
    /// "master reset" will:
    /// . disconnect from DD app (if connected)
    /// . delete all created layers and tunnels; hence, DO NOT use the pointers to them after "master reset"
    /// . DumbDisplay object will be just like at initial state; it will *not* be deleted
    /// @since 0.9.8-r1
    void masterReset();  
#endif 
  public:
    /// set debug use callback
    /// @param debugInterface a concrete implementation of DDDebugInterface 
#ifndef DD_NO_DEBUG_INTERFACE
    void debugSetup(DDDebugInterface *debugInterface);
#endif    
    void debugOnly(int i);
  private:
    void initialize(DDInputOutput* pIO, uint16_t sendBufferSize, long idleTimeout/*, bool enableDoubleClick*/);
    //bool canLogToSerial();
};

#include "_dd_misc.h"

#endif

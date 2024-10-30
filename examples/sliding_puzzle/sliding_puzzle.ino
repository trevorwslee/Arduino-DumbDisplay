/**
 * If BLUETOOTH is #defined, it uses ESP32 bluetooth connection
 * . BLUETOOTH is the name of the bluetooth device
 * If WIFI_SSID is #defined, it uses wifi connection
 * . once sketch running, connect to it with Serial Monitor to check for IP address
 * Otherwise, it uses USB connection (OTG) with the default 115200 baud
 * To run and see the result of this sketch, you will need two addition things:
 * . you will need to install Android DumbDisplay app from Play store
 *   https://play.google.com/store/apps/details?id=nobody.trevorlee.dumbdisplay
 * . although there are several ways for microcontroller board to establish connection
 *   with DumbDisplay app, here, the simple OTG USB connection is assume;
 *   hence, you will need an OTG adaptor cable for connecting your microcontroller board
 *   to your Android phone
 * . after uploading the sketch to your microcontroller board, plug the USB cable
 *   to the OTG adaptor connected to your Android phone
 * . open the DumbDisplay app and make connection to your microcontroller board via the USB cable;
 *   hopefully, the UI is obvious enough :)
 * . for more details on DumbDisplay Arduino Library, please refer to
 *   https://github.com/trevorwslee/Arduino-DumbDisplay#readme
 * There is a related blog post that you may want to take a look:
 * . https://www.instructables.com/Blink-Test-With-Virtual-Display-DumbDisplay/
 */


#include "dumbdisplay.h"

#if defined(BLUETOOTH)
  #include "esp32dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDBluetoothSerialIO(BLUETOOTH, true));
#elif defined(WIFI_SSID)
  #include "wifidumbdisplay.h"
  DumbDisplay dumbdisplay(new DDWiFiServerIO(WIFI_SSID, WIFI_PASSWORD));
#else
  #include "dumbdisplay.h"
  DumbDisplay dumbdisplay(new DDInputOutput());
#endif


// DDMasterResetPassiveConnectionHelper is for making "passive" connection; i.e. it can be reconnected after disconnect
DDMasterResetPassiveConnectionHelper pdd(dumbdisplay);

// the only layer for the board
GraphicalDDLayer* board;


const int BOARD_SIZE = 400;
const int TILE_COUNT = 4;   // the the sliding puzzle is 4x4; i.e. 16 tiles
const int TILE_SIZE = BOARD_SIZE / TILE_COUNT;

// tells what levelId is at what tile position
String boardTileLevelIds[TILE_COUNT][TILE_COUNT];

long waitingToRestartMillis = -1;  // -1 means not waiting

int holeTileColIdx;  // -1 means board not initialize
int holeTileRowIdx;

int randomizeCanFromDirs[4];
long randomizeMoveTileInMillis;
int initRandomizeTileStepCount;
int randomizeTilesStepCount;
int randomizeCanFromDir;

int moveTileColIdx;
int moveTileRowIdx;
int moveTileFromDir;
int moveTileDelta;
int moveTileRefX;
int moveTileRefY;
String moveTileLevelId;


// show / hide the tile at tile position (0, 0) ... i.e. the "hold" tile when the board is just initialized
void showHideTileAt00(bool show) {
  String holeTileLevelId = boardTileLevelIds[0][0];
  board->switchLevel(holeTileLevelId);
  board->setLevelAnchor(0, 0);
  board->levelTransparent(!show);// levelOpacity(0);}
}


void initializeBoard() {
  dumbdisplay.log("Creating board ...");

  // export what has been draw as an image named "boardimg"
  board->exportLevelsAsImage("boardimg", true);
  
  board->clear();

  // add a "ref" level and draw the exported image "boardimg" on it (as reference)
  board->addLevel("ref", true);
  board->levelOpacity(5);
  board->drawImageFile("boardimg");
  
  for (int rowTileIdx = 0; rowTileIdx < TILE_COUNT; rowTileIdx++) {
    for (int colTileIdx = 0; colTileIdx < TILE_COUNT; colTileIdx++) {
      int tileIdx = colTileIdx + rowTileIdx * TILE_COUNT;

      // imageName refers to a tile of the image "boardimg"; e.g. "0!4x4@boardimg" refers to the 0th tile of a 4x4 image named "boardimg"
      String imageName = String(tileIdx) + "!" + String(TILE_COUNT) + "x" + String(TILE_COUNT) + "@boardimg";
      
      String tileLevelId = String(tileIdx);
      int x = colTileIdx * TILE_SIZE;
      int y = rowTileIdx * TILE_SIZE;

      // add a level that represents a tile ... and switch to it
      board->addLevel(tileLevelId, TILE_SIZE, TILE_SIZE, true);

      // the the tile anchor of the level to the tile position on the board
      board->setLevelAnchor(x, y);

      // set the back of the level to the tile image, with board around (b:3-gray-round)
      board->setLevelBackground("", imageName, "b:3-gray-round");
      
      boardTileLevelIds[colTileIdx][rowTileIdx] = tileLevelId;
    }
  }

  // reorder the "ref" level to the bottom, so that it will be drawn underneath the tiles
  board->reorderLevel("ref", "B");

  holeTileColIdx = 0;
  holeTileRowIdx = 0;
  moveTileColIdx = -1;
  moveTileRowIdx = -1;
  randomizeMoveTileInMillis = 300;
  initRandomizeTileStepCount = 5;

  dumbdisplay.log("... done creating board");
}

void randomizeTilesStep() {
  int canCount = 0;
  if (holeTileColIdx > 0 && randomizeCanFromDir != 1) {
    randomizeCanFromDirs[canCount++] = 0;  // 0: left
  }
  if (holeTileColIdx < (TILE_COUNT - 1) && randomizeCanFromDir != 0) {
    randomizeCanFromDirs[canCount++] = 1;  // 1: right
  }
  if (holeTileRowIdx > 0 && randomizeCanFromDir != 3) {
    randomizeCanFromDirs[canCount++] = 2;  // 2: up
  }
  if (holeTileRowIdx < (TILE_COUNT - 1) && randomizeCanFromDir != 2) {
    randomizeCanFromDirs[canCount++] = 3;  // 3: down
  }
  randomizeCanFromDir = randomizeCanFromDirs[random(canCount)];
  int fromColIdx;
  int fromRowIdx;
  if (randomizeCanFromDir == 0) {
    fromColIdx = holeTileColIdx - 1;
    fromRowIdx = holeTileRowIdx;
  } else if (randomizeCanFromDir == 1) {
    fromColIdx = holeTileColIdx + 1;
    fromRowIdx = holeTileRowIdx;
  } else if (randomizeCanFromDir == 2) {
    fromColIdx = holeTileColIdx;
    fromRowIdx = holeTileRowIdx - 1;
  } else {
    fromColIdx = holeTileColIdx;
    fromRowIdx = holeTileRowIdx + 1;
  }
  int toColIdx = holeTileColIdx;
  int toRowIdx = holeTileRowIdx;
  String fromTileLevelId = boardTileLevelIds[fromColIdx][fromRowIdx];
  boardTileLevelIds[fromColIdx][fromRowIdx] = boardTileLevelIds[holeTileColIdx][holeTileRowIdx];
  boardTileLevelIds[holeTileColIdx][holeTileRowIdx] = fromTileLevelId;
  board->switchLevel(fromTileLevelId);
  int x = toColIdx * TILE_SIZE;
  int y = toRowIdx * TILE_SIZE;

  // move the anchor of the level to the destination in randomizeMoveTileInMillis
  board->setLevelAnchor(x, y, randomizeMoveTileInMillis);
  
  holeTileColIdx = fromColIdx;
  holeTileRowIdx = fromRowIdx;

  // since the tile will be moved to the destination in randomizeMoveTileInMillis, delay randomizeMoveTileInMillis here
  delay(randomizeMoveTileInMillis); 
  
  // make sure the tile is at the destination
  board->setLevelAnchor(x, y);
}

void ensureBoardInitialized() {
  if (holeTileColIdx == -1) {
    initializeBoard();
  }
}

void startRandomizeBoard() {
  showHideTileAt00(false);
  randomizeTilesStepCount = initRandomizeTileStepCount;
  randomizeCanFromDir = -1;
}

int posToHoleTileFromDir(int x, int y) {
  if (y >= holeTileRowIdx * TILE_SIZE && y < (holeTileRowIdx + 1) * TILE_SIZE) {
    if (x < holeTileColIdx * TILE_SIZE) {
      if (x < (holeTileColIdx - 1) * TILE_SIZE) {
        return -1;
      } else {
        return 0 ;  // left
      }
    }
    if (x >= (holeTileColIdx + 1) * TILE_SIZE) {
      if (x >= (holeTileColIdx + 2) * TILE_SIZE) {
        return -1;
      } else {
        return 1;  // right
      }
    }
  }
  if (x >= holeTileColIdx * TILE_SIZE && x < (holeTileColIdx + 1) * TILE_SIZE) {
    if (y < holeTileRowIdx * TILE_SIZE) {
      if (y < (holeTileRowIdx - 1) * TILE_SIZE) {
        return -1;
      } else {
        return 2;  // up
      }
    }
    if (y >= (holeTileRowIdx + 1) * TILE_SIZE) {
      if (y >= (holeTileRowIdx + 2) * TILE_SIZE) {
        return -1;
      } else {
        return 3;  // down
      }
    }
  }
  return -1;
}

bool posToHoleTileFromIdxes(int x, int y, int& colIdx, int& rowIdx, int& fromDir) {
  colIdx = -1;
  rowIdx = -1;
  fromDir = posToHoleTileFromDir(x, y);
  if (fromDir == -1) {
    return false;
  }
  if (fromDir == 0) {
    colIdx = holeTileColIdx - 1;
    rowIdx = holeTileRowIdx;
  } else if (fromDir == 1) {
    colIdx = holeTileColIdx + 1;
    rowIdx = holeTileRowIdx;
  } else if (fromDir == 2) {
    colIdx = holeTileColIdx;
    rowIdx = holeTileRowIdx - 1;
  } else {
    colIdx = holeTileColIdx;
    rowIdx = holeTileRowIdx + 1;
  }
  return true;
}

bool onBoardDragged(int x, int y) {
  bool tileMoved = false;
  if (x != -1 && y != -1) {
    if (moveTileColIdx == -1) {
      int colIdx;
      int rowIdx;
      int fromDir;
      if (posToHoleTileFromIdxes(x, y, colIdx, rowIdx, fromDir)) {
        moveTileColIdx = colIdx;
        moveTileRowIdx = rowIdx;
        moveTileFromDir = fromDir;
        moveTileDelta = 0;
        moveTileRefX = x;
        moveTileRefY = y;
        moveTileLevelId = boardTileLevelIds[moveTileColIdx][moveTileRowIdx];
      }
    } else {
      int tileAnchorX = moveTileColIdx * TILE_SIZE;
      int tileAnchorY = moveTileRowIdx * TILE_SIZE;
      int delta;
      if (moveTileFromDir == 0) {
        delta = x - moveTileRefX;
        if (delta > 0) {
          if (delta > TILE_SIZE) {
            delta = TILE_SIZE;
          }
          tileAnchorX += delta;
        }
      } else if (moveTileFromDir == 1) {
        delta = moveTileRefX - x;
        if (delta > 0) {
          if (delta > TILE_SIZE) {
            delta = TILE_SIZE;
          }
          tileAnchorX -= delta;
        }
      } else if (moveTileFromDir == 2) {
        delta = y - moveTileRefY;
        if (delta > 0) {
          if (delta > TILE_SIZE) {
            delta = TILE_SIZE;
          }
          tileAnchorY += delta;
        }
      } else {
        delta = moveTileRefY - y;
        if (delta > 0) {
          if (delta > TILE_SIZE) {
            delta = TILE_SIZE;
          }
          tileAnchorY -= delta;
        }
      }
      board->switchLevel(moveTileLevelId);
      board->setLevelAnchor(tileAnchorX, tileAnchorY);
      moveTileDelta = delta;
    }
  } else {
    if (moveTileColIdx != -1) {
      int tileAnchorX;
      int tileAnchorY;
      if (moveTileDelta >= TILE_SIZE / 3) {
        tileAnchorX = holeTileColIdx * TILE_SIZE;
        tileAnchorY = holeTileRowIdx * TILE_SIZE;
        String prevHoleTileLevelId = boardTileLevelIds[holeTileColIdx][holeTileRowIdx];
        boardTileLevelIds[holeTileColIdx][holeTileRowIdx] = boardTileLevelIds[moveTileColIdx][moveTileRowIdx];
        boardTileLevelIds[moveTileColIdx][moveTileRowIdx] = prevHoleTileLevelId;
        holeTileColIdx = moveTileColIdx;
        holeTileRowIdx = moveTileRowIdx;
      } else {
        tileAnchorX = moveTileColIdx * TILE_SIZE;
        tileAnchorY = moveTileRowIdx * TILE_SIZE;
      }
      board->switchLevel(moveTileLevelId);
      board->setLevelAnchor(tileAnchorX, tileAnchorY);
      tileMoved = true;
    }
    moveTileColIdx = -1;
    moveTileRowIdx = -1;
  }
  return tileMoved;
}

bool checkBoardSolved() {
  for (int rowTileIdx = 0; rowTileIdx < TILE_COUNT; rowTileIdx++) {
    for (int colTileIdx = 0; colTileIdx < TILE_COUNT; colTileIdx++) {
      int tileIdx = colTileIdx + rowTileIdx * TILE_COUNT;
      String tileLevelId = boardTileLevelIds[colTileIdx][rowTileIdx];
      if (tileLevelId != String(tileIdx)) {
        return false;
      }
    }
  }
  return true;
}


void initializeDD() {
  board = dumbdisplay.createGraphicalLayer(BOARD_SIZE, BOARD_SIZE);
  board->backgroundColor("teal");
  board->border(8, "navy", "round", 5);
  board->drawRect(0, 0, BOARD_SIZE, BOARD_SIZE, "azure", true);
  board->drawRoundRect(20, 20, BOARD_SIZE - 40, BOARD_SIZE - 40, 10, "aqua", true);

  board->drawImageFileFit("dumbdisplay.png");
  board->setTextFont("DL::Roboto");
  board->drawTextLine("In God We Trust", 34, "C", "white", "purple", 32);
  board->drawTextLine("May God bless you❤️", 340, "R-20", "purple", "", 20);

  holeTileColIdx = -1;
  holeTileRowIdx = -1;
  randomizeTilesStepCount = 0;

  board->enableFeedback();

  if (false) {
    ensureBoardInitialized();
  }

  waitingToRestartMillis = 0;
}
void updateDD(bool isFirstUpdate) {

  if (waitingToRestartMillis != -1) {
    // starts off waiting for double tab
    long nowMillis = millis();
    long diffMillis = nowMillis - waitingToRestartMillis;
    if (diffMillis > 5000) {
      dumbdisplay.log("! double tab to start !");
      waitingToRestartMillis = nowMillis;
    }
  }

  const DDFeedback* fb = board->getFeedback();

  if (randomizeTilesStepCount > 0) {
    // randomizing the board
    randomizeTilesStep();
    randomizeTilesStepCount--;
    if (randomizeTilesStepCount == 0) {
      // randomization is done
      dumbdisplay.log("... done randomizing board");
      board->enableFeedback(":drag");  // :drag to allow dragging that produces MOVE feedback type (and ended with -1, -1 MOVE feedback)
    }
  } else {
    if (fb != NULL) {
      if (fb->type == DOUBLECLICK) {
        // double click ==> randomize the board, even during play
        board->flash();
        board->disableFeedback();
        ensureBoardInitialized();
        dumbdisplay.log("Randomizing board ...");
        waitingToRestartMillis = -1;
        startRandomizeBoard();
        return;
      } else if (fb->type == MOVE) {
        // dragging / moving a tile ... handle it in onBoardDragged
        if (onBoardDragged(fb->x, fb->y)) {
          // ended up moving a tile ... check if the board is solved
          if (checkBoardSolved()) {
            showHideTileAt00(true);
            dumbdisplay.log("***** Board Solved *****");
            board->enableFeedback();
            showHideTileAt00(true);
            delay(200);
            showHideTileAt00(false);
            delay(200);
            showHideTileAt00(true);
            randomizeMoveTileInMillis -= 50;  // randomize faster and faster
            if (randomizeMoveTileInMillis < 100) {
              randomizeMoveTileInMillis = 100;
            }
            initRandomizeTileStepCount += 5;  // randomize more and more
            if (initRandomizeTileStepCount > 50) {
              initRandomizeTileStepCount = 50;
            }
            waitingToRestartMillis = 0;
          }
        }
      }
    }
  }
}

void disconnectedDD() {
}

void handleIdle(bool justBecameIdle) {
}



void setup() {
}

void loop() {
  // standard way of using pdd  
  pdd.loop([](){
    // **********
    // *** initializeCallback ***
    // **********
    initializeDD();
  }, [](){
    // **********
    // *** updateCallback ***
    // **********
    updateDD(!pdd.firstUpdated());
  }, [](){
    // **********
    // *** disconnectedCallback ***
    // **********
    disconnectedDD();
  });
  if (pdd.isIdle()) {
    handleIdle(pdd.justBecameIdle());
  }
}
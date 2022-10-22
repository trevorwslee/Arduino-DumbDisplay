

#include "PressTracker.h"
#include "BasicCalculator.h"


const int8_t MS_MENU = 0;
const int8_t MS_CALC = 1;
const int8_t MS_STOP = 2;

int8_t dms = -1;
int32_t dss = -1;

void resetDisplayState() {
  dms = -1;
  dss = -1;
}
boolean checkDisplayState(int8_t mainState, int32_t subState) {
  int8_t oridms = dms;
  int32_t oridss = dss;
  dms = mainState;
  dss = subState;
  return oridms != dms || oridss != dss;
}

ButtonPressTracker selectTracker;
JoyStickPressTracker horizontalTracker;
JoyStickPressTracker verticalTracker;
PrimitiveCalculator calculator(6);




////////////////////////////////


byte fase = 0;

void _drawCalc() {
  display->clear();
  //display.clearDisplay();
  display->drawRoundRect(0, 0, 64, 128, 3, COLOR_1);
  display->fillRoundRect(6, 16, 52, 16, 2, COLOR_1);

  display->setCursor(6, 2);
  display->print("CALC");

  display->setTextColor(COLOR_0);
  for (int i = 0; i < n; i++) {
    posY[i] = fromTop + (boxH * i) + (space * i);
    for (int j = 0; j < m; j++) {
      posX[j] = fromLeft + (boxW * j) + (space * j);
      display->fillRoundRect(posX[j], posY[i], boxW, boxH, 2, COLOR_1);
      display->setCursor(posX[j] + (boxW / 2) - 3, posY[i] + (boxH / 2) - 6);
      //display->setTextColor(COLOR_0);
      display->print(String(buttons[j][i]));
    }
  }
  display->setTextColor(COLOR_1);
  display->fillRoundRect(posX[cx], posY[cy], boxW, boxH, 2, COLOR_0);
  display->drawRoundRect(posX[cx], posY[cy], boxW, boxH, 2, COLOR_1);
  display->setCursor(posX[cx] + (boxW / 2) - 3, posY[cy] + (boxH / 2) - 6);
  display->print(String(buttons[cx][cy]));

  // display->setCursor(7, 3);
  // display->print("CALC");
  display->setCursor(10, 18);
  display->setTextColor(COLOR_0);
  const char* formatted = calculator.getFormatted();
  display->print(String(formatted));
  //display->setTextColor(COLOR_1);
  display->setTextColor(COLOR_DEF);
}

void checkButtonsCalc() {
  int8_t horizontalPress = horizontalTracker.setReading(analogRead(horizontal));
  int8_t verticalPress = verticalTracker.setReading(analogRead(vertical));

  if (/*upTracker.setPressed(digitalRead(up) == 0) || */verticalPress == 1) {
      cy = (cy + 3) % 4;
  } else if (/*downTracker.setPressed(digitalRead(down) == 0) || */verticalPress == -1) {
      cy = (cy + 1) % 4;
  } else if (/*rightTracker.setPressed(digitalRead(right) == 0) || */horizontalPress == 1) {
      cx = (cx + 1) % 4;
  } else if (/*leftTracker.setPressed(digitalRead(left) == 0) || */horizontalPress == -1) {
      cx = (cx + 3) % 4;
  }

  if (selectTracker.setPressed(digitalRead(presS) == 0)) {
    char what = buttons[cx][cy];
    if (what == 'C') {
      calculator.reset();
    } else {
      if (!calculator.push(what)) {
        dumbdisplay.tone(/*9, */2100, 50);
      }
    }
    resetDisplayState();
  }
}

void _drawStop() {
  
  //display.setFont();
  display->clear();
  display->setTextColor(COLOR_1);
  display->setCursor(0, 0);
  display->print("STOPWATCH");

  char buffer[10];

  display->setCursor(0, 20);
  display->setTextSize(TEXT_SIZE_2);
  sprintf(buffer, "%02d", s_min);
  display->print(buffer);

  display->setCursor(24, 20);
  display->print(String(":"));
  display->setCursor(34, 20);
  sprintf(buffer, "%02d", s_sec);
  display->print(buffer);

  display->setTextSize(TEXT_SIZE_4);
  display->setCursor(6, 64);
  sprintf(buffer, "%d00", s_10_sec);
  display->print(buffer);

  //display.display();

  // if (s_fase == 1) {
  //   s_milis = s_milis + 3.5;

  //   if (s_milis > 99) {
  //     s_sec++;
  //     s_milis = 0;
  //   }

  //   if (s_sec > 59) {
  //     s_min++;
  //     s_sec = 0;
  //   }
  // }
  //display->setTextSize(0);
  display->setTextSize(TEXT_SIZE_DEF);
}

void checkButtonsStop() {
  if (selectTracker.setPressed(digitalRead(presS) == 0)) {
    s_fase++;
    if (s_fase == 3) {
      s_fase = 0;
      //s_milis = 0;
      s_sec = 0;
      s_min = 0;
      s_start = -1;
    }
  }
}

void _drawGame() {

  if (!g_shown) {
    display->clear();

    display->setCursor(2, 0);
    display->print("Score:");

    display->drawLine(0, 9, 0, 127, COLOR_1);
    display->drawLine(63, 9, 63, 127, COLOR_1);
    display->drawLine(0, 9, 63, 9, COLOR_1);
    //display->fillRect(playerX, 118, playerW, 2, COLOR_1);
    display->fillCircle(ballX, ballY, 1, COLOR_1);

    for (int i = 0; i < 14; i++)
      if (enL[i] == 1)
        display->fillRect(enX[i], enY[i], 8, 2, COLOR_1);

    g_shown = true;    
  }

  display->fillRect(1, 118, 62, 2, COLOR_BG);
  display->fillRect(playerX, 118, playerW, 2, COLOR_1);

  display->setCursor(40, 0);
  display->print(String(gameScore));

  //display.display();
}

void checkButtonsGame() {

  // if (digitalRead(presS) == 0) {
  //   controler = !controler;
  //   digitalWrite(3, controler);
  // }


  // if (controler == 1) {
  //   playerX = map(analogRead(horizontal/*A0*/), 0, 1023, 1, 63 - playerW);
  // }
  playerX = map(analogRead(horizontal/*A0*/), 0, 1023, 1, 63 - playerW);

  // if (controler == 0) {
  //   if (digitalRead(down) == 0)
  //     if (playerX > 1)
  //       playerX--;

  //   if (digitalRead(up) == 0)
  //     if (playerX < 62 - playerW)
  //       playerX++;
  // }
}

void GameReset() {
  ballX = random(10, 50);
  ballY = 50;
  ballDirectionX = 1;
  ballDirectionY = 1;
  gameScore = 0;
  for (int i = 0; i < 14; i++)
    enL[i] = 1;
  g_shown = false;  
}

void gameOver() {
  display->clear();
  display->setCursor(6, 20);
  display->print(String(gameScore));
  display->setTextSize(2);
  display->setCursor(6, 40);
  display->print("GAME");
  display->setCursor(6, 60);
  display->print("OVER");
  display->setTextSize(0);
  //display.display();
  delay(3000);
  GameReset();
}

void checkColision() {
  if (ballX < 4 || ballX > 62)
    ballDirectionX = ballDirectionX * -1;
  if (ballY < 10)
    ballDirectionY = ballDirectionY * -1;

  if (ballY > 116 && ballX > playerX && ballX < playerX + playerW)
    ballDirectionY = ballDirectionY * -1;

  for (int i = 0; i < 14; i++)
    if (ballX > enX[i] && ballX < enX[i] + 8 && ballY > enY[i] && ballY < enY[i] + 2 && enL[i] == 1) {
      ballDirectionY = ballDirectionY * -1;
      enL[i] = 0;
      gameScore++;
    }

  ballX = ballX + ballDirectionX;
  ballY = ballY + ballDirectionY;

  if (ballY > 124)
    gameOver();


  if (gameScore % 14 == 0 && gameScore != 0)
    GameReset();
}

void calendarDraw() {
  display->setRotation(0);
  display->clear();
  display->setCursor(0, 0);
  display->print("Mo Tu We Th Fr Sa Su");
  int d = 1;
  int dd = 1;
  for (int i = 0; i < 6; i++)
    for (int j = 0; j < 7; j++) {
      if (dd <= dayS[chosenMonth] && d >= startDay[chosenMonth]) {
        display->setCursor(j * 18, (i + 1) * 9);
        display->print(String(dd));
        dd++;
      }
      d++;
    }
  display->setCursor(70, 55);
  display->print(String(chosenMonth + 1));
  display->print("/2022");

  //display.display();
}

void checkButtonsCalendar() {
  if (digitalRead(up) == 0) {
    if (chosenMonth > 0)
      chosenMonth--;
  }

  if (digitalRead(down) == 0) {
    if (chosenMonth < 11)
      chosenMonth++;
  };
}

void _drawMenu() {
  display->clear();

  display->setTextSize(TEXT_SIZE_DEF);

  display->setCursor(34, 92/*94*/);
  display->print("Beep");
  display->setCursor(38, 104);
  if (sounds == 1)
    display->print("ON");
  else
    display->print("OFF");

  display->setCursor(2/*0*/, 116/*120*/);
  if (chosenMenu == 0)
    display->print("Calculator");
  if (chosenMenu == 1)
    display->print("Stopwatch");
  if (chosenMenu == 2)
    display->print("Games");
  if (chosenMenu == 3)
    display->print("Calendar");
  if (chosenMenu == 4)
    display->print("PhoneBook");

  display->drawImageFile("logo.png", 0, 0);
  display->drawImageFile("calc.png", menuX[0], menuY[0]);
  display->drawImageFile("stop.png", menuX[1], menuY[1]);
  display->drawImageFile("game.png", menuX[2], menuY[2]);
  display->drawImageFile("calen.png", menuX[3], menuY[3]);
  display->drawImageFile("phone.png", menuX[4], menuY[4]);

  display->drawRoundRect(menuX[chosenMenu] - 2, menuY[chosenMenu] - 2, 28, 28, 2, COLOR_1);
  //display.display();
}

void checkButtonsMenu() {
  //dumbdisplay.writeComment(String(analogRead(horizontal)));
  int8_t horizontalPress = horizontalTracker.setReading(analogRead(horizontal));
  int8_t verticalPress = verticalTracker.setReading(analogRead(vertical));

  if (/*upTracker.setPressed(digitalRead(up) == 0) || 
      leftTracker.setPressed(digitalRead(left) == 0) ||
      */horizontalPress == -1 || verticalPress == 1) {
    if (sounds == 1) dumbdisplay.tone(/*9, */1100, 50);
    chosenMenu = (chosenMenu + 5) % 6;
  } else if (/*downTracker.setPressed(digitalRead(down) == 0) ||
      rightTracker.setPressed(digitalRead(right) == 0) ||
      */horizontalPress == 1 || verticalPress == -1) {
    if (sounds == 1) dumbdisplay.tone(/*9, */1100, 50);
    chosenMenu = (chosenMenu + 1) % 6;
  }

  if (selectTracker.setPressed(digitalRead(presS) == 0)) {
    if (chosenMenu == 5)
      sounds = !sounds;
    else
      fase = chosenMenu + 1;
    if (sounds == 1) dumbdisplay.tone(/*9, */1100, 50);
    delay(400);
    resetDisplayState();
  }
}

void resetAll() {  //display.setFont();
  cy = 0;
  cy = 0;
  n1 = 0;
  n2 = 0;
  //num = 0;
  //digit = 0;
  //operation = 0;
  g_shown = false;
}

void phoneDraw() {
  display->clear();
  display->setCursor(0, 4);
  display->print("Mike");
  display->setCursor(0, 14);
  display->print("0436789");
  display->drawLine(0, 1, 63, 1, COLOR_1);

  display->setCursor(0, 28);
  display->print("Betty");
  display->setCursor(0, 38);
  display->print("6574834");
  display->drawLine(0, 25, 63, 25, COLOR_1);

  display->setCursor(0, 52);
  display->print("JohnConor");
  display->setCursor(0, 62);
  display->print("6453722");
  display->drawLine(0, 49, 63, 49, COLOR_1);

  display->setCursor(0, 76);
  display->print("Luke");
  display->setCursor(0, 86);
  display->print("2275849");
  display->drawLine(0, 73, 63, 73, COLOR_1);

  //display.display();
}



////////////////////



void drawMenu() {
  if (!checkDisplayState(MS_MENU, chosenMenu)) {
    return;
  }
  dumbdisplay.recordLayerCommands();
  _drawMenu();
  dumbdisplay.playbackLayerCommands();
}
void drawCalc() {
  int32_t ss = 10 * cx + cy;
  if (!checkDisplayState(MS_CALC, ss)) {
    return;
  }
  dumbdisplay.recordLayerCommands();
  _drawCalc();
  dumbdisplay.playbackLayerCommands();
}
void drawStop() {
  if (s_fase == 1) {
    long now = millis();
    if (s_start != -1) {
      long diff = now - s_start;
      long diff_10_sec = diff / 100;
      if (diff_10_sec > 0) {
        long new_10_sec = s_10_sec + diff_10_sec;
        s_10_sec = new_10_sec % 10;
        long new_sec = s_sec + new_10_sec / 10;
        //long new_sec = s_sec + diff_sec;
        s_sec = new_sec % 60;
        long inc_min = new_sec / 60;
        s_min = (s_min + inc_min) % 60; 
        s_start = now;
      }
    } else {
      s_start = now;
    }
  }
  int32_t ss = 10 * (60 * s_min + s_sec) + (s_10_sec / 2);  // s_10_sec divide by 2 to make it refresh less frequent
  if (!checkDisplayState(MS_STOP, ss)) {
    return;
  }
  dumbdisplay.recordLayerCommands();
  _drawStop();
  dumbdisplay.playbackLayerCommands();
}
void drawGame() {
  int32_t ss = (100 * playerX + gameScore) + (gameOver ? 10000 : 0);
  if (!checkDisplayState(MS_STOP, ss)) {
    return;
  }
  dumbdisplay.recordLayerCommands();
  _drawGame();
  dumbdisplay.playbackLayerCommands();
}


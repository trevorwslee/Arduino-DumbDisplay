

#include "PressTracker.h"

// const uint8_t up PIN_A1;
// const uint8_t down PIN_A4;
// const uint8_t presS PIN_A0;


// pins
// const int a_btn = 2;
// const int c_btn = 4;
// const int d_btn = 5;
// const int b_btn = 3;
// const int e_btn = 6;
// const int f_btn = 7;
// const int k_btn = 8;
// const int x_joystick = A0;
// const int y_joystick = A1;


const uint8_t left = 5;
const uint8_t up = 2;
const uint8_t right = 3;
const uint8_t down = 4;

const uint8_t horizontal = A0;
const uint8_t vertical = A1;


//const uint8_t presS = 8;
const uint8_t analogy = A0;




const int8_t MS_MENU = 0;
const int8_t MS_CALC = 1;

int8_t dms = -1;
int8_t dss = -1;

void resetDisplayState() {
  dms = -1;
  dss = -1;
}
boolean checkDisplayState(int8_t mainState, int8_t subState) {
  int8_t oridms = dms;
  int8_t oridss = dss;
  dms = mainState;
  dss = subState;
  return oridms != dms || oridss != dss;
}

//ButtonPressTracker leftTracker;
//ButtonPressTracker upTracker;
ButtonPressTracker rightTracker;
//ButtonPressTracker downTracker;
//ButtonPressTracker selectTracker;

JoyStickPressTracker horizontalTracker;
JoyStickPressTracker verticalTracker;






byte fase = 0;


void drawCalc() {
  int8_t ss = 10 * cx + cy;
  if (!checkDisplayState(MS_CALC, ss)) {
    return;
  }

  display->clear();
  //display.clearDisplay();
  display->drawRoundRect(0, 0, 64, 128, 3, COLOR_1);
  display->fillRoundRect(6, 16, 52, 16, 2, COLOR_1);

  display->setCursor(6, 4);
  display->print("CALC");



  for (int i = 0; i < n; i++) {
    posY[i] = fromTop + (boxH * i) + (space * i);
    for (int j = 0; j < m; j++) {
      posX[j] = fromLeft + (boxW * j) + (space * j);
      display->fillRoundRect(posX[j], posY[i], boxW, boxH, 2, COLOR_1);
      display->setCursor(posX[j] + (boxW / 2) - 3, posY[i] + (boxH / 2) - 3);
      display->setTextColor(COLOR_0);
      display->print(String(buttons[j][i]));
    }
  }
  display->setTextColor(COLOR_1);
  display->fillRoundRect(posX[cx], posY[cy], boxW, boxH, 2, COLOR_0);
  display->drawRoundRect(posX[cx], posY[cy], boxW, boxH, 2, COLOR_1);
  display->setCursor(posX[cx] + (boxW / 2) - 2, posY[cy] + (boxH / 2) - 4);
  display->print(String(buttons[cx][cy]));

  display->setCursor(6, 4);
  display->print("CALC");
  display->setCursor(10, 20);
  display->setTextColor(COLOR_0);

  temp = num * 10;
  if (temp % 10 == 0)
    display->print(String((int)num));
  else
    display->print(String(num));
  //display->display();
  display->setTextColor(COLOR_1);
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

  if (rightTracker/*selectTracker*/.setPressed(digitalRead(right/*presS*/) == 0)) {

    if (buttons[cx][cy] == '0' || buttons[cx][cy] == '1' || buttons[cx][cy] == '2' || buttons[cx][cy] == '3' || buttons[cx][cy] == '4' || buttons[cx][cy] == '5' || buttons[cx][cy] == '6' || buttons[cx][cy] == '7' || buttons[cx][cy] == '8' || buttons[cx][cy] == '9' || buttons[cx][cy] == '.') {
      num = num * (digit * 10) + buttons[cx][cy] - '0';
      digit = 1;
    }

    if (buttons[cx][cy] == 'C') {
      num = 0;
      cx = 0;
      cy = 0;
      operation = 0;
    }

    if (buttons[cx][cy] == '+') {
      operation = 1;
      n1 = num;
      num = 0;
    }
    if (buttons[cx][cy] == '-') {
      operation = 2;
      n1 = num;
      num = 0;
    }
    if (buttons[cx][cy] == '*') {
      operation = 3;
      n1 = num;
      num = 0;
    }
    if (buttons[cx][cy] == '/') {
      operation = 4;
      n1 = num;
      num = 0;
    }

    if (buttons[cx][cy] == '=') {

      if (operation == 1) {
        float r = n1 + num;
        num = r;
        n1 = num;
      }

      if (operation == 2) {
        float r = n1 - num;
        num = r;
        n1 = num;
      }

      if (operation == 3) {
        float r = n1 * num;
        num = r;
        n1 = num;
      }

      if (operation == 4) {
        float r = n1 / num;
        num = r;
        n1 = num;
      }

      delay(200);
    }

    resetDisplayState();
  }
}

void drawStop() {
  //display.setFont();
  display->clear();
  display->setTextColor(COLOR_1);
  display->setCursor(0, 0);
  display->print("STOPWATCH");





  display->setCursor(0, 20);
  display->setTextSize(TEXT_SIZE_2);
  display->print(String(s_min));
  display->setCursor(24, 20);
  display->print(String(":"));
  display->setCursor(34, 20);
  display->print(String(s_sec));

  display->setTextSize(TEXT_SIZE_4);
  display->setCursor(6, 64);
  display->print(String((int)s_milis));



  //display.display();

  if (s_fase == 1) {
    s_milis = s_milis + 3.5;

    if (s_milis > 99) {
      s_sec++;
      s_milis = 0;
    }

    if (s_sec > 59) {
      s_min++;
      s_sec = 0;
    }
  }
  display->setTextSize(0);
}

void checkButtonsStop() {
  if (digitalRead(right/*presS*/) == 0) {
    s_fase++;
    if (s_fase == 3) {
      s_fase = 0;
      s_milis = 0;
      s_sec = 0;
      s_min = 0;
    }
  }
}

void drawGame() {
  display->clear();

  display->setCursor(40, 0);
  display->print(String(gameScore));
  display->setCursor(2, 0);
  display->print("Score:");
  display->drawLine(0, 9, 0, 127, COLOR_1);
  display->drawLine(63, 9, 63, 127, COLOR_1);
  display->drawLine(0, 9, 63, 9, COLOR_1);
  display->fillRect(playerX, 118, playerW, 2, COLOR_1);
  display->fillCircle(ballX, ballY, 1, COLOR_1);

  for (int i = 0; i < 14; i++)
    if (enL[i] == 1)
      display->fillRect(enX[i], enY[i], 8, 2, COLOR_1);

  //display.display();
}

void checkButtonsGame() {

  if (digitalRead(right/*presS*/) == 0) {
    controler = !controler;
    digitalWrite(3, controler);
  }


  if (controler == 1) {
    playerX = map(analogRead(analogy/*A0*/), 0, 1023, 1, 63 - playerW);
  }

  if (controler == 0) {
    if (digitalRead(down) == 0)
      if (playerX > 1)
        playerX--;

    if (digitalRead(up) == 0)
      if (playerX < 62 - playerW)
        playerX++;
  }
}

void GameReset() {
  ballX = random(10, 50);
  ballY = 50;
  ballDirectionX = 1;
  ballDirectionY = 1;
  gameScore = 0;
  for (int i = 0; i < 14; i++)
    enL[i] = 1;
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

void drawMenu() {
  if (!checkDisplayState(MS_MENU, chosenMenu)) {
    return;
  }

  display->clear();

  display->setTextSize(TEXT_SIZE_MENU);

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
  //display.drawBitmap(0,0,epd_bitmap_logo,64,30,1);

  display->drawImageFile("calc.png", menuX[0], menuY[0]);
  //display.drawBitmap(menuX[0],menuY[0],myBitmapcalc,24,24,1);

  display->drawImageFile("stop.png", menuX[1], menuY[1]);
  //display.drawBitmap(menuX[1],menuY[1],myBitmapstop,24,24,1);

  display->drawImageFile("game.png", menuX[2], menuY[2]);
  //display.drawBitmap(menuX[2],menuY[2],myBitmapgam,24,24,1);

  display->drawImageFile("calen.png", menuX[3], menuY[3]);
  //display.drawBitmap(menuX[3],menuY[3],myBitmapcalen,24,24,1);

  display->drawImageFile("phone.png", menuX[4], menuY[4]);
  //display.drawBitmap(menuX[4],menuY[4],myBitmapphone,24,24,1);

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


  if (rightTracker/*selectTracker*/.setPressed(digitalRead(right/*presS*/) == 0)) {
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
  num = 0;
  digit = 0;
  operation = 0;
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

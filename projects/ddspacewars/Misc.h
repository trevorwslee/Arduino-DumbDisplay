const long DueGapMillis = 40;


class Position
{
public:
  Position()
  {
    reset(0, 0);
  }
  Position(float pos_x, float pos_y)
  {
    reset(pos_x, pos_y);
  }

public:
  void moveBy(float inc_x, float inc_y)
  {
    this->pos_x += inc_x;
    this->pos_y += inc_y;
    this->last_x = this->x;
    this->last_y = this->y;
    this->x = this->pos_x;
    this->y = this->pos_y;
    if (last_valid)
    {
      this->moved = this->last_x != this->x || this->last_y != this->y;
    }
    else
    {
      this->last_valid = true;
      this->moved = true;
    }
  }
  void moveTo(float pos_x, float pos_y)
  {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->x = this->pos_x;
    this->y = this->pos_y;
    if (last_valid)
    {
      this->moved = this->last_x != this->x || this->last_y != this->y;
    }
    else
    {
      this->last_valid = true;
      this->moved = true;
    }
  }
  void moveXTo(float pos_x)
  {
    moveTo(pos_x, this->pos_y);
  }
  void moveYTo(float pos_y)
  {
    moveTo(this->pos_x, pos_y);
  }
  bool checkMoved()
  {
    bool res = this->moved;
    this->moved = false;
    return res;
  }
  void reset(float pos_x, float pos_y)
  {
    this->pos_x = pos_x;
    this->pos_y = pos_y;
    this->x = this->pos_x;
    this->y = this->pos_y;
    this->last_valid = false;
    this->moved = true;
  }
  inline int getX() { return this->x; }
  inline int getY() { return this->y; }

private:
  float pos_x;
  float pos_y;
  int x;
  int y;
  bool last_valid;
  int last_x;
  int last_y;
  bool moved;
};

class PositionGroup
{
public:
  PositionGroup(Position *positions, int positionCount)
  {
    this->positions = positions;
    this->positionCount = positionCount;
  }

public:
  void resetAll(float pos_x, float pos_y)
  {
    for (int i = 0; i < this->positionCount; i++)
    {
      Position *position = this->positions + i;
      position->reset(pos_x, pos_y);
    }
  }
  bool checkAnyMoved()
  {
    bool moved = false;
    for (int i = 0; i < this->positionCount; i++)
    {
      Position *position = this->positions + i;
      if (position->checkMoved())
      {
        moved = true;
      }
    }
    return moved;
  }

private:
  Position *positions;
  int positionCount;
};

class FrameControl
{
public:
  FrameControl()
  {
    reset();
  }

public:
  bool checkDue()
  {
    long nowMillis = millis();
    bool due = (this->nextDueMillis - nowMillis) <= 0;
    if (due)
    {
      if (this->nextDueMillis == 0)
      {
        this->nextDueMillis = nowMillis;
      }
      this->nextDueMillis += DueGapMillis;
#if defined(DEBUG_LED_PIN)
      int status = (this->nextDueMillis - nowMillis) > 10 ? 1 : 0;
      digitalWrite(DEBUG_LED_PIN, status);
#endif
    }
    if (due)
    {
      this->frameNum += 1;
    }
    return due;
  }
  inline long getFrameNum()
  {
    return this->frameNum;
  }
  void reset()
  {
    this->nextDueMillis = 0;
    this->frameNum = 0;
  }

private:
  long nextDueMillis;
  long frameNum;
};

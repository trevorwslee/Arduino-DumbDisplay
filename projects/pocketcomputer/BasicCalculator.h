#include <stdio.h>
#include <string.h>
#include <math.h>

char CaculatorDisplayBuffer[30];
int CaculatorFormatForDisplayGetLen(float num) {
    sprintf(CaculatorDisplayBuffer, "%f", num);  // will have decimal point
    int len = strlen(CaculatorDisplayBuffer);
    while (len > 1) {
        if (CaculatorDisplayBuffer[len - 1] != '0') {
            break;
        }
        CaculatorDisplayBuffer[--len] = 0;
    }
    if (len > 0) {
        if (CaculatorDisplayBuffer[len - 1] == '.') {
            CaculatorDisplayBuffer[--len] = 0;
        }
    }
    return len;
}
const char* CaculatorFormatForDisplay(float num, int max_width) {
    int len = CaculatorFormatForDisplayGetLen(num);
    return len < max_width ? CaculatorDisplayBuffer : NULL;
}

class PrimitiveCalcUnit {
    public:  
        PrimitiveCalcUnit() {
            reset();
        }
    public:
        bool isValid() {
            return this->oper != 0;
        }
        void reset() {
            this->num = 0;
            this->oper = 0;
        }
    public:
        float num;
        char oper;
};

class PrimitiveCalculator {
    public:
        PrimitiveCalculator(short DISP_WIDTH) {
            this->DISP_WIDTH = DISP_WIDTH;
            reset();
        }
    public:
        inline float getNum() {
            return _getNum();
        }
        inline bool isInError() {
            return this->entering == 'E';
        }   
        void reset() {
            // this->entering = 0;
            // _setNum(0);
            _restart(0);
            this->lhs.reset();
            this->prev_lhs.reset();
        }
        bool push(char what) {
            bool ok = _push(what);
            if (ok && this->entering == 0) {
                float num = _getNum();
                if (num == NAN || num == -NAN || num == INFINITY || num == -INFINITY) {
                    this->entering = 'E';
                }
            }
            return ok;
        }
        bool _push(char what) {
            if (isInError()) {
                return false;
            }
            if (entering != 0) {
                if (this->lhs.isValid()) {
                    if (_IsOperOrEq(what)) {
                        if (this->lhs.oper == '*' || this->lhs.oper == '/') {
                            _restart(_Calc(this->lhs.num, this->lhs.oper, _getNum()));
                        } else if (this->lhs.oper == '+' || this->lhs.oper == '-') {
                            if (this->prev_lhs.isValid()) {
                              this->lhs.num = _Calc(this->prev_lhs, this->lhs.num);
                            }
                            this->prev_lhs = this->lhs;
                        }
                    }
                }
                if (_IsOper(what)) {
                    this->lhs.oper = what;
                    this->lhs.num = _getNum();
                    _setNum(0);
                    this->entering = 0;
                    return true;
                }
                else if (what == '=') {
                    float res = _getNum();
                    if (this->prev_lhs.isValid()) {
                        res = _Calc(this->prev_lhs, res);
                    } 
                    _restart(res);
                    return true;
                }
            }
            // char ori_entering = this->entering;
            // short ori_intPartWidth = this->intPartWidth;
            // float ori_numPart = this->numPart;
            // short ori_fracPart = this->fracPart;
            // short ori_fracPartSize = this->fracPartSize;
            if (entering == 0 && what == '-') {
                this->negate = true;
                return true;
            } else if (what == '.') {
                if (entering == 'F') {
                    return false;
                }
                entering = 'F';
                return true;
            }
            if (what < '0' || what > '9') {
                return false;
            }
            int displayLen = CaculatorFormatForDisplayGetLen(_getNum());
            if (displayLen + 1 > this->DISP_WIDTH) {
                return false;
            }
            short digit = what - '0';
            if (entering == 'F') {
                fracPart = 10 * fracPart + digit;
                fracPartSize += 1;
            } else {
                if (entering == '!') {
                    return false;
                }
                // if (digit != 0 || numPart > 0.9) {
                //     intPartWidth += 1;
                // }
                numPart = 10 * numPart + digit;
                entering = 'I';
            }
            // short width;
            // if (fracPartSize > 0) {
            //     width = (intPartWidth == 0 ? 1 : intPartWidth) + 1 + fracPartSize; 
            // } else {
            //     width = intPartWidth;
            // }
            // if (width > DISP_WIDTH) {
            //     this->entering = ori_entering;
            //     this->intPartWidth = ori_intPartWidth;
            //     this->numPart = ori_numPart;
            //     this->fracPart = ori_fracPart;
            //     this->fracPartSize = ori_fracPartSize;
            //     return false;
            // }
            return true;
        }
        // assume 0 terminated
        bool push(const char* whats) {
            while (true) {
                char what = *whats;
                if (what == 0) {
                    return true;
                }
                if (!push(what)) {
                    return false;
                }
                whats += 1;
            }            
        }
        bool isEntering() {
            return entering != 0;
        }
        void _push(float num) {
            _setNum(num);
            this->entering = '!';
        }
    private:
        static float _Calc(PrimitiveCalcUnit lhs, float num) {
            return _Calc(lhs.num, lhs.oper, num);
        }
        static float _Calc(float left_num, char oper, float right_num) {
            if (oper == '+') {
                return left_num + right_num;
            } else if (oper == '-') {
                return left_num - right_num;
            } else if (oper == '*') {
                return left_num * right_num;
            } else if (oper == '/') {
                return left_num / right_num;
            }
            return 0;
        }    
        static bool _IsOperOrEq(char what) {
            return _IsOper(what) || what == '=';
        }    
        static bool _IsOper(char what) {
            return what == '+' || what == '-' || what == '*' || what == '/';
        }    
        float _getNum() {
            float res;
            if (this->fracPartSize > 0) {
                float demominator = 10;
                for (short i = 1; i < this->fracPartSize; i++) {
                    demominator *= 10;
                }
                res = this->numPart + (this->fracPart / demominator);
            } else {
                res = this->numPart;
            }
            return this->negate ? -res : res;
        }
        void _setNum(float num) {
            this->numPart = num;
            this->fracPart = 0;
            this->fracPartSize = 0;
        }
        void _restart(float num) {
            _setNum(num);
            this->entering = 0;
            this->negate = false;
            //this->intPartWidth = 0;  // doesn't count the init 0
        }
    private:
        short DISP_WIDTH;
        char entering;  // 0; 'I', 'F', '!', 'E'
        //short intPartWidth;
        bool negate;
        float numPart;
        short fracPart;
        short fracPartSize;
        PrimitiveCalcUnit prev_lhs;
        PrimitiveCalcUnit lhs;
};

class BasicCalcUnit {
    public:  
        BasicCalcUnit(short DISP_WIDTH, BasicCalcUnit* prev): calc(DISP_WIDTH)  {
            //reset();
            this->prev = prev;
        }
        ~BasicCalcUnit() {
            if (this->prev != NULL) {
                delete this->prev;
            }
        }
    public:
        // bool isValid() {
        //     return this->oper != 0;
        // }
        // void reset() {
        //     this->num = 0;
        //     this->oper = 0;
        // }
    public:
        BasicCalcUnit* prev;
        PrimitiveCalculator calc;
};

class BasicCalculator {
    public:
        BasicCalculator(short DISP_WIDTH) {
            this->DISP_WIDTH = DISP_WIDTH;
            this->curr = new BasicCalcUnit(DISP_WIDTH, NULL);
        }
        ~BasicCalculator() {
            delete this->curr;
        }
    public:
        inline float getNum() {
            return this->curr->calc.getNum();
        }    
        inline bool isInError() {
            return this->curr->calc.isInError();
        }
        void reset() {
            return this->curr->calc.reset();
        }
        bool push(char what) {
            if (what == '(') {
                if (!this->curr->calc.isEntering()) {
                    this->curr = new BasicCalcUnit(this->DISP_WIDTH, this->curr);
                    return true;
                }
                return false;
            } else if (what == ')') {
                if (this->curr->prev != NULL) {
                    if (this->curr->calc.push('=')) {
                        float num = this->curr->calc.getNum(); 
                        this->curr = this->curr->prev;
                        this->curr->calc._push(num);
                        return true;
                    }
                }
                return false;
            } else {
                return this->curr->calc.push(what);
            }
        }
        // assume 0 terminated
        bool push(const char* whats) {
            while (true) {
                char what = *whats;
                if (what == 0) {
                    return true;
                }
                if (!push(what)) {
                    return false;
                }
                whats += 1;
            }            
        }

    private:
        short DISP_WIDTH;
        BasicCalcUnit* curr;
};


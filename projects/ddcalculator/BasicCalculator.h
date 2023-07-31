#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

char* StrRev(char* str) {
    int size = strlen(str);
    int count = size / 2;
    for (int i = 0; i < count; i++) {
        char tmp = str[i];
        str[i] = str[size - i - 1];
        str[size - i - 1] = tmp;
    }
    return str;
}

char CaculatorDisplayBuffer[30];
void CaculatorIntToString(int32_t intPart, char* buffer) {
    if (intPart == 0) {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }
    bool negate = false;
    if (intPart < 0) {
        intPart = -intPart;
        negate = true;
    }
    int i = 0;
    while (intPart > 0) {
        int in = intPart % 10;
        intPart = intPart / 10;
        buffer[i++] = '0' + in;        
    }
    if (negate) {
        buffer[i++] = '-';
    }
    buffer[i] = 0;
    StrRev(buffer);
    //strrev(buffer);
} 
int CaculatorFormatForDisplayGetLen(double num, int8_t max_width, char* buffer = CaculatorDisplayBuffer) {
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
    // if sprintf doesn't work, enable it
    int32_t intPart = (int32_t) num;
    num = abs(num - intPart);
    int32_t fracPart = 100000 + (int32_t) (0.5 + 10000 * num);  // 4 decimal places
    char intBuffer[10];
    char fracBuffer[10];
    CaculatorIntToString(intPart, intBuffer);
    CaculatorIntToString(fracPart, fracBuffer);
    sprintf(buffer, "%s.%s", intBuffer, fracBuffer + 1);
    //sprintf(buffer, "%d.%d", intPart, fracPart);  // will have decimal point
#else        
    sprintf(buffer, "%f", num);  // will have decimal point
#endif
    int len = strlen(buffer);
    while (len > 1) {
        if (buffer[len - 1] != '0') {
            break;
        }
        buffer[--len] = 0;
    }
    if (len > 0) {
        if (buffer[len - 1] == '.') {
            buffer[--len] = 0;
        }
    }
    return len;
}
const char* CaculatorFormatForDisplay(double num, int8_t max_width, char* buffer = CaculatorDisplayBuffer) {
    int8_t len = CaculatorFormatForDisplayGetLen(num, max_width, buffer);
    return len <= max_width ? buffer : NULL;
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
        double num;
        char oper;
};

class PrimitiveCalculator {
    public:
        PrimitiveCalculator(int8_t DISP_WIDTH) {
            this->DISP_WIDTH = DISP_WIDTH;
            reset();
        }
    public:
        double getNum() {
            if (entering == 0) {
                if (lhs.isValid()) {
                    return lhs.num;
                }
            }
            return _getNum();
        }
        inline bool isInError() {
            return this->entering == 'E';
        }   
        const char* getFormatted(char* buffer = CaculatorDisplayBuffer) {
            if (!isInError()) {
                double num = getNum();
                if (!isnan(num) && !isinf(num)) {
                    return CaculatorFormatForDisplay(num, DISP_WIDTH, buffer);
                }
            }
            this->entering = 'E';
            return "Error";
        }
        void reset() {
            // this->entering = 0;
            // _setNum(0);
            _restart(0);
            //this->lhs.reset();
            //this->prev_lhs.reset();
        }
        bool push(char what) {
            // if (true) {
            //     double num = _getNum();
            //     if (num == NAN) {
            //         this->entering = 'E';
            //         return false;
            //     }
            // }
            bool ok = _push(what);
            // if (ok) {
            //     double num = _getNum();
            //     if (num == NAN) {
            //         this->entering = 'E';
            //     }
            // }
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
                            _setNum(_Calc(this->lhs.num, this->lhs.oper, _getNum()));
                            //_restart(_Calc(this->lhs.num, this->lhs.oper, _getNum()));
                        } else if (this->lhs.oper == '+' || this->lhs.oper == '-') {
                            if (this->prev_lhs.isValid()) {
                              this->lhs.num = _Calc(this->prev_lhs, this->lhs.num);
                            }
                            this->prev_lhs = this->lhs;
                        }
                    }
                }
                if (what == '%') {
                    _setNum(_Calc(_getNum(), '/', 100));
                    return true;
                } else if (_IsOper(what)) {
                    this->lhs.oper = what;
                    this->lhs.num = _getNum();
                    _setNum(0);
                    this->entering = 0;
                    return true;
                } else if (what == '=') {
                    double res = _getNum();
                    if (this->prev_lhs.isValid()) {
                        res = _Calc(this->prev_lhs, res);
                    } 
                    _restart(res, '='/*'!'*/);
                    return true;
                }
            }
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
            int displayLen = CaculatorFormatForDisplayGetLen(_getNum(), this->DISP_WIDTH, CaculatorDisplayBuffer);
            if (displayLen + 1 > this->DISP_WIDTH) {
                return false;
            }
            int32_t digit = what - '0';
            if (this->entering == 'F') {
                this->fracPart = 10 * fracPart + digit;
                this->fracPartSize += 1;
            } else {
                if (entering == '!') {
                    return false;
                }
                if (entering == '=') {
                    this->numPart = 0;
                }
                this->numPart = 10 * this->numPart + (double) digit;
                this->entering = 'I';
            }
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
        void _push(double num) {
            _setNum(num);
            this->entering = '!';
        }
    private:
        static double _Calc(PrimitiveCalcUnit lhs, double num) {
            return _Calc(lhs.num, lhs.oper, num);
        }
        static double _Calc(double left_num, char oper, double right_num) {
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
        double _getNum() {
            double res;
            if (this->fracPartSize > 0) {
                double demominator = 10;
                for (int8_t i = 1; i < this->fracPartSize; i++) {
                    demominator *= 10;
                }
                res = this->numPart + ((double) this->fracPart / demominator);
            } else {
                res = this->numPart;
            }
            double num = this->negate ? -res : res;
            // if (isnan(num) || isinf(num)) {
            //     num = NAN;
            // }
            return num;
        }
        void _setNum(double num) {
            this->numPart = num;
            this->fracPart = 0;
            this->fracPartSize = 0;
        }
        void _restart(double num, char entering = 0) {
            _setNum(num);
            this->entering = entering;
            this->negate = false;
            this->lhs.reset();
            this->prev_lhs.reset();
            //this->intPartWidth = 0;  // doesn't count the init 0
        }
    private:
        int8_t DISP_WIDTH;
        char entering;  // 0; 'I', 'F', '!', 'E'
        //short intPartWidth;
        bool negate;
        double numPart;
        int32_t fracPart;
        int8_t fracPartSize;
        PrimitiveCalcUnit prev_lhs;
        PrimitiveCalcUnit lhs;
};

class BasicCalcUnit {
    public:  
        BasicCalcUnit(int8_t DISP_WIDTH, BasicCalcUnit* prev): calc(DISP_WIDTH)  {
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
        BasicCalculator(int8_t DISP_WIDTH) {
            this->DISP_WIDTH = DISP_WIDTH;
            this->curr = new BasicCalcUnit(DISP_WIDTH, NULL);
        }
        ~BasicCalculator() {
            delete this->curr;
        }
    public:
        inline double getNum() {
            return this->curr->calc.getNum();
        }    
        inline bool isInError() {
            return this->curr->calc.isInError();
        }
        inline bool isGrouping() {
            return this->curr->prev != NULL;
        }
        inline const char* getFormatted(char* buffer = CaculatorDisplayBuffer) {
            return this->curr->calc.getFormatted(buffer + 1);
        }
        const char* getFormattedEx(char* buffer = CaculatorDisplayBuffer) {
            const char* formatted = this->curr->calc.getFormatted(buffer + 1);
            if (this->curr->prev == NULL) {
                return formatted;
            } else {
                buffer[0] = '(';
                return buffer;
            }
        }
        // const char* getFormatted() {
        //     if (isInError()) {
        //         return "error";
        //     } else {
        //         return CaculatorFormatForDisplay(getNum(), DISP_WIDTH);
        //     }
        // }
        void reset() {
            if (this->curr->prev != NULL) {
                delete this->curr->prev;
                this->curr->prev = NULL;
            }
            this->curr->calc.reset();
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
                        double num = this->curr->calc.getNum(); 
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
        int8_t DISP_WIDTH;
        BasicCalcUnit* curr;
};


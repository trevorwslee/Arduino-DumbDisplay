

#include "BasicCalculator.h"




char Buffer[256];

bool check(float num1, float num2) {
    float diff = abs(num1 - num2);
    return diff < 0.1;
}
const char* format(bool ok, float expecting, float num, const char* formatted) {
    bool checked = check(expecting, num);
    sprintf(Buffer, ". %s -- %s; expecting %f ... got %f (%s)", checked?"GOOD":"XXX", ok?"OK":"?", expecting, num, formatted);
    return Buffer;
}

void verify(PrimitiveCalculator& cal, const char* input, float expecting) {
    cal.reset();
    bool ok = cal.push(input) && !cal.isInError();
    //bool inError = cal.isInError();
    float num = cal.getNum();
    const char* formatted = cal.getFormatted();
    Serial.println(format(ok, expecting, num, formatted));
}
void verify(BasicCalculator& cal, const char* input, float expecting) {
    cal.reset();
    bool ok = cal.push(input) && !cal.isInError();
    Serial.println(format(ok, expecting, cal.getNum(), cal.getFormatted()));
}

void primitiveMain() {
    PrimitiveCalculator cal(10);
    verify(cal, "0.5-0.5+.5+1/2+.1*2.0=", 1.2);
    verify(cal, ".15+0.15+0.7=", 1);
    verify(cal, "0.123", 0.123);
    verify(cal, "123=", 123);
    verify(cal, "1+1+5+2*3*4*5+2+3+4+2*3*4=", 160);
    verify(cal, "2+3*4+5*2=", 24);
    verify(cal, "2+3*4+5=", 19);
    verify(cal, "2+3*4=", 14);
    verify(cal, "2*3*4*5+1+2=", 123);
    verify(cal, "2*3+4=", 10);
    verify(cal, "1+2+3=", 6);
    verify(cal, "1+2+3+4=", 10);
    verify(cal, "123", 123);
    verify(cal, "4/2-3=", -1);
    verify(cal, "2*3+10=", 16);
    verify(cal, "123+321=", 444);
}
void basicMain() {
    BasicCalculator cal(10);
    verify(cal, "(1+(2+1)-2+1)*(3+(4-7))=", 0);
    verify(cal, "(1+2)*(3+4)=", 21);
    verify(cal, "(1+2)*3=", 9);
    verify(cal, "(1)=", 1);
    verify(cal, "12.34=", 12.34);
    verify(cal, "123", 123);
}

void displaySizeMain() {
    PrimitiveCalculator cal(5);
    verify(cal, "12.3", 12.3);
    if (true) {
        cal.reset();
        bool ok = !cal.push("-12345");
        float expecting = -1234;
        Serial.println(format(ok, expecting, cal.getNum(), cal.getFormatted()));
    }
    if (true) {
        cal.reset();
        bool ok = !cal.push("123456");
        float expecting = 12345;
        Serial.println(format(ok, expecting, cal.getNum(), cal.getFormatted()));
    }
    verify(cal, "-1234", -1234);
    verify(cal, "12.34", 12.34);
    verify(cal, "12345", 12345);
}

void otherMain() {
    PrimitiveCalculator cal(10);
    if (true) {
        cal.reset();
        bool ok = cal.push("1/0=");
        Serial.println(format(ok, 1, cal.isInError() ? 1 : 0, cal.getFormatted()));
    }
    verify(cal, "0/123=", 0);
}


void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.println();
    for (int i = 0; i < 5; i++) {
        delay(1000);
        Serial.println("...");
    }
    primitiveMain();        
    basicMain();
    displaySizeMain();
    otherMain();
    if (false) {
        double num;
        const char* formatted;
        num = 12345678; formatted = CaculatorFormatForDisplay(num, 10); Serial.println(formatted);
        num = -12345678; formatted = CaculatorFormatForDisplay(num, 10); Serial.println(formatted);
        // char buffer[30];
        // double ori_num = 1234567;
        // int32_t intPart = (int32_t) ori_num;
        // double num = abs(ori_num - intPart);
        // int32_t fracPart = (int) (0.5 + pow(10, 4) * num);
        // sprintf(buffer, "%d.%d", intPart, fracPart);  // will have decimal point
        // Serial.print(ori_num);
        // Serial.print("==>");
        // Serial.print(intPart);
        // Serial.print("==>");
        // Serial.println(buffer);
    }
}
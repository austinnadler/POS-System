// File: PromptItem.cpp

#include "PromptItem.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

string PromptItem::toStringFile() const {
    ostringstream oss;
    oss << "warn," << warning << "," << name << "," << price << "," << numOnHand << "," << code;
    return oss.str();
}//end toStringBack()



bool PromptItem::setWarning(const string& warning) {
    if(!(warning.length() > MAX_WARNING_LENGTH)) {
        try {
            this -> warning = warning;
            return true;
        } catch (invalid_argument e) {
            return false;
        }
    } else {
        return false;
    }
}// end setWarning()
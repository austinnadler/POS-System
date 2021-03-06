// File: GMItem.cpp

// General Merchandise Items (Non-grocery)
#include "GMItem.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

bool GMItem::setItemName(const string& name)  {
    if(!(name.length() > MAX_NAME_LENGTH)) {
        this -> name = name;
        return true;
    } else {
        return false;
    }
}//end setItemName()

bool GMItem::setItemPrice(const string& price){
        try {
            this -> price = stod(price);
            return true;
        } catch (invalid_argument e) {
            return false;
        }
    
}//end setItemPrice()

bool GMItem::setNumOnHand(const string& n) {
    try {
        this -> numOnHand = stoi(n);
        return true;
    } catch (invalid_argument e) {
         return false;
    }
    
}//end setNumOnHand()

void GMItem::decreaseCount(const int& n) {
    this->numOnHand -= n;
}//end decreseCount()

void GMItem::increaseCount(const int& n) {
    this->numOnHand -= n;
}//end increaseCount()

bool GMItem::setItemCode(const string& code) {
        try {
            if(code.length() > 5 || stoi(code) < 0) {
                return false;
            } else {
                this -> code = stoi(code);
                return true;
            }
        } catch (invalid_argument e) {
            return false;
        }
    
}//end setItemCode()

string GMItem::toStringPOS() const {
    ostringstream oss;
    oss << left << code << "\t" << setw(25) << name << fixed << setprecision(2) << price;
    return oss.str();
}// end toStringPOS()

string GMItem::toStringFile() const {
    ostringstream oss;
    oss << "gm," << name << "," << price << "," << numOnHand << "," << code;
    return oss.str();
}//end toStringBack()
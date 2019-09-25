// File: AgeRestrictedItem.cpp

#include "AgeRestrictedItem.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

string AgeRestrictedItem::toStringFile() const {
    ostringstream oss;
    oss << "ar," << minAge << "," << name << "," << price << "," << numOnHand << "," << code;
    return oss.str();
}

bool AgeRestrictedItem::setMinAge(const string& minAge) {
    try {
        this -> minAge = stoi(minAge);
        return true;
    } catch(invalid_argument e) {
        return false;
    }
}
// File: main.cpp

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "List.h"

#include "GMItem.h"
#include "PromptItem.h"
#include "AgeRestrictedItem.h"

using namespace std;
using file_status_t = bool;
using total_price_t = double;

const int MAX_CART_SIZE = 400;                         // I know from working at Wal-Mart that POS systems have built in caps around 400 or 500 so I figured I'd add one.
const double TAX_RATE = 0.071;                         // IL 7.1%
void printItemsPOS(List<GMItem*>& items);              // To screen, displays only code, name, and price with periods for spacing, for use in checkout()
void printPOSPriceSection(List<GMItem*>& items);       // print subtotal, taxes, and total, for use in checkout()
void printPOSHeader();                                 // print the formatted POS header       
total_price_t calculateTax(const double& subTotal);    // Take the subTotal and calculate the tax that needs to be added based on the constant declared TAX_RATE. Program doesn't support multiple rates but could be implimented later
total_price_t calculateTotal(double& subTotal);        // Take the subtotal and calculate the tax, add it to the subtotal and return it as a double
total_price_t calculateSubtotal(List<GMItem*>& items); // Take the array of items and go through it adding all of the prices up and return it as a double
// Testing file reading and writing
void testFileIOandPricing();

// Utilities:
void loadItemsFromFile(ifstream& ifs, List<GMItem*>& items); 
file_status_t openFileIn(ifstream& ifs, const string& fileName);
file_status_t openFileOut(ofstream& ofs, const string& fileName);      
void totalOrderPrintResults(List<GMItem*>& cart, List<GMItem*>& inventory);
void decrementItemCount(List<GMItem*>& items, int& index);
void updateInventory(ofstream& ofs, List<GMItem*>& cart, List<GMItem*>& inventory);

// Default file names to make testing quicker.
string inFileName = "items.csv";

int main() {
    ostringstream oss;
    string input;
    int code = 0;
    int numItemsInCart = 0;
    double subTotal = 0;
    List<GMItem*> inventory;
    List<GMItem*> cart;
    ifstream ifs;
    ofstream ofs;

    file_status_t fInStatus = openFileIn(ifs, inFileName);
    if(!fInStatus) {
        cout << "Error opening file \"" << inFileName << "\"" << endl;
        exit(1);
    }
    loadItemsFromFile(ifs, inventory); 

    do { 
        if(numItemsInCart >= MAX_CART_SIZE) {
            cout << "Maximum cart sized reached. Totaling the order..." << endl;
                    totalOrderPrintResults(cart, inventory);
            cout << endl;
            return 0;
        }                        
        cout << "Enter product code (Type 'total' when done entering items): ";
        string index;
        getline(cin, input);
        if(input == "total") {
            totalOrderPrintResults(cart, inventory);
            return 0;
        } else if (input == "undo") {
                if(cart.size() < 1) {
                    cout << endl << "The list is already empty!" << endl;
                } else {
                    cart.popBack();
                    printPOSHeader();
                    printItemsPOS(cart);
                    printPOSPriceSection(cart);
                }
        } else if (input == "delete") {
            cout << "Enter item index: ";
            getline(cin, index);
            if(stoi(index) < 0 or stoi(index) > numItemsInCart - 1) {
                cout << "Invalid index." << endl;
            } else {
                cart.deleteAt(stoi(index));
            }   
        } else {
            try {
                code = stoi(input);
                bool validCode = false;
                bool validAge = false;
                bool validAcknowledge = false;
                for(int j = 0; j < inventory.size(); j++) {
                    if(inventory.getAt(j)->getItemCode() == code) {
                        validCode = true;
                    }
                }

                if(!validCode) {
                    cout << "Item " << input << " not found" << endl;
                }

                for(int i = 0; i < inventory.size(); i++) {
                    if(code == inventory.getAt(i)->getItemCode()) {
                        
                        // RTTI Run Time Type Idenification used to determine if the item is age restricted or not and take steps to verify age

                        GMItem *gm = inventory.getAt(i);
                        // Try to convert to subclass to check type
                        AgeRestrictedItem *ar = dynamic_cast<AgeRestrictedItem*>(gm);
                        PromptItem *pi = dynamic_cast<PromptItem*>(gm);
                        string input;



                        if(ar != nullptr) {
                            do {
                                cout << "This is an age restricted item. Verify with a valid photo ID." << endl;
                                cout << "Verify customer is at least " << ar->getMinAge() << " years or older (y/n): ";
                                getline(cin, input);
                                if(input == "y") {
                                    oss << inventory.getAt(i)->toStringPOS();
                                    subTotal += inventory.getAt(i)->getItemPrice();
                                    cart.pushBack(inventory.getAt(i));
                                    numItemsInCart++;
                                    validAge = true;
                                } else if(input == "n") {
                                    cout << "Sale not allowed." << endl;
                                    validAge = true;
                                }
                            } while(!validAge);
                        }
                        
                        if(pi != nullptr) {
                            do {
                                cout << pi->getWarning() << endl;
                                cout << "Ackowledge? (y/n): ";
                                getline(cin, input);
                                if(input == "y") {
                                    oss << inventory.getAt(i)->toStringPOS();
                                    subTotal += inventory.getAt(i)->getItemPrice();
                                    numItemsInCart++;
                                    cart.pushBack(inventory.getAt(i));
                                    validAcknowledge = true;
                                } else if(input == "n") {
                                    cout << "Sale not allowed." << endl;
                                    validAcknowledge = true;
                                }
                            } while(!validAcknowledge);      
                        }
                        if(ar == NULL && pi == NULL) {
                            cart.pushBack(inventory.getAt(i));
                        }
                    }                 
                } 
            } catch(exception e) {
                cout << "Invalid entry - Not a Number!" << endl;
            }
        }
        cout << endl;
                printPOSHeader();
                printItemsPOS(cart);
            cout << endl;
                printPOSPriceSection(cart);
        cout << endl;
    } while (input != "total" && numItemsInCart < MAX_CART_SIZE);
}

total_price_t calculateSubtotal(List<GMItem*>& items){
    total_price_t total = 0;
    for(int i = 0; i < items.size(); i++) {
        total += items.getAt(i)->getItemPrice();
    }
    return total;
}// end calculateSubtotal()

total_price_t calculateTotal(double& subtotal){
    return subtotal + calculateTax(subtotal);
}// end calculateTotal()

total_price_t calculateTax(const double& subTotal) {
    total_price_t taxes = subTotal * TAX_RATE;
    return taxes;
}// end calculateTax()

void printItemsPOS(List<GMItem*>& items) {
    for(int i = 0; i < items.size(); i++) {
        cout  << i << "\t" << items.getAt(i)->toStringPOS() << endl;
    }
}// end printItemsPOS()


void printPOSHeader() {
    cout << left << "INDEX\t" << "CODE\t" << "NAME" << right << setw(26) << "PRICE" << endl;
}

void printPOSPriceSection(List<GMItem*>& items) {
    total_price_t subTotal = calculateSubtotal(items);
    total_price_t taxes = calculateTax(subTotal);
    total_price_t totalPrice = calculateTotal(subTotal);

    cout << "Total number of items: " << items.size() << endl
         << "Subtotal: $" << subTotal << endl
         << "Tax: $" << fixed << setprecision(2) << taxes << " (at " << TAX_RATE*100 << "%)" << endl
         << "Total: $" << fixed << setprecision(2) << totalPrice << endl;
}

void totalOrderPrintResults(List<GMItem*>& cart, List<GMItem*>& inventory) {
    string input;
    bool isNum = false;
    double total = calculateSubtotal(cart) + calculateTax(calculateSubtotal(cart));
    double tender = 0;
    double change = 0;
    ofstream ofs;

    do {
        cout << "Enter the tender amount: ";
        getline(cin, input);
        try {
            tender += stod(input);
            change = tender - total;
            isNum = true;
        } catch(invalid_argument& e) {
            isNum = false;
        }
    } while(!isNum || tender < total);
    openFileOut(ofs, inFileName);
    updateInventory(ofs, cart, inventory);

    cout << endl;
    printPOSHeader();
    printItemsPOS(cart);
    printPOSPriceSection(cart);
    cout << "Tender amount: $" << fixed << setprecision(2) << tender << endl;
    cout << "Change due: $" << fixed << setprecision(2) << change << endl;
}

/*-------- File IO --------*/
void loadItemsFromFile(ifstream& ifs, List<GMItem*>& items) {
    string itemType;
    string name;
    string price;
    string qtyOnHand;
    string expirationDate;
    string code;
    string minAge;

    using type_id_t = string;
    type_id_t ageRestricted = "ar";
    type_id_t warning = "warn";
    type_id_t general = "gm";

    getline(ifs, itemType, ',');
    while(!ifs.eof()) {
        if(itemType == warning) {
            getline(ifs, expirationDate, ',');
            getline(ifs, name, ',');
            getline(ifs, price, ',');
            getline(ifs, qtyOnHand, ',');
            getline(ifs, code);

            PromptItem * pi = new PromptItem(expirationDate, name, stod(price), stoi(qtyOnHand), stoi(code));
            items.pushBack(pi);
            getline(ifs, itemType, ',');
        } else if (itemType == general) {
            getline(ifs, name, ',');
            getline(ifs, price, ',');
            getline(ifs, qtyOnHand, ',');
            getline(ifs, code);

            GMItem * gm = new GMItem(name, stod(price), stoi(qtyOnHand), stoi(code));
            items.pushBack(gm);
            getline(ifs, itemType, ',');
        } 
        else if (itemType == ageRestricted) {
            getline(ifs, minAge, ',');
            getline(ifs, name, ',');
            getline(ifs, price, ',');
            getline(ifs, qtyOnHand, ',');
            getline(ifs, code);

            AgeRestrictedItem * ar = new AgeRestrictedItem(stoi(minAge), name, stod(price), stoi(qtyOnHand), stoi(code));
            items.pushBack(ar);
            getline(ifs, itemType, ',');
        }
    }
}// end loadItemsFromFile()

void updateInventory(ofstream& ofs, List<GMItem*>& cart, List<GMItem*>& inventory) {
    for(int i = 0; i < cart.size(); i++) {
        cart.getAt(i)->decreaseCount();
    }
    for(int i = 0; i < inventory.size(); i++) {
        ofs << inventory.getAt(i)->toStringFile() << endl;
    }
    ofs.close();
}// end updateInventory

file_status_t openFileIn(ifstream& ifs, const string& fileName) {
    ifs.open(fileName);
    return ifs.is_open();
}// end openFileIn()

file_status_t openFileOut(ofstream& ofs, const string& fileName){
    ofs.open(fileName);
    return ofs.is_open();
}// end openFileOut()
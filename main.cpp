// File: main.cpp

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "GMItem.h"
#include "PromptItem.h"
#include "AgeRestrictedItem.h"

using namespace std;
using file_status_t = bool;
using total_price_t = double;

// FIX: writeItems needs to be updated to print the different types of items in distinct sections.
// FIX: add something to either verify that item numbers are unique, or present user with all of the items with that code and go from there.
// FIX: add returns to checkout()

// This program can really be broken into two programs: checkout() and performAdminFunctions()
// performAdminFunctions() and related functions:
void writeBack(ofstream& ofs, vector<GMItem*> items); // writes to a new file with the same format as the example input file so it can be re-used.
void printAdminInfo(vector<GMItem*> items);           // outputs to the screen the list of objects with all special information, for use in performAdminFunctions()

void checkout(); // create a new array of pointers and put the items that you want to check out into it. Program totals the purchase and adds tax, then decreases whatever counts need to be decreased. Uses vector functions for deletion and adding.
const int MAX_CART_SIZE = 500; // I know from working at Wal-Mart that POS systems have built in caps around 400 or 500 so I figured I'd add one.
const double TAX_RATE = 0.071;  // IL 7.1%
void printItemsPOS(vector<GMItem*> items);          // To screen, displays only code, name, and price with periods for spacing, for use in checkout()
void printPOSPriceSection(vector<GMItem*> items);   // print subtotal, taxes, and total, for use in checkout()
void printPOSHeader();  
total_price_t calculateTax(const double& subTotal);
total_price_t calculateTotal(double& subTotal);


// Testing file reading and writing
void testFileIOandPricing();

// Utilities:
void loadItemsFromFile(ifstream& ifs, vector<GMItem*> &items); 
void writeItems(ofstream& ofs, vector<GMItem*> items); // To file, includes expiration, age restrictions, etc. formatted for a csv file with a header
file_status_t openFileIn(ifstream& ifs, const string& fileName);
file_status_t openFileOut(ofstream& ofs, const string& fileName);

void sortItemsByName(vector<GMItem*> items);

// Default file names to make testing quicker.
string inFileName = "items.csv";
string outFileName = "itemsOut.csv";

int main() {
    ostringstream oss;
    string codeString;
    int code = 0;
    int numItemsInCart = 0;
    double subTotal = 0;
    bool foundItem = false;
    vector<GMItem*> inventory;
    vector<GMItem*> cart;
    ifstream ifs;
    ofstream ofs;

    file_status_t fInStatus = openFileIn(ifs, inFileName);
    if(!fInStatus) {
        cout << "Error opening file \"" << inFileName << "\"" << endl;
        exit(1);
    }
    loadItemsFromFile(ifs, inventory); 

    do { 
        foundItem = false;
        if(numItemsInCart == MAX_CART_SIZE) {
            cout << endl << "Maximum cart sized reached. Totaling the order..." << endl << endl;
                    printPOSHeader();
                    printItemsPOS(cart);
                    printPOSPriceSection(cart);
            cout << endl;
            return 0;
        }                        

        cout << "Enter product code (Type 'total' when done entering items): ";
        getline(cin, codeString);
        // FIX: add else if clause to do returns!
        if(codeString == "total") {
            cout << endl;
            try {
                    printPOSHeader();
                    printItemsPOS(cart);
                    printPOSPriceSection(cart);

                    // Change the on hand quantity.
                    for(int i = 0; i < numItemsInCart; i++) {
                        cart.at(i)->decreaseCount();
                    }
            } catch (exception e) {
                cout << "Invalid input: " << codeString << endl;
            }
            cout << endl;
        } else if (codeString == "undo") {
                if(cart.size() < 1) {
                    cout << endl << "The list is already empty!" << endl;
                } else {
                    cart.pop_back();
                    printPOSHeader();
                    printItemsPOS(cart);
                    printPOSPriceSection(cart);
                }
        } else {
            try {
                code = stoi(codeString);
                for(int i = 0; i < inventory.size(); i++) {
                        
                        // FIX: This code does not work
                        if (foundItem == false && i == inventory.size()) { 
                            cout << "No item with that code was found in the inventory file." << endl;
                        } 

                    if(code == inventory.at(i)->getItemCode()) {
                        
                        // RTTI Run Time Type Idenification used to determine if the item is age restricted or not and take steps to verify age

                        GMItem *gm = inventory.at(i);
                        // Try to convert to subclass to check type
                        AgeRestrictedItem *ar = dynamic_cast<AgeRestrictedItem*>(gm);
                        PromptItem *pi = dynamic_cast<PromptItem*>(gm);
                        string input;
                        bool valid = false;

                        if(ar != nullptr) {
                            do {
                                cout << "This is an age restricted item. Verify with a valid photo ID." << endl;
                                cout << "Verify customer is at least " << ar->getMinAge() << " years or older (y/n): ";
                                getline(cin, input);
                                if(input == "y") {
                                    oss << inventory.at(i)->toStringPOS();
                                    subTotal += inventory.at(i)->getItemPrice();
                                    numItemsInCart++;
                                    cart.push_back(inventory.at(i));
                                    foundItem = true;
                                    valid = true;
                                } else if(input == "n") {
                                    cout << "Sale not allowed." << endl;
                                    valid = true;
                                }
                            } while(!valid);
                        }
                        
                        if(pi != nullptr) {
                            do {
                                cout << pi->getWarning() << endl;
                                cout << "Ackowledge? (y/n): ";
                                getline(cin, input);
                                if(input == "y") {
                                    oss << inventory.at(i)->toStringPOS();
                                    subTotal += inventory.at(i)->getItemPrice();
                                    numItemsInCart++;
                                    cart.push_back(inventory.at(i));
                                    foundItem = true;
                                    valid = true;
                                } else if(input == "n") {
                                    cout << "Sale not allowed." << endl;
                                    valid = true;
                                }
                            } while(!valid);      
                        }
                        
                        
                        if(ar == NULL && pi == NULL) {
                            cart.push_back(inventory.at(i));
                        }

                    }                     
                }
            } catch(exception e) {
                cout << "Invalid Code Entered!" << endl;
            }
        }

        cout << endl;
                printPOSHeader();
                printItemsPOS(cart);
            cout << endl;
                printPOSPriceSection(cart);
        cout << endl;
        
    } while (codeString != "total" && numItemsInCart < MAX_CART_SIZE);
}

// Take the array of items and go through it adding all of the prices up and return it as a double
total_price_t calculateSubtotal(vector<GMItem*> items){
    total_price_t total = 0;
    for(int i = 0; i < items.size(); i++) {
        total += items.at(i)->getItemPrice();
    }
    return total;
}// end calculateSubtotal()


// Take the subtotal and calculate the tax, add it to the subtotal and return it as a double
total_price_t calculateTotal(double& subtotal){
    return subtotal + calculateTax(subtotal);
}// end calculateTotal()


// Take the subTotal and calculate the tax that needs to be added based on the constant declared TAX_RATE. Program doesn't support multiple rates but could be implimented later
total_price_t calculateTax(const double& subTotal) {
    total_price_t taxes = subTotal * TAX_RATE;
    return taxes;
}// end calculateTax()


// Take an ifs and an empty array and fill said array with items from a FORMATTED file
void loadItemsFromFile(ifstream& ifs, vector<GMItem*> &items) {
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
            items.push_back(pi);
            getline(ifs, itemType, ',');
        } else if (itemType == general) {
            getline(ifs, name, ',');
            getline(ifs, price, ',');
            getline(ifs, qtyOnHand, ',');
            getline(ifs, code);

            GMItem * gm = new GMItem(name, stod(price), stoi(qtyOnHand), stoi(code));
            items.push_back(gm);
            getline(ifs, itemType, ',');
        } 
        else if (itemType == ageRestricted) {
            getline(ifs, minAge, ',');
            getline(ifs, name, ',');
            getline(ifs, price, ',');
            getline(ifs, qtyOnHand, ',');
            getline(ifs, code);

            AgeRestrictedItem * ar = new AgeRestrictedItem(stoi(minAge), name, stod(price), stoi(qtyOnHand), stoi(code));
            items.push_back(ar);
            getline(ifs, itemType, ',');
        }
    }
}// end loadItemsFromFile()


void saveStoreCopy(ofstream& ofs, vector<GMItem*> items) {
    for(int i = 0; i < items.size(); i++) {
        ofs << items.at(i)->toStringPOS() << endl;
    }
    ofs.close();
}// end writeItems()


// Take a provided array of items and print it to the screen using the toStringPOS() method
void printItemsPOS(vector<GMItem*> items) {
    for(int i = 0; i < items.size(); i++) {
        cout  << items.at(i)->toStringPOS() << endl;
    }
}// end printItemsPOS()


// use provided ifstream to open provided filename to read information from
file_status_t openFileIn(ifstream& ifs, const string& fileName) {
    ifs.open(fileName);
    return ifs.is_open();
}// end openFileIn()


// use provided ofstream to open provided filename to write data to
file_status_t openFileOut(ofstream& ofs, const string& fileName){
    ofs.open(fileName);
    return ofs.is_open();
}// end openFileOut()


// print the formatted POS header
void printPOSHeader() {
    cout << left << "INDEX\t" << "CODE\t" << "NAME" <<  right << setw(26) << "PRICE" << endl;
}


// do the end of the transaction totaling and outputting all at once given the array of items and actual size size
void printPOSPriceSection(vector<GMItem*> items) {
    total_price_t subTotal = calculateSubtotal(items);
    total_price_t taxes = calculateTax(subTotal);
    total_price_t totalPrice = calculateTotal(subTotal);

    cout << "Total number of items: " << items.size() << endl
         << "Subtotal: $" << subTotal << endl
         << "Tax: $" << fixed << setprecision(2) << taxes << " (at " << TAX_RATE*100 << "%)" << endl
         << "Total: $" << fixed << setprecision(2) << totalPrice << endl;
}


// Insertion sort used 
void sortItemsByName(vector<GMItem*> items) {
    int i, j; 
    GMItem * key;
    for (i = 1; i < items.size(); i++) { 
       key = items.at(i); 
       j = i-1; 
       while (j >= 0 && items.at(i)->getItemName() > key->getItemName()) { 
           items.at(i+1) = items.at(j); 
           j = j-1; 
       } 
       items.at(j+1) = key; 
   } 
}
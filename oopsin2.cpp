// create two classes savigAcc and flexFDAcc
// each should have balance, acc num, rate of interest, autogen acc nums,
// threshhold for saving bank acc, etc each should have private data mems

// write funcs that deposit money into accounts, transfers money to and from
// acc, and operations like Add interest, view details, etc Use concept of
// friend class or friend funcs & static keywords. Demonstrate all
// functionalities through a menu driven program.#include <iostream>

#include <cstdio>
#include <iostream>
#include <map>
#include <string>

using namespace std;

class SavingAcc;
class FlexFDAcc;

map<int, SavingAcc *> allSavingAcc;
map<int, FlexFDAcc *> allFlexFDAcc;

class SavingAcc {
private:
  static inline int autoGenANums = 121212;
  static inline int minAmt = 5000;

  int accNum;
  string pin;
  int linkedAcc;
  double balance;
  string fullUsrName;

public:
  SavingAcc(string pin, string fullUsrName) {
    accNum = autoGenANums;
    autoGenANums++;
    this->pin = pin;
    linkedAcc = 0;
    balance = 0;
    this->fullUsrName = fullUsrName;
  }

  SavingAcc(string pin, double balance, string fullUsrName) {
    accNum = autoGenANums;
    autoGenANums++;
    this->pin = pin;
    linkedAcc = 0;
    this->balance = balance;
    this->fullUsrName = fullUsrName;
  }

  friend void linkAccounts(SavingAcc &, FlexFDAcc &);
  friend int transaction(SavingAcc &, SavingAcc &, double);
  friend double checkUpWithFD(SavingAcc &, double);

  int verifyPin(string pin) {
    if (this->pin == pin) {
      return 1;
    }

    return 0;
  }

  void DisplayAcc() {
    cout << "Full User Name: " << fullUsrName << endl;
    cout << "Account Number: " << accNum << endl;
    cout << "Balance: " << balance << endl;
    cout << "Linked FD Acc: " << linkedAcc << endl;
  }

  int myAccNo() { return this->accNum; }
  double myAccBal() { return this->balance; }
  int myLinkAcc() { return this->linkedAcc; }

  double deposit(double dpAmt) {
    balance += dpAmt;

    return balance;
  }

  double withdraw(double wdAmt) {
    if (wdAmt > balance) {
      cout << "Insufficient Balance" << endl;
      return balance;
    }
    balance -= wdAmt;

    if (balance < minAmt) {
      cout << "NOTE: Your balance is below minimum required Amount!\n\n";
    }
    return balance;
  }
};

class FlexFDAcc {
private:
  static inline int autoGenANums = 343434;
  static inline double interestRate = 3;
  static inline int minAmt = 5000;

  int accNum;
  string pin;
  int linkedAcc;
  double balance;
  string fullUsrName;

public:
  FlexFDAcc(string pin, string fullUsrName) {
    accNum = autoGenANums;
    autoGenANums++;
    this->pin = pin;
    linkedAcc = 0;
    balance = 0;
    this->fullUsrName = fullUsrName;
  }

  FlexFDAcc(string pin, double balance, string fullUsrName) {
    accNum = autoGenANums;
    autoGenANums++;
    this->pin = pin;
    linkedAcc = 0;
    this->balance = balance;
    this->fullUsrName = fullUsrName;
  }

  friend SavingAcc;

  friend void linkAccounts(SavingAcc &, FlexFDAcc &);
  friend void passYears(FlexFDAcc &, int);
  friend void updateInterestRate(double);
  friend double checkUpWithFD(SavingAcc &, double);

  int verifyPin(string pin) {
    if (this->pin == pin) {
      return 1;
    }

    return 0;
  }

  void DisplayAcc() {
    cout << "Full User Name: " << fullUsrName << endl;
    cout << "Account Number: " << accNum << endl;
    cout << "Linked Savings Acc: " << linkedAcc << endl;
    cout << "Balance: " << balance << endl;
    cout << "Interest Rate: " << interestRate << endl;
  }

  int myAccNo() { return this->accNum; }
  double myAccBal() { return this->balance; }
  int myLinkAcc() { return this->linkedAcc; }

  void empty() {
    if (!this->linkedAcc) {
      cout << "Transation Failed:" << endl;
      cout << "No Linked SavingAcc to transfer money to." << endl;
      return;
    }

    allSavingAcc[this->linkedAcc]->deposit(this->balance);
    this->balance = 0;

    cout << "Transation Successfull";

    return;
  }
};

void linkAccounts(SavingAcc &sva, FlexFDAcc &fda) {
  sva.linkedAcc = fda.accNum;
  fda.linkedAcc = sva.accNum;

  if (sva.balance > sva.minAmt && fda.balance < fda.minAmt) {
    double excs = sva.balance - sva.minAmt;
    double inNeed = fda.minAmt - fda.balance;

    if (inNeed > excs) {
      sva.withdraw(excs);
      fda.balance += excs;
    } else {
      sva.withdraw(inNeed);
      fda.balance = fda.minAmt;
    }
  }
}

int transaction(SavingAcc &from, SavingAcc &to, double amt) {
  if (from.balance < amt) {
    cout << "Transation Failed:" << endl;
    cout << "Insufficient balance to perform transaction." << endl;
    return 0;
  }

  from.withdraw(amt);
  to.deposit(amt);

  cout << "Transation Successfull" << endl;
  cout << "Your Acc Deatils:" << endl;
  from.DisplayAcc();

  return 1;
}

void passYears(FlexFDAcc &fda, int yrs) {
  double interest = fda.balance * fda.interestRate * yrs / 100;
  cout << "Current Amt: " << fda.balance << endl;
  cout << "Current interestRate: " << fda.interestRate << endl;
  cout << "Predicted interest after " << yrs << " year(s): ";
  cout << interest << endl; // P * R * T / 100
  cout << "Predicted Amt: " << fda.balance + interest << endl;
  cout << endl;

  return;
}

void updateInterestRate(double newRate) {
  FlexFDAcc::interestRate = newRate;

  return;
}

double checkUpWithFD(SavingAcc &sva, double amt) {
  if (!sva.linkedAcc) {
    return amt;
  }

  FlexFDAcc &fda = *allFlexFDAcc[sva.linkedAcc];

  if (fda.balance >= fda.minAmt) {
    return amt;
  }

  if (fda.balance + amt <= fda.minAmt) {
    fda.balance += amt;
    return 0;
  }

  double retAmt = fda.minAmt - fda.balance;
  fda.balance = fda.minAmt;

  return amt - retAmt;
}

SavingAcc *crtSavingAcc(double initBal, string pin, string fullUsrName) {
  SavingAcc *newAcc = new SavingAcc(pin, initBal, fullUsrName);
  allSavingAcc[newAcc->myAccNo()] = newAcc;

  return newAcc;
}

FlexFDAcc *crtFlexFDAcc(double initBal, string pin, string fullUsrName) {
  FlexFDAcc *newAcc = new FlexFDAcc(pin, initBal, fullUsrName);
  allFlexFDAcc[newAcc->myAccNo()] = newAcc;

  return newAcc;
}

void hardCodeAcc() {
  crtSavingAcc(5678, "0009", "Demo User"); // 121212
  crtSavingAcc(7500, "0009", "Demo User"); // 121213
  crtSavingAcc(7898, "0009", "Demo User"); // 121214
  crtSavingAcc(9863, "0009", "Demo User"); // 121215
  crtSavingAcc(6327, "0009", "Demo User"); // 121216
  crtSavingAcc(8427, "0009", "Demo User"); // 121217
  crtSavingAcc(1677, "0009", "Demo User"); // 121218

  crtFlexFDAcc(7500, "0009", "Demo User"); // 343434
  crtFlexFDAcc(8327, "0009", "Demo User"); // 343435
  crtFlexFDAcc(2157, "0009", "Demo User"); // 343436
  crtFlexFDAcc(8265, "0009", "Demo User"); // 343437
  crtFlexFDAcc(3566, "0009", "Demo User"); // 343438
  crtFlexFDAcc(1626, "0009", "Demo User"); // 343439
  crtFlexFDAcc(3266, "0009", "Demo User"); // 343440
}

bool authCredentials(int accNum, string pin, int accType) {
  if (accType) {
    if (allSavingAcc.find(accNum) == allSavingAcc.end()) {
      cout << "Provided SavingAcc does not exist." << endl;
      return false;
    }
    if (!allSavingAcc[accNum]->verifyPin(pin)) {
      cout << "Incorrect Pin." << endl;
      return false;
    }

    return true;
  }

  if (allFlexFDAcc.find(accNum) == allFlexFDAcc.end()) {
    cout << "Provided FlexFDAcc does not exist." << endl;
    return false;
  }
  if (!allFlexFDAcc[accNum]->verifyPin(pin)) {
    cout << "Incorrect Pin." << endl;
    return false;
  }

  return true;
}

void displayHelp() {
  cout << endl;
  cout << "Enter to Initiate Proccess:" << endl;
  cout << "0. Display this Menu" << endl;
  cout << "1. Create SavingAcc" << endl;
  cout << "2. Create FlexFDAcc" << endl;
  cout << "3. SavingAcc Display Info " << endl;
  cout << "4. FlexFDAcc Display Info" << endl;
  cout << "5. Link SavingAcc and FlexFDAcc" << endl;
  cout << "6. Transfer Money from your SavingAcc to else's" << endl;
  cout << "7. Time Machine, Increment Years" << endl;
  cout << "8. Update interestRate" << endl;
  cout << "9. Deposit in SavingAcc" << endl;
  cout << "10. Withdra from SavingAcc" << endl;
  cout << endl;
  cout << "-1: exit";
  cout << endl << endl;

  return;
}

void uiCreateAcct(int accType) {
  string fullUsrName;
  string pin;
  double initBal;

  cout << "Enter your Entire Name: ";
  getchar();
  getline(cin, fullUsrName);

  cout << "Enter a pin (whitespace na): ";
  cin >> pin;

  cout << "Enter initial deposit: ";
  cin >> initBal;

  cout << "------------------------------" << endl;

  if (accType == 1) {
    SavingAcc *newAcc = crtSavingAcc(initBal, pin, fullUsrName);
    newAcc->DisplayAcc();
  }

  FlexFDAcc *newAcc = crtFlexFDAcc(initBal, pin, fullUsrName);
  newAcc->DisplayAcc();
}

void uiDisplayInfo(int accType) {
  int accNum = 0;
  string pin;

  cout << "Enter Acc No.: ";
  cin >> accNum;

  cout << "Enter your Pin: ";
  cin >> pin;

  cout << endl;

  if (!authCredentials(accNum, pin, accType)) {
    return;
  }

  if (accType == 1) {
    allSavingAcc[accNum]->DisplayAcc();
    return;
  }

  allFlexFDAcc[accNum]->DisplayAcc();
}

void uiLinkAcc() {
  int svaAccNum = 0;
  int fdaAccNum = 0;
  string svaPin = "";
  string fdaPin = "";

  cout << "Enter SavingAcc Num: ";
  cin >> svaAccNum;

  cout << "Enter SavingAcc Pin: ";
  cin >> svaPin;

  cout << "Enter FlexFDAcc Num: ";
  cin >> fdaAccNum;

  cout << "Enter FlexFDAcc Pin: ";
  cin >> fdaPin;

  cout << endl;

  int auth = authCredentials(svaAccNum, svaPin, 1);
  auth *= authCredentials(fdaAccNum, fdaPin, 0);

  if (!auth) {
    return;
  }

  linkAccounts(*allSavingAcc[svaAccNum], *allFlexFDAcc[fdaAccNum]);

  cout << endl;

  allSavingAcc[svaAccNum]->DisplayAcc();
}

void uiTransaction() {
  int fromAccNum = 0;
  string pin = "";
  int toAccNum = 0;
  double amt;

  cout << "Enter Your SavingAcc Num: ";
  cin >> fromAccNum;

  cout << "Enter your Pin: ";
  cin >> pin;

  if (!authCredentials(fromAccNum, pin, 1)) {
    return;
  }

  cout << "Enter SavingAcc Num to transfer money to: ";
  cin >> toAccNum;
  if (allSavingAcc.find(toAccNum) == allSavingAcc.end()) {
    cout << "Transation Failed:";
    cout << "SavingAcc w/ Acc Number " << toAccNum << " not found.";
    return;
  }

  cout << "Enter Amount to Transfer: ";
  cin >> amt;

  transaction(*allSavingAcc[fromAccNum], *allSavingAcc[toAccNum], amt);

  return;
}

void uiTimeMachine() {
  int accNum;
  string pin;
  int yrs;

  cout << "Enter FlexFDAcc Num to Track: ";
  cin >> accNum;

  cout << "Enter pin: ";
  cin >> pin;

  if (!authCredentials(accNum, pin, 0)) {
    return;
  }

  cout << "View Amount after years: ";
  cin >> yrs;

  cout << endl;

  passYears(*allFlexFDAcc[accNum], yrs);

  return;
}

void uiUpdtInterestRate() {
  double newRate;

  cout << "Enter new Interest Rate: ";
  cin >> newRate;
  cout << endl;

  updateInterestRate(newRate);

  return;
}

void uiDeposit() {
  int accNum = 0;
  string pin;
  double amt;

  cout << "Enter your SavingAcc Num: ";
  cin >> accNum;

  cout << "Enter your Pin: ";
  cin >> pin;

  if (!authCredentials(accNum, pin, 1)) {
    return;
  }

  cout << "Enter Amt to Deposit: ";
  cin >> amt;
  cout << endl;

  amt = checkUpWithFD(*allSavingAcc[accNum], amt);
  allSavingAcc[accNum]->deposit(amt);

  cout << endl;

  allSavingAcc[accNum]->DisplayAcc();

  return;
}

void uiWithdraw() {
  int accNum;
  string pin;
  double amt;

  cout << "Enter your SavingAcc Num: ";
  cin >> accNum;

  cout << "Enter your Pin: ";
  cin >> pin;

  if (!authCredentials(accNum, pin, 1)) {
    return;
  }

  cout << "Enter amt to Withdraw";
  cin >> amt;

  allSavingAcc[accNum]->withdraw(amt);

  cout << endl;

  allSavingAcc[accNum]->DisplayAcc();

  return;
}

int main() {
  hardCodeAcc();
  displayHelp();

  int option = 0;

  while (option != -1) {
    cout << endl;
    cin >> option;
    cout << endl;

    switch (option) {

    case 0:
      displayHelp();
      break;

    case 1:
      uiCreateAcct(1);
      break;

    case 2:
      uiCreateAcct(0);
      break;

    case 3:
      uiDisplayInfo(1);
      break;

    case 4:
      uiDisplayInfo(0);
      break;

    case 5:
      uiLinkAcc();
      break;

    case 6:
      uiTransaction();
      break;

    case 7:
      uiTimeMachine();
      break;

    case 8:
      uiUpdtInterestRate();
      break;

    case 9:
      uiDeposit();
      break;

    case 10:
      uiWithdraw();
      break;
    }
  }

  return 0;
}

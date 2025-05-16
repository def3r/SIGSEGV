#include <iostream>

using namespace std;

class Employee {
protected:
  string name;
  string empId;
  string gender;
  string department;
  string designation;

  int age;
  int salary;


public:
  Employee(string n, string eId, int age, string g, string dept, int s) {
    this->name = n;
    this->empId = eId;
    this->age = age;
    this->gender = g;
    this->department = dept;
    this->salary = s;
    this->designation = "Employee";
  }

  void display() {
    cout << "Employee Details: " << endl;
    cout << "Name: " << this->name << endl;
    cout << "Emp ID: " << this->empId << endl;
    cout << "Age: " << this->age << endl;
    cout << "Gender: " << this->gender << endl;
    cout << "Department: " << this->department << endl;
    cout << endl;
  }
};

class Manager : protected Employee {
  
};

int main() {
  Employee adrien("Adrien Duport", "D-24601", 34, "Male", "Intern Management", 15000);

  adrien.display();

  return 0;
}

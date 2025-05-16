#include <iostream>
#include <string>

using namespace std;

class Book {
private:
  string title;
  string author;
  string isbn;
  int price;
  int year;
  int pages;

public:
  Book(string title, string author, string isbn, int price, int year,
       int pages) {
    this->title = title;
    this->author = author;
    this->price = price;
    this->isbn = isbn;
    this->year = year;
    this->pages = pages;
  }

  void display() {
    cout << "Title: " << title << endl;
    cout << "Author: " << author << endl;
    cout << "Price: " << price << endl;
    cout << "ISBN: " << isbn << endl;
    cout << "Year: " << year << endl;
    cout << "Pages: " << pages << endl;
  }
};

class EBook : public Book {
private:
  double fileSize;
  string fileFormat;

public:
  EBook(string t, string a, string i, int p, int y, int pg, double fs,
        string ff)
      : Book(t, a, i, p, y, pg) {
    this->fileSize = fs;
    this->fileFormat = ff;
  }

  void display() {
    Book::display();
    cout << "File Format: " << fileFormat << endl;
    cout << "File Size: " << fileSize << " Mb" << endl;
  }
};

class AudioBook : public Book {
protected:
  double audio_len;

public:
  AudioBook(Book &book, double audio_len) : Book(book) {
    this->audio_len = audio_len;
  }

  void display() {
    Book::display();
    cout << "Audio Length: " << this->audio_len << " hrs" << endl;
  }
};

// person, employee, manager
class Person {
protected:
  string name;
  string dob;
  string gender;

public:
  Person(string name, string dob, string gender) {
    this->name = name;
    this->dob = dob;
    this->gender = gender;
  }

  int calcAge(int yr) {
    int birthYr = stoi(this->dob) % 10000;
    return yr - birthYr;
  }

  void display() {
    cout << "Name: " << name << endl;
    cout << "DOB: " << stoi(dob) / 1000000 << "/" << (stoi(dob) / 10000) % 100
         << "/" << (stoi(dob) % 10000) << endl;
    cout << "Gender: " << gender << endl;
  }
};

class Employee : public Person {
protected:
  int empId;
  double salary;

public:
  Employee(string name, string dob, string gender, int empId, double salary)
      : Person(name, dob, gender) {
    this->empId = empId;
    this->salary = salary;
  }

  Employee(Person &p, int empId, int salary) : Person(p) {
    this->empId = empId;
    this->salary = salary;
  }

  void display() {
    Person::display();

    cout << "Emp Id: " << empId << endl;
    cout << "Salary: " << salary << endl;
  }
};

class Teacher : public Employee {
protected:
  string subject;
  int semester;

public:
  Teacher(Employee &e, string subject, int sem) : Employee(e) {
    this->subject = subject;
    this->semester = sem;
  }

  Teacher(Person &p, int empId, double salary, string subject, int sem)
      : Employee(p, empId, salary) {
    this->subject = subject;
    this->semester = sem;
  }

  void display() {
    Employee::display();
    cout << "Subject: " << this->subject << endl;
    cout << "Semester: " << this->semester << endl;
  }
};

class Student : public Person {
protected:
  int roll;
  float cgpa;

public:
  Student(Person &p, int roll, float cgpa) : Person(p) {
    this->roll = roll;
    this->cgpa = cgpa;
  }

  void display() {
    Person::display();
    cout << "Roll No: " << this->roll << endl;
    cout << "CGPA: " << this->cgpa << endl;
  }
};

class TeachingAssistant : public Student, public Teacher {

public:
  TeachingAssistant(Person &p, int eID, double sal, int roll, float cgpa,
                    string subject, int sem)
      : Student(p, roll, cgpa), Teacher(p, eID, sal, subject, sem) {}

  void display() {
    Student::display();
    cout << "Subject: " << this->subject << endl;
    cout << "Semester: " << this->semester << endl;
  }
};

// class C : public B, A { };
// teaching assistant: derived from student and faculty (is a rel).

int main() {
  EBook demobook("Aatmakatha~", "Arpit", "919-232-121-0004", 500, 2005, 29839,
                 34, "pdf");
  AudioBook demoaudiobook(demobook, 1.36);

  cout << "Book:" << endl;
  demobook.display();

  cout << endl;

  cout << "Audiobook:" << endl;
  demoaudiobook.display();

  cout << endl;

  // dob in string: ddmmyyyy
  Person Adrien("Adrien", "09092005", "male");
  Employee demo(Adrien, 123, 50000);
  Teacher newSir(demo, "Geography", 4);

  Person Louis("Louis", "14071789", "male");
  TeachingAssistant ta(Louis, 124, 12000, 15, 7.9, "EVS", 4);

  Adrien.display();
  cout << "Age of this person is " << Adrien.calcAge(2024) << " yrs" << endl;

  cout << endl;

  cout << "Teacher Details:" << endl;
  newSir.display();

  cout << endl;

  cout << "Teaching Assistant Details:" << endl;
  ta.display();

  return 0;
}

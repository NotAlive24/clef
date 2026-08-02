#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
class ReadWrite {
public:
  int write() {
    ifstream vault(".vault.clef");
    if (!vault.is_open()) {
      cerr << "File Not Found\n";
      return 404;
    }
    return 0;
  }
  int create() {
    ofstream vault(".vault.clef");
    if (vault) {
      cout << "\nNew file is created successfully";
      return 0;
    } else {
      cerr << "\nFile was not created, possible problems could be lack of "
              "permission to write";
      exit(1);
    }
    return 0;
  }
};

int main() {
  ReadWrite RW;
  int filecheck = RW.write();
  char choice = 'n';
  if (filecheck == 404) {
    cout << "Do you want to create a new vault? (y/n): ";
    cin >> choice;
  }
  if (choice == 'y' || choice == 'Y') {
    RW.create();
  } else {
    exit(1);
  }
  return 0;
}
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
using namespace std;
class CheckCreate {
public:
    // Checking file existance
    int check() {
        ifstream vault(".vault.clef");
        if (!vault.is_open()) {
            vault.close();
            cerr << "File Not Found\n";
            return 404;
        } else {
            cout << "vault found, Do you want to use it? (y/n): ";
            char open;
            cin >> open;
            if (open == 'y' || open == 'Y') {
                return 1;
            } else if (open == 'n' || open == 'N') {
                exit(1);
            } else {
                cerr << "Invaild input";
                exit(1);
            }
        }
        vault.close();
        return 0;
    }
    // Creating file
    int create() {
        ofstream vault(".vault.clef");
        if (vault) {
            cout << "\nNew file is created successfully\n";
            vault.close();
            return 0;
        } else {
            cerr << "\nFile was not created, possible problems could be lack of "
                    "permission to write";
            exit(1);
        }
        return 0;
    }
};

class Read {
public:
    int Readvault() {
        ifstream vault(".vault.clef");
        string line;
        int i = 1;
        while (getline(vault, line)) {
            cout << i << ".\t" << line << endl;
            i++;
        }
        return 0;
    }
};

class Write {
public:
    int Writevault() {
        ofstream vault(".vault.clef", ios::app);
        if (!vault.is_open()) {
            cerr << "Error: Could not open vault for writing.\n";
            return 1;
        }
        string addLine;
        int i = 1;
        string appName, username, password;

        while (true) {
            cout << "Enter the application name (Use EXIT to exit): ";
            if (!getline(cin, appName))
                break;
            if (appName == "EXIT") {
                break;
            }
            if (appName.empty()) {
                continue;
            }
            cout << "Enter the username: ";
            getline(cin, username);
            cout << "Enter the password: ";
            getline(cin, password);

            vault << appName << "\t\t" << username << "\t\t" << password << endl;
        }
        vault.close();
        cout << "\n";
        cout << "Data saved successfully!\n";
        return 0;
    }
};

int main() {
    string uname, password;
    // Check and create vault
    CheckCreate CC;
    Read rd;
    Write wt;
    int filecheck = CC.check();
    if(filecheck == 1){
        rd.Readvault();
    }

    char choice = 'n';
    if (filecheck == 404) {
        cout << "Do you want to create a new vault? (y/n): ";
        cin >> choice;
        cin.ignore();
        if (choice == 'y' || choice == 'Y') {
            CC.create();
        } else {
            exit(1);
        }
    }
    if (choice != 'n') {
        rd.Readvault();
        cout << "\n";
    }
    wt.Writevault();
    cout << "\n";
    rd.Readvault();
    return 0;
}
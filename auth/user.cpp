// user.cpp
#include "user.h"
#include <iostream>
#include <sstream>
using namespace std;

User::User() {
    username = "";
    password = "";
    fullName = "";
    email = "";
    phone = "";
    role = "user";
}

User::User(string username, string password, string fullName, 
           string email, string phone, string role) {
    this->username = username;
    this->password = password;
    this->fullName = fullName;
    this->email = email;
    this->phone = phone;
    this->role = role;
}

string User::getUsername() { return username; }
string User::getPassword() { return password; }
string User::getFullName() { return fullName; }
string User::getEmail() { return email; }
string User::getPhone() { return phone; }
string User::getRole() { return role; }

void User::setPassword(string newPassword) { password = newPassword; }
void User::setFullName(string newFullName) { fullName = newFullName; }
void User::setEmail(string newEmail) { email = newEmail; }
void User::setPhone(string newPhone) { phone = newPhone; }

void User::displayProfile() {
    cout << "\n=== PROFILE USER ===\n";
    cout << "Username    : " << username << "\n";
    cout << "Nama Lengkap: " << fullName << "\n";
    cout << "Email       : " << email << "\n";
    cout << "No. Telepon : " << phone << "\n";
    cout << "Role        : " << role << "\n";
    cout << "====================\n";
}

string User::toString() {
    return username + "|" + password + "|" + fullName + "|" + email + "|" + phone + "|" + role;
}

User User::fromString(string data) {
    stringstream ss(data);
    string username, password, fullName, email, phone, role;

    getline(ss, username, '|');
    getline(ss, password, '|');
    getline(ss, fullName, '|');
    getline(ss, email, '|');
    getline(ss, phone, '|');
    getline(ss, role, '|');

    return User(username, password, fullName, email, phone, role);
}
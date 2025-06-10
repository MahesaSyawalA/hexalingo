// user.h
#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User {
private:
    string username;
    string password;
    string fullName;
    string email;
    string phone;
    string role;

public:
    User();
    User(string username, string password, string fullName, 
         string email, string phone, string role = "user");

    string getUsername();
    string getPassword();
    string getFullName();
    string getEmail();
    string getPhone();
    string getRole();

    void setPassword(string newPassword);
    void setFullName(string newFullName);
    void setEmail(string newEmail);
    void setPhone(string newPhone);

    void displayProfile();
    string toString();
    static User fromString(string data);
};

#endif
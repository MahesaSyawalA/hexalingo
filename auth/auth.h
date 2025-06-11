// auth.h
#ifndef AUTH_H
#define AUTH_H

#include "user.h"

struct JsonUser
{
    int id;
    std::string username;
    std::string password;
    std::string name;
    std::string email;
    std::string phone;
    std::string role;
};

class Auth {
private:
    User currentUser;
    bool loggedIn = false;  
    void updateSession(const JsonUser &user);
    void clearSession();

public:
    bool isLoggedIn();
    bool loadSession(); 
    User* getCurrentUser();

    void loginUser();
    void registerUser();
    void viewProfile();
    void editProfile();
    void changePassword();
    void logout();
};

#endif
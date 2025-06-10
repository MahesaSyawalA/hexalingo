// auth.h
#ifndef AUTH_H
#define AUTH_H

#include "user.h"

class Auth {
private:
    User currentUser;
    bool loggedIn = false;

public:
    bool isLoggedIn();
    User* getCurrentUser();

    void loginUser();
    void registerUser();
    void viewProfile();
    void editProfile();
    void changePassword();
    void logout();
};

#endif
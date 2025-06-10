#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cctype>
#include "auth.h"

// Platform-specific includes for password masking
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

// Cross-platform password input with asterisks
string inputPassword()
{
    string password;
    char ch;

    cout << "Password: ";

#ifdef _WIN32
    while ((ch = _getch()) != '\r')
    {
        if (ch == '\b')
        { // backspace
            if (!password.empty())
            {
                cout << "\b \b";
                password.pop_back();
            }
        }
        else
        {
            password += ch;
            cout << '*';
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((ch = getchar()) != '\n')
    {
        if (ch == 127 || ch == 8)
        { // backspace
            if (!password.empty())
            {
                cout << "\b \b";
                password.pop_back();
            }
        }
        else
        {
            password += ch;
            cout << '*';
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    cout << endl;
    return password;
}

// Function to validate phone number (only digits allowed)
bool isValidPhoneNumber(const string &phone)
{
    if (phone.empty())
        return false;
    for (char c : phone)
    {
        if (!isdigit(c))
        {
            return false;
        }
    }
    return true;
}

// Function to escape JSON strings
string escapeJson(const string &str)
{
    string escaped;
    for (char c : str)
    {
        switch (c)
        {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}

// Simple JSON parser for our specific format
struct JsonUser
{
    int id;
    string username;
    string password;
    string name;
    string email;
    string phone;
    string role;
};

vector<JsonUser> parseJsonUsers(const string &content)
{
    cout << "[DEBUG] JSON masuk ke parser:\n"
         << content << endl;
    vector<JsonUser> users;
    size_t pos = content.find("\"users\"");
    if (pos == string::npos)
        return users;

    pos = content.find('[', pos);
    if (pos == string::npos)
        return users;

    size_t start = pos + 1;
    size_t end = content.find(']', start);
    if (end == string::npos)
        return users;

    string usersSection = content.substr(start, end - start);

    // Parse each user object
    size_t objStart = 0;
    while ((objStart = usersSection.find('{', objStart)) != string::npos)
    {
        size_t objEnd = usersSection.find('}', objStart);
        if (objEnd == string::npos)
            break;

        string userObj = usersSection.substr(objStart + 1, objEnd - objStart - 1);
        JsonUser user;

        // Parse fields
        size_t fieldPos = 0;
        while (fieldPos < userObj.length())
        {
            size_t keyStart = userObj.find('"', fieldPos);
            if (keyStart == string::npos)
                break;

            size_t keyEnd = userObj.find('"', keyStart + 1);
            if (keyEnd == string::npos)
                break;

            string key = userObj.substr(keyStart + 1, keyEnd - keyStart - 1);

            size_t colonPos = userObj.find(':', keyEnd);
            if (colonPos == string::npos)
                break;

            size_t valueStart = userObj.find_first_not_of(" \t", colonPos + 1);
            if (valueStart == string::npos)
                break;

            string value;
            if (userObj[valueStart] == '"')
            {
                size_t valueEnd = userObj.find('"', valueStart + 1);
                if (valueEnd == string::npos)
                    break;
                value = userObj.substr(valueStart + 1, valueEnd - valueStart - 1);
                fieldPos = valueEnd + 1;
            }
            else
            {
                size_t valueEnd = userObj.find_first_of(",}", valueStart);
                if (valueEnd == string::npos)
                    valueEnd = userObj.length();
                value = userObj.substr(valueStart, valueEnd - valueStart);
                // Remove trailing whitespace
                while (!value.empty() && isspace(value.back()))
                {
                    value.pop_back();
                }
                fieldPos = valueEnd;
            }

            // Assign values to user struct
            if (key == "id")
                user.id = stoi(value);
            else if (key == "username")
                user.username = value;
            else if (key == "password")
                user.password = value;
            else if (key == "name")
                user.name = value;
            else if (key == "email")
                user.email = value;
            else if (key == "phone")
                user.phone = value;
            else if (key == "role")
                user.role = value;
            // Abaikan field lainnya seperti "matakuliah"
        }

        users.push_back(user);
        objStart = objEnd + 1;
    }

    return users;
}

string generateJsonFromUsers(const vector<JsonUser> &users)
{
    string json = "{\n    \"users\": [\n";

    for (size_t i = 0; i < users.size(); i++)
    {
        const JsonUser &user = users[i];
        json += "        {\n";
        json += "            \"id\": " + to_string(user.id) + ",\n";
        json += "            \"username\": \"" + escapeJson(user.username) + "\",\n";
        json += "            \"password\": \"" + escapeJson(user.password) + "\",\n";
        json += "            \"name\": \"" + escapeJson(user.name) + "\",\n";
        json += "            \"email\": \"" + escapeJson(user.email) + "\",\n";
        json += "            \"phone\": \"" + escapeJson(user.phone) + "\",\n";
        json += "            \"role\": \"" + escapeJson(user.role) + "\"\n";
        // json += "            \"matakuliah\": []\n";
        json += "        }";

        if (i < users.size() - 1)
        {
            json += ",";
        }
        json += "\n";
    }

    json += "    ]\n}";
    return json;
}

bool Auth::isLoggedIn()
{
    return loggedIn;
}

User *Auth::getCurrentUser()
{
    return &currentUser;
}

void Auth::viewProfile()
{
    if (loggedIn)
    {
        currentUser.displayProfile();
    }
    else
    {
        cout << "Anda belum login!\n";
    }
}

void Auth::loginUser()
{
    string uname, pass;
    cout << "Masukkan username: ";
    cin >> uname;
    pass = inputPassword();

    // Read database.json
    ifstream inFile("database.json");
    if (!inFile.is_open())
    {
        cout << "Gagal membuka database.json\n";
        return;
    }

    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    vector<JsonUser> users = parseJsonUsers(content);

    bool found = false;
    for (const JsonUser &user : users)
    {
        if (user.username == uname && user.password == pass)
        {
            currentUser = User(user.username, user.password, user.name, user.email, user.phone, user.role);
            loggedIn = true;
            found = true;
            break;
        }
    }

    if (found)
    {
        cout << "Login berhasil!\n";
    }
    else
    {
        cout << "Login gagal. Username atau password salah.\n";
    }
}

// void Auth::registerUser() {
//     string uname, pass, nama, email, phone;
//     cout << "Username: ";
//     cin >> uname;

//     pass = inputPassword();

//     cin.ignore();
//     cout << "Nama lengkap: ";
//     getline(cin, nama);
//     cout << "Email: ";
//     getline(cin, email);

//     // Validate phone number
//     do {
//         cout << "No. Telepon (hanya angka): ";
//         getline(cin, phone);
//         if (!isValidPhoneNumber(phone)) {
//             cout << "No. telepon hanya boleh berisi angka!\n";
//         }
//     } while (!isValidPhoneNumber(phone));

//     // Read existing database
//     vector<JsonUser> users;
//     ifstream inFile("database.json");
//     if (inFile.is_open()) {
//         string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
//         inFile.close();
//         users = parseJsonUsers(content);
//     }

//     // Check if username already exists
//     for (const JsonUser& user : users) {
//         if (user.username == uname) {
//             cout << "Username sudah terdaftar!\n";
//             return;
//         }
//     }

//     // Find max ID
//     int maxId = 0;
//     for (const JsonUser& user : users) {
//         maxId = max(maxId, user.id);
//     }

//     // Create new user
//     JsonUser newUser;
//     newUser.id = maxId + 1;
//     newUser.username = uname;
//     newUser.password = pass;
//     newUser.name = nama;
//     newUser.email = email;
//     newUser.phone = phone;
//     newUser.role = "user";

//     users.push_back(newUser);

//     // Save to database.json
//     ofstream outFile("database.json");
//     if (outFile.is_open()) {
//         outFile << generateJsonFromUsers(users);
//         outFile.close();
//         cout << "Registrasi berhasil!\n";

//         // Auto login after registration
//         currentUser = User(uname, pass, nama, email, phone, "user");
//         loggedIn = true;
//     } else {
//         cout << "Gagal Register.\n";
//     }
// }

void Auth::registerUser()
{
    string uname, pass, nama, email, phone;
    cout << "Username: ";
    cin >> uname;
    pass = inputPassword();

    cin.ignore();
    cout << "Nama lengkap: ";
    getline(cin, nama);
    cout << "Email: ";
    getline(cin, email);

    do
    {
        cout << "No. Telepon (hanya angka): ";
        getline(cin, phone);
        if (!isValidPhoneNumber(phone))
        {
            cout << "No. telepon hanya boleh berisi angka!\n";
        }
    } while (!isValidPhoneNumber(phone));

    vector<JsonUser> users;
    ifstream inFile("database.json");
    if (inFile.is_open())
    {
        string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        inFile.close();
        users = parseJsonUsers(content);

        cout << "[DEBUG] Jumlah user sebelum register: " << users.size() << endl;
    }

    for (const JsonUser &user : users)
    {
        if (user.username == uname)
        {
            cout << "Username sudah terdaftar!\n";
            return;
        }
    }

    int maxId = 0;
    for (const JsonUser &user : users)
    {
        maxId = max(maxId, user.id);
    }

    JsonUser newUser{maxId + 1, uname, pass, nama, email, phone, "user"};
    users.push_back(newUser);
    cout << "[DEBUG] Jumlah user setelah register: " << users.size() << endl;

    ofstream outFile("database.json");
    if (outFile.is_open())
    {
        string jsonOutput = generateJsonFromUsers(users);
        cout << "\n[DEBUG] Menyimpan ke database.json:\n"
             << jsonOutput << endl;
        outFile << jsonOutput;
        outFile.close();
        cout << "Registrasi berhasil!\n";

        currentUser = User(uname, pass, nama, email, phone, "user");
        loggedIn = true;
    }
    else
    {
        cout << "Gagal membuka database.json untuk menyimpan.\n";
    }
}

void Auth::editProfile()
{
    if (!loggedIn)
    {
        cout << "Anda belum login!\n";
        return;
    }

    string nama, email, phone;
    cin.ignore();
    cout << "Nama baru (current: " << currentUser.getFullName() << "): ";
    getline(cin, nama);
    cout << "Email baru (current: " << currentUser.getEmail() << "): ";
    getline(cin, email);

    // Validate phone number
    do
    {
        cout << "Telepon baru (current: " << currentUser.getPhone() << ", hanya angka): ";
        getline(cin, phone);
        if (!phone.empty() && !isValidPhoneNumber(phone))
        {
            cout << "No. telepon hanya boleh berisi angka!\n";
        }
    } while (!phone.empty() && !isValidPhoneNumber(phone));

    // Update current user object (only if not empty)
    if (!nama.empty())
        currentUser.setFullName(nama);
    if (!email.empty())
        currentUser.setEmail(email);
    if (!phone.empty())
        currentUser.setPhone(phone);

    // Update database.json
    ifstream inFile("database.json");
    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    vector<JsonUser> users = parseJsonUsers(content);

    for (JsonUser &user : users)
    {
        if (user.username == currentUser.getUsername())
        {
            user.name = currentUser.getFullName();
            user.email = currentUser.getEmail();
            user.phone = currentUser.getPhone();
            break;
        }
    }

    ofstream outFile("database.json");
    outFile << generateJsonFromUsers(users);
    outFile.close();

    cout << "Profil berhasil diperbarui!\n";
}

void Auth::changePassword()
{
    if (!loggedIn)
    {
        cout << "Anda belum login!\n";
        return;
    }

    string oldPass, newPass;
    cout << "Masukkan password lama: ";
    cin >> oldPass;

    if (oldPass != currentUser.getPassword())
    {
        cout << "Password lama salah.\n";
        return;
    }

    cout << "Masukkan password baru: ";
    cin >> newPass;

    currentUser.setPassword(newPass);

    // Update database.json
    ifstream inFile("database.json");
    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    vector<JsonUser> users = parseJsonUsers(content);

    for (JsonUser &user : users)
    {
        if (user.username == currentUser.getUsername())
        {
            user.password = newPass;
            break;
        }
    }

    ofstream outFile("database.json");
    outFile << generateJsonFromUsers(users);
    outFile.close();

    cout << "Password berhasil diubah!\n";
}

void Auth::logout()
{
    loggedIn = false;
    cout << "Berhasil logout.\n";
}
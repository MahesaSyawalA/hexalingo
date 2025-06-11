#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cctype>
#include "auth.h"
#include "../json.hpp"
using json = nlohmann::json;


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
// struct JsonUser
// {
//     int id;
//     string username;
//     string password;
//     string name;
//     string email;
//     string phone;
//     string role;
// };

vector<JsonUser> parseJsonUsers(const string &jsonStr)
{
    vector<JsonUser> result;

    try
    {
        json jsonData = json::parse(jsonStr);

        // Pastikan bagian "users" ada dan berupa array
        if (!jsonData.contains("users") || !jsonData["users"].is_array())
        {
            cout << "Data 'users' tidak ditemukan atau bukan array." << endl;
            return result;
        }

        for (auto &item : jsonData["users"])
        {
            JsonUser user;
            user.id = item["id"];
            user.username = item["username"];
            user.password = item["password"];
            user.name = item["name"];
            user.email = item["email"];
            user.phone = item["phone"];
            user.role = item["role"];

            // Kalau kamu punya field tambahan seperti id atau matakuliah:
            // user.id = item["id"];
            // user.matakuliah = item["matakuliah"].get<vector<int>>();

            result.push_back(user);
        }
    }
    catch (const std::exception &e)
    {
        cout << "Gagal parsing JSON: " << e.what() << endl;
    }

    return result;
}

string generateJsonFromUsers(const vector<JsonUser> &users) {
    string json = "{\n";
    json += "    \"matakuliah\": [\n";
    // Tambahkan data matakuliah yang ada di file asli
    json += "        { \"id\": 1, \"nama\": \"Matematika Diskrit\" },\n";
    json += "        { \"id\": 2, \"nama\": \"Desain Berpikir\" },\n";
    // ... tambahkan semua matakuliah
    json += "    ],\n";
    
    json += "    \"users\": [\n";
    for (size_t i = 0; i < users.size(); i++) {
        const JsonUser &user = users[i];
        json += "        {\n";
        json += "            \"id\": " + to_string(user.id) + ",\n";
        json += "            \"username\": \"" + escapeJson(user.username) + "\",\n";
        json += "            \"password\": \"" + escapeJson(user.password) + "\",\n";
        json += "            \"name\": \"" + escapeJson(user.name) + "\",\n";
        json += "            \"email\": \"" + escapeJson(user.email) + "\",\n";
        json += "            \"phone\": \"" + escapeJson(user.phone) + "\",\n";
        json += "            \"role\": \"" + escapeJson(user.role) + "\",\n";
        json += "            \"matakuliah\": []\n"; // Default empty array
        json += "        }";
        if (i < users.size() - 1) json += ",";
        json += "\n";
    }
    json += "    ],\n";
    
    // Tambahkan bagian tugas dan jawaban kosong
    json += "    \"tugas\": [],\n";
    json += "    \"jawaban\": [],\n";
    json += "    \"session\": {}\n";
    json += "}";
    
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

void Auth::updateSession(const JsonUser &user)
{
    // Baca database.json
    ifstream inFile("database.json");
    if (!inFile.is_open())
    {
        cout << "Gagal membuka database.json untuk update session\n";
        return;
    }

    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // Cari posisi session dalam JSON
    size_t sessionPos = content.find("\"session\"");
    if (sessionPos == string::npos)
    {
        // Jika tidak ada session, tambahkan sebelum closing brace terakhir
        size_t lastBrace = content.rfind('}');
        if (lastBrace != string::npos)
        {
            string sessionData = ",\n    \"session\": {\n";
            sessionData += "        \"user_id\": " + to_string(user.id) + ",\n";
            sessionData += "        \"nama\": \"" + escapeJson(user.name) + "\",\n";
            sessionData += "        \"role\": \"" + escapeJson(user.role) + "\",\n";
            sessionData += "        \"username\": \"" + escapeJson(user.username) + "\"\n";
            sessionData += "    }";
            
            content.insert(lastBrace, sessionData);
        }
    }
    else
    {
        // Jika sudah ada session, update datanya
        size_t sessionStart = content.find('{', sessionPos);
        size_t sessionEnd = content.find('}', sessionStart);
        
        if (sessionStart != string::npos && sessionEnd != string::npos)
        {
            string newSessionData = "{\n";
            newSessionData += "        \"user_id\": " + to_string(user.id) + ",\n";
            newSessionData += "        \"nama\": \"" + escapeJson(user.name) + "\",\n";
            newSessionData += "        \"role\": \"" + escapeJson(user.role) + "\",\n";
            newSessionData += "        \"username\": \"" + escapeJson(user.username) + "\"\n";
            newSessionData += "    }";
            
            content.replace(sessionStart, sessionEnd - sessionStart + 1, newSessionData);
        }
    }

    // Simpan kembali ke file
    ofstream outFile("database.json");
    if (outFile.is_open())
    {
        outFile << content;
        outFile.close();
        cout << "Session berhasil disimpan untuk user: " << user.name << endl;
    }
    else
    {
        cout << "Gagal menyimpan session ke database.json\n";
    }
}

// Fungsi untuk membersihkan session saat logout
void Auth::clearSession()
{
    ifstream inFile("database.json");
    if (!inFile.is_open())
    {
        return;
    }

    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // Cari dan hapus/kosongkan session
    size_t sessionPos = content.find("\"session\"");
    if (sessionPos != string::npos)
    {
        size_t sessionStart = content.find('{', sessionPos);
        size_t sessionEnd = content.find('}', sessionStart);
        
        if (sessionStart != string::npos && sessionEnd != string::npos)
        {
            string emptySession = "{\n        \"user_id\": null,\n        \"nama\": null,\n        \"role\": null\n    }";
            content.replace(sessionStart, sessionEnd - sessionStart + 1, emptySession);
        }
    }

    ofstream outFile("database.json");
    if (outFile.is_open())
    {
        outFile << content;
        outFile.close();
    }
}

// Fungsi untuk load session saat program dimulai
bool Auth::loadSession()
{
    ifstream inFile("database.json");
    if (!inFile.is_open())
    {
        return false;
    }

    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // Parse session data
    size_t sessionPos = content.find("\"session\"");
    if (sessionPos == string::npos)
        return false;

    size_t sessionStart = content.find('{', sessionPos);
    size_t sessionEnd = content.find('}', sessionStart);
    
    if (sessionStart == string::npos || sessionEnd == string::npos)
        return false;

    string sessionData = content.substr(sessionStart + 1, sessionEnd - sessionStart - 1);
    
    // Parse session fields
    int userId = 0;
    string nama, role;
    
    // Simple parsing untuk session data
    size_t pos = 0;
    while (pos < sessionData.length())
    {
        size_t keyStart = sessionData.find('"', pos);
        if (keyStart == string::npos) break;
        
        size_t keyEnd = sessionData.find('"', keyStart + 1);
        if (keyEnd == string::npos) break;
        
        string key = sessionData.substr(keyStart + 1, keyEnd - keyStart - 1);
        
        size_t colonPos = sessionData.find(':', keyEnd);
        if (colonPos == string::npos) break;
        
        size_t valueStart = sessionData.find_first_not_of(" \t", colonPos + 1);
        if (valueStart == string::npos) break;
        
        string value;
        if (sessionData[valueStart] == '"')
        {
            size_t valueEnd = sessionData.find('"', valueStart + 1);
            if (valueEnd == string::npos) break;
            value = sessionData.substr(valueStart + 1, valueEnd - valueStart - 1);
            pos = valueEnd + 1;
        }
        else if (sessionData.substr(valueStart, 4) == "null")
        {
            pos = valueStart + 4;
            continue; // Skip null values
        }
        else
        {
            size_t valueEnd = sessionData.find_first_of(",}", valueStart);
            if (valueEnd == string::npos) valueEnd = sessionData.length();
            value = sessionData.substr(valueStart, valueEnd - valueStart);
            
            while (!value.empty() && isspace(value.back()))
                value.pop_back();
            pos = valueEnd;
        }
        
        if (key == "user_id" && !value.empty())
            userId = stoi(value);
        else if (key == "nama")
            nama = value;
        else if (key == "role")
            role = value;
    }
    
    // Jika ada session yang valid, load user data
    if (userId > 0 && !nama.empty() && !role.empty())
    {
        // Load full user data from users array
        vector<JsonUser> users = parseJsonUsers(content);
        for (const JsonUser &user : users)
        {
            if (user.id == userId)
            {
                currentUser = User(user.username, user.password, user.name, user.email, user.phone, user.role);
                loggedIn = true;
                cout << "Session ditemukan. Selamat datang kembali, " << nama << "!" << endl;
                return true;
            }
        }
    }
    
    return false;
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

    // Baca database.json
    ifstream inFile("database.json");
    if (!inFile.is_open())
    {
        cout << "Gagal membuka database.json\n";
        return;
    }

    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    vector<JsonUser> users = parseJsonUsers(content);

    bool loginSuccess = false; // Flag untuk menandakan status login

    // Loop mencari user yang cocok
    for (const JsonUser &user : users)
    {
        // cout << "Memeriksa user: " << user.username << " dengan password: " << user.password << endl;
 // Menampilkan username yang sedang diperiksa

        // Periksa apakah username dan password cocok
        if (user.username == uname && user.password == pass)
        {
            currentUser = User(user.username, user.password, user.name, user.email, user.phone, user.role);
            loggedIn = true;
            loginSuccess = true;

            updateSession(user);

            cout << "Login berhasil! Selamat datang, " << user.name << endl;
            break;
        }
    }

    // if (!loginSuccess)
    // {
    //     cout << "Login gagal. Username atau password salah.\n";
    // }
}

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

    // Baca seluruh file JSON
    ifstream inFile("database.json");
    if (!inFile.is_open())
    {
        cout << "Gagal membuka database.json\n";
        return;
    }

    string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    try
    {
        // Parse JSON menggunakan nlohmann::json
        json jsonData = json::parse(content);
        
        // Pastikan array users ada
        if (!jsonData.contains("users") || !jsonData["users"].is_array())
        {
            cout << "Format database.json tidak valid - array 'users' tidak ditemukan\n";
            return;
        }

        // Cek apakah username sudah ada
        for (auto &user : jsonData["users"])
        {
            if (user.contains("username") && user["username"] == uname)
            {
                cout << "Username sudah terdaftar!\n";
                return;
            }
        }

        // Cari ID tertinggi untuk user baru
        int maxId = 0;
        for (auto &user : jsonData["users"])
        {
            if (user.contains("id") && user["id"].is_number())
            {
                maxId = max(maxId, (int)user["id"]);
            }
        }

        // Buat user baru
        json newUser;
        newUser["id"] = maxId + 1;
        newUser["username"] = uname;
        newUser["password"] = pass;
        newUser["name"] = nama;
        newUser["email"] = email;
        newUser["phone"] = phone;
        newUser["role"] = "user";
        newUser["matakuliah"] = json::array(); // Array kosong untuk matakuliah

        // Tambahkan user baru ke array users
        jsonData["users"].push_back(newUser);

        cout << "[DEBUG] Jumlah user setelah register: " << jsonData["users"].size() << endl;

        // Simpan kembali ke file dengan format yang rapi
        ofstream outFile("database.json");
        if (outFile.is_open())
        {
            outFile << jsonData.dump(4); // Pretty print dengan indentasi 4 spasi
            outFile.close();
            cout << "Registrasi berhasil!\n";

            // Set user sebagai logged in
            currentUser = User(uname, pass, nama, email, phone, "user");
            loggedIn = true;

            // Update session dengan user baru
            JsonUser sessionUser{maxId + 1, uname, pass, nama, email, phone, "user"};
            updateSession(sessionUser);
        }
        else
        {
            cout << "Gagal membuka database.json untuk menyimpan.\n";
        }
    }
    catch (const json::parse_error &e)
    {
        cout << "Error parsing JSON: " << e.what() << endl;
        return;
    }
    catch (const std::exception &e)
    {
        cout << "Error: " << e.what() << endl;
        return;
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
    clearSession();
    loggedIn = false;
    cout << "Berhasil logout.\n";
}
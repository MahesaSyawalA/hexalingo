#include <iostream>
#include <vector>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

void Leaderboard() {
    ifstream inputFile("database.json");
    if (!inputFile) {
        cerr << "Failed to open file.\n";
        return;
    }

    json data;
    inputFile >> data;

    map<int, tuple<string, string, string>> userInfo;
    map<int, pair<double, int>> userScores;           

    
    for (const auto& user : data["users"]) {
        int id = user["id"];
        userInfo[id] = make_tuple(
            user["username"].get<string>(),
            user["name"].get<string>(),
            user["role"].get<string>()
        );
    }

    
    for (const auto& answer : data["jawaban"]) {
        int userId = answer["user_id"];
        double score = answer["nilai"];

        userScores[userId].first += score;
        userScores[userId].second += 1;
    }

    
    vector<tuple<int, string, string, string, double>> leaderboard;

    for (map<int, pair<double, int>>::const_iterator it = userScores.begin(); it != userScores.end(); ++it) {
        int userId = it->first;
        double totalScore = it->second.first;
        int submissionCount = it->second.second;

        if (submissionCount == 0) continue;

        tuple<string, string, string> userTuple = userInfo[userId];
        string username = get<0>(userTuple);
        string name = get<1>(userTuple);
        string role = get<2>(userTuple);

        if (role == "admin") continue; 

        double avg = totalScore / submissionCount;
        leaderboard.push_back(make_tuple(userId, username, name, role, avg));
    }

    
    sort(leaderboard.begin(), leaderboard.end(), [](const tuple<int, string, string, string, double>& a,
                                                    const tuple<int, string, string, string, double>& b) {
        return get<4>(a) > get<4>(b);
    });

    
    cout << left << setw(6) << "Rank"
         << setw(15) << "Username"
         << setw(20) << "Nama"
         << setw(10) << "Role"
         << setw(15) << "Rata-Rata (%)" << "\n";

    cout << string(70, '-') << "\n";

    int rank = 1;
    for (vector<tuple<int, string, string, string, double>>::const_iterator it = leaderboard.begin(); it != leaderboard.end(); ++it) {
        cout << setw(6) << rank++
             << setw(15) << get<1>(*it)  
             << setw(20) << get<2>(*it)  
             << setw(10) << get<3>(*it)  
             << fixed << setprecision(2)
             << setw(15) << get<4>(*it)  
             << "\n";
    }
}

int 
mainLeaderboard(){
    Leaderboard();

    return 0;
}

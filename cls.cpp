#include <cstdlib>   // std::system
#include <limits>    // numeric_limits
#include <iostream>

/*  Membersihkan tampilan terminal / cmd  */
void clearCls() {
#ifdef _WIN32
    std::system("cls");   // Windows
#else
    std::system("clear"); // Linux / macOS
#endif
}

/*  Stop sejenak agar user sempat membaca output  */
void pause() {
    std::cout << "\nTekan <Enter> untuk melanjutkan...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}
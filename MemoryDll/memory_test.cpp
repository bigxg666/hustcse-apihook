#include <iostream>
#include <cstring>

int main() {
    char source[] = "Hello, world!";
    char destination[20];

    // µ÷ÓÃ memcpy º¯Êý¿½±´×Ö·û´®
    memcpy(destination, source, strlen(source) + 1);
    std::cout << "" << strlen(source) + 1 << std::endl;
    // Êä³ö¿½±´ºóµÄ×Ö·û´®
    std::cout << "Copied string: " << destination << std::endl;

    return 0;
}

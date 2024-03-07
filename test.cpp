#include <iostream>
#include <cstring>

FILE *filePointer;
int main()
{
    filePointer = fopen("google-10000-english-no-swears.txt", "r");
    char listWords[5000];
    std::cout << fgets(listWords, 100, filePointer);
    
    std::cout << fgets(listWords, 100, filePointer);
    
    std::cout << fgets(listWords, 100, filePointer);
    fclose(filePointer);
    return 0;
}
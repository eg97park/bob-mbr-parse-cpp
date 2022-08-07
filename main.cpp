#include <iostream>

int main(int argc, char* argv[]){
    if (argc != 2){
        std::cout << "usage: " << argv[0] << " [IMAGE_FILE]" << std::endl;
        return 1;
    }
    return 0;
}
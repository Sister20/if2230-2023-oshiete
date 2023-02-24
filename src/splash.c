#include "lib-header/framebuffer.h"
#include "lib-header/splash.h"

void splash(){
    char * asciiart = 
        "\n"
        "  /$$$$$$   /$$$$$$  /$$       /$$             /$$              \n" 
        " /$$__  $$ /$$__  $$| $$      |__/            | $$              \n"
        "| $$  \\ $$| $$  \\__/| $$$$$$$  /$$  /$$$$$$  /$$$$$$    /$$$$$$ \n"
        "| $$  | $$|  $$$$$$ | $$__  $$| $$ /$$__  $$|_  $$_/   /$$__  $$\n"
        "| $$  | $$ \\____  $$| $$  \\ $$| $$| $$$$$$$$  | $$    | $$$$$$$$\n"
        "| $$  | $$ /$$  \\ $$| $$  | $$| $$| $$_____/  | $$ /$$| $$_____/\n"
        "|  $$$$$$/|  $$$$$$/| $$  | $$| $$|  $$$$$$$  |  $$$$/|  $$$$$$$\n"
        " \\______/  \\______/ |__/  |__/|__/ \\_______/   \\___/   \\_______/\n"
        "\n";

    int i = 5;
    int j = 8;
    char* ptr = asciiart;
    while (*ptr != '\0') {
        // Set the framebuffer with the current character
        if (*ptr == '\n'){
            i++;
            j=8;
        } else {
            framebuffer_write(i, j, *ptr, 0x01, 0x03);
            j++;
        }
        ptr++;
    }
}
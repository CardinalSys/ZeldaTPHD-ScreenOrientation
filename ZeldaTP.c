#include "proc.c"

char* ExtractBaseAddress(const char* text) {
    const char* basePos = strstr(text, "base:");
    if (basePos == NULL) {
        return NULL;
    }

    basePos += strlen("base:");

    while (*basePos == ' ' || *basePos == '\t') {
        basePos++;
    }

    const char* endPos = basePos;
    while (*endPos != '\0' && *endPos != '\n') {
        endPos++;
    }

    size_t length = endPos - basePos - 1;

    char* address = (char*)malloc(length + 1);
    if (address == NULL) {
        return NULL;
    }

    strncpy(address, basePos, length);
    address[length] = '\0';

    return address;
}

int GetBaseAddressFromText(const char* path, char* address){
    printf("%s\n", path);
    char logPath[500];

    const char* file = strstr(path, "Cemu.exe");

    if(file == NULL){
        printf("Da fack?");
    }

    size_t prefixLength = file - path;

    strncpy(logPath, path, prefixLength);

    strcpy(logPath + prefixLength, "log.txt");

    printf("%s\n", logPath);


    FILE *fptr;

    fptr = fopen(logPath, "r");

    if(fptr == NULL){
        printf("Cemu log.txt file not found\n");
    }else{
        printf("Cemu log.txt file found, getting base address...\n");
    }

    char text[500];
    char* baseAddress = NULL;
    
    if(fptr != NULL) {
        int i = 0;
        // Read the content and print it
        while (fgets(text, sizeof(text), fptr)) {
            if (i > 2) {
                break;
            }
            baseAddress = ExtractBaseAddress(text);
            if (baseAddress != NULL) {
                strcpy(address, baseAddress);
                free(baseAddress);
                break;
            }
            i++;
        }
    }
    fclose(fptr);

    return 0;
}



void CheckScreenStatus(HANDLE handle, LPCVOID baseAddress){
    byte status;
    SIZE_T bytesRead;

    if(ReadProcessMemory(handle, baseAddress, &status, sizeof(status), &bytesRead)){
        if(status == 0){
            printf("\ncurrent status %x: Wii Mode \n", status);
        }else if (status == 1){
            printf("\ncurrent status %x: Wiiu/GameCube Mode \n", status);
        }
    }
}

void ChangeScreenStatus(HANDLE handle, LPCVOID baseAddress) {
    int newValue = -1;
    SIZE_T bytesWritten;

    while (newValue < 0 || newValue > 1) {
        printf("\nWrite 0 for Wii mode or 1 for WiiU/GameCube Mode\n");

        if (scanf("%d", &newValue) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            newValue = -1;
        } else if (newValue < 0 || newValue > 1) {
            printf("Invalid value. Please enter 0 or 1.\n");
        }
    }

    if (WriteProcessMemory(handle, baseAddress, &newValue, sizeof(newValue), &bytesWritten)) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 9);
        if (newValue == 0) {    
            printf("Changed to Wii mode\a\n");
        } else if (newValue == 1) {
            printf("Changed to WiiU/GameCube mode\a\n");
        }
    } else {
        printf("Error writing address: %lu\n", GetLastError());
    }
}



int main(void) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 8);
    struct Process proc = GetProcessByName("Cemu.exe");

    if (proc.pid != 0) {
        char baseAddressStr[100];
        GetBaseAddressFromText(proc.fullPath, baseAddressStr);

        if (strlen(baseAddressStr) > 0) {
            printf("Base Address: %s\n", baseAddressStr);
            
            LPCVOID baseAddress = (LPCVOID)(strtoull(baseAddressStr, NULL, 16) + 0x1012667D);
            printf("Screen orientation address 0x%p\n", baseAddress);


            while(TRUE){
                SetConsoleTextAttribute(hConsole, 12);
                CheckScreenStatus(proc.handle, baseAddress);
                ChangeScreenStatus(proc.handle, baseAddress);
            }


            
        } else {
            printf("Base Address not found.\n");
        }



        CloseHandle(proc.handle);
    } else {
        printf("Process not found.\n");
    }
    return 0;
}
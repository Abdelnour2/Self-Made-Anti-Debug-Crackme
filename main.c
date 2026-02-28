#include <windows.h>
#include <stdio.h>

int check_password(int input) {
    // Logic: (input XOR 0x1337) + 100
    int secret = (input ^ 0x1337) + 100;
    return secret;
}

int main() {
    int user_input;

    if (IsDebuggerPresent()) {
        printf("DEBUGGER DETECTED! Exiting for security...\n");
        Sleep(2000);
        return 1; 
    }

    printf("--- Secure Vault v1.0 ---\n");
    printf("Enter Access Code: ");
    
    if (scanf("%d", &user_input) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    if (check_password(user_input) == 6000) {
        printf("ACCESS GRANTED. Welcome, Operator.\n");
    } else {
        printf("ACCESS DENIED.\n");
    }

    printf("Press Enter to exit...");
    getchar(); getchar();
    return 0;
}
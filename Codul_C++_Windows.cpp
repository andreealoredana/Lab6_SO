#include <iostream>
#include <windows.h>
#include <vector>
#include <cmath>

bool isPrime(int num) {
    if (num < 2) return false;
    for (int i = 2; i <= std::sqrt(num); i++)
        if (num % i == 0) return false;
    return true;
}

void findPrimesInRange(int start, int end, HANDLE writePipe) {
    std::vector<int> primes;
    for (int i = start; i <= end; i++) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }
    DWORD written;
    WriteFile(writePipe, primes.data(), primes.size() * sizeof(int), &written, NULL);
    CloseHandle(writePipe);
}

int main() {
    const int NUM_PROCESSES = 10;
    const int RANGE_SIZE = 1000;

    HANDLE pipes[NUM_PROCESSES][2];
    PROCESS_INFORMATION pInfo[NUM_PROCESSES];
    STARTUPINFO sInfo[NUM_PROCESSES];

    for (int i = 0; i < NUM_PROCESSES; i++) {
        SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        if (!CreatePipe(&pipes[i][0], &pipes[i][1], &sa, 0)) {
            std::cerr << "Error creating pipe\n";
            return 1;
        }

        ZeroMemory(&pInfo[i], sizeof(PROCESS_INFORMATION));
        ZeroMemory(&sInfo[i], sizeof(STARTUPINFO));
        sInfo[i].cb = sizeof(STARTUPINFO);

        std::string cmd = "child_process.exe " + std::to_string(i * RANGE_SIZE + 1) + " " + std::to_string((i + 1) * RANGE_SIZE);
        if (!CreateProcess(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &sInfo[i], &pInfo[i])) {
            std::cerr << "Error creating process\n";
            return 1;
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        CloseHandle(pipes[i][1]); // Close write end
        std::vector<int> primes(1000);
        DWORD read;
        ReadFile(pipes[i][0], primes.data(), 1000 * sizeof(int), &read, NULL);
        CloseHandle(pipes[i][0]);
        std::cout << "Primes from process " << i + 1 << ": ";
        for (int prime : primes) {
            if (prime != 0) std::cout << prime << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}

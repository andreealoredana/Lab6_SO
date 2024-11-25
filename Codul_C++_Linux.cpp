#include <iostream>
#include <unistd.h>
#include <vector>
#include <cmath>

bool isPrime(int num) {
    if (num < 2) return false;
    for (int i = 2; i <= std::sqrt(num); i++)
        if (num % i == 0) return false;
    return true;
}

void findPrimesInRange(int start, int end, int writePipe) {
    std::vector<int> primes;
    for (int i = start; i <= end; i++) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }
    write(writePipe, primes.data(), primes.size() * sizeof(int));
    close(writePipe);
}

int main() {
    const int NUM_PROCESSES = 10;
    const int RANGE_SIZE = 1000;

    int pipes[NUM_PROCESSES][2];
    pid_t pids[NUM_PROCESSES];

    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) {
            std::cerr << "Error creating pipe\n";
            return 1;
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            std::cerr << "Error forking process\n";
            return 1;
        }

        if (pids[i] == 0) {  // Child process
            close(pipes[i][0]); // Close read end
            int start = i * RANGE_SIZE + 1;
            int end = (i + 1) * RANGE_SIZE;
            findPrimesInRange(start, end, pipes[i][1]);
            return 0;
        }
    }

    // Parent process
    for (int i = 0; i < NUM_PROCESSES; i++) {
        close(pipes[i][1]); // Close write end
        std::vector<int> primes(1000);
        read(pipes[i][0], primes.data(), 1000 * sizeof(int));
        close(pipes[i][0]);
        std::cout << "Primes from process " << i + 1 << ": ";
        for (int prime : primes) {
            if (prime != 0) std::cout << prime << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

std::string currentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::ctime(&in_time_t); // Convert time_t to string
    return ss.str();
}

bool isIPReachable(const std::string& ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;

    if (sock < 0) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(80); // HTTP port
    if(inet_pton(AF_INET, ip.c_str(), &address.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported.\n";
        return false;
    }

    // Connect to the specified IP
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(sock);
        return false; // IP is unreachable
    }

    close(sock);
    return true; // IP is reachable
}

void usage(char* programName) {
    std::cout << "Usage: " << programName << " <IP address> </path/to/script/when-is-down>\n";
    std::cout << "Example: " << programName << "142.250.180.78 /home/user/restart-network-manager.sh\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    std::string ipAddress = argv[1];
    std::string scriptPath = argv[2];
    std::string logPath = "~/is-my-internet-down.log";

    std::ofstream logFile(logPath, std::ios::app);
    bool wasReachable = false;

    while (true) {
        bool isReachable = isIPReachable(ipAddress);

        if (isReachable != wasReachable) { // Status changed
            std::string status = isReachable ? "reachable" : "unreachable";
            std::string logEntry = currentDateTime() + ipAddress + " is " + status + "\n";

            // Log to file
            logFile << logEntry;
            logFile.flush();

            // Execute specified script if internet is unreachable
            if (!isReachable) {
                std::string command = scriptPath + " &"; // Adding '&' to run the script in the background
                system(command.c_str());
            }

            // Show notification (you might need a different method depending on your environment)
            std::string notifyCommand = "notify-send '" + ipAddress + " Status' '" + status + "'";
            system(notifyCommand.c_str());

            wasReachable = isReachable;
        }

        // Wait for some time before checking again
        sleep(60); // Check every 60 seconds
    }
}

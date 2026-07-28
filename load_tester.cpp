#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include "network/socket_client.h"

constexpr int NUM_CLIENTS = 50;

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "  KungFu Chess C++ Load Tester (" << NUM_CLIENTS << " Clients)" << std::endl;
    std::cout << "==================================================" << std::endl;

    std::atomic<int> successful_connections{0};
    std::atomic<int> failed_connections{0};

    auto start_total = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int i = 1; i <= NUM_CLIENTS; ++i) {
        threads.emplace_back([i, &successful_connections, &failed_connections]() {
            try {
                auto start_time = std::chrono::steady_clock::now();
                network::SocketClient client("127.0.0.1", 8080);
                
                if (client.connect_to_server()) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start_time
                    ).count();
                    
                    successful_connections++;
                    std::cout << "[Client " << i << "] Connected successfully in " << elapsed << " ms" << std::endl;
                    
                    std::string uname = "test_user_" + std::to_string(i);
                    client.send_login(uname, "1234");
                    client.send_find_match();
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    client.disconnect();
                } else {
                    failed_connections++;
                    std::cout << "[Client " << i << "] Failed to connect!" << std::endl;
                }
            } catch (const std::exception& e) {
                failed_connections++;
                std::cerr << "[Client " << i << "] Exception: " << e.what() << std::endl;
            }
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    auto total_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_total
    ).count();

    std::cout << "==================================================" << std::endl;
    std::cout << "  Load Test Results:" << std::endl;
    std::cout << "  Successful Connections: " << successful_connections << " / " << NUM_CLIENTS << std::endl;
    std::cout << "  Failed Connections:     " << failed_connections << std::endl;
    std::cout << "  Total Execution Time:   " << total_elapsed << " ms" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}

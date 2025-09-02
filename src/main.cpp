#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include "server/components/storage_component.hpp"
#include "server/handlers/get.hpp"
#include "server/handlers/set.hpp"
#include "server/handlers/del.hpp"


#include <userver/utils/daemon_run.hpp>

#include <hello.hpp>

int main(int argc, char* argv[]) {

    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<userver::server::handlers::Ping>()
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::components::HttpClient>()
                              .Append<userver::clients::dns::Component>()
                              .Append<userver::server::handlers::TestsControl>()
                              .Append<userver::congestion_control::Component>()
                              .Append<myservice::Hello>()
                              .Append<myservice::components::StorageComponent>()
                              .Append<myservice::handlers::Get>()
                              .Append<myservice::handlers::Set>()
                              .Append<myservice::handlers::Del>()
        ;

    return userver::utils::DaemonMain(argc, argv, component_list);
}

// добавить индексацию в операций AOF
// добавить engine::mutex в Partition и Storage
// разделить Storage и handlers на отдельные task processors
// сделать ручки с json

// #include <iostream>
// #include <thread>
// #include <vector>
// #include <string>
// #include <random>
// #include <memory>
// #include <mutex>
// #include <atomic>
// #include <chrono>
// #include <sstream>

// #include "storage/storage.hpp"

// int main() {
//     const int partitions = 16;
//     const int num_threads = 100;
//     const int ops_per_thread = 1000;
//     const int num_keys = 5000;

//     const std::string aof_path = "logs/appendonly.aof";
//     const std::string snapshot_dir = "snapshots/after_aof_replay";

//     auto storage = std::make_shared<Storage>(partitions, aof_path, snapshot_dir);

//     // storage->GetAofLogger().Clear();

//     std::vector<std::string> keys;
//     keys.reserve(num_keys);
//     for (int i = 0; i < num_keys; ++i) {
//         keys.push_back("key_" + std::to_string(i));
//     }

//     std::mutex cout_mutex;
//     std::atomic<int> total_sets{0}, total_gets{0}, total_dels{0};
//     std::atomic<int> op_exceptions{0};
//     std::atomic<bool> stop_flag{false};

//     auto safe_log = [&](const std::string& s) {
//         std::lock_guard<std::mutex> lg(cout_mutex);
//         std::cout << s << std::endl;
//     };

//     auto worker = [&](int thread_id) {
//     try {
//         std::mt19937 rng(static_cast<unsigned>(
//             std::chrono::high_resolution_clock::now().time_since_epoch().count()
//         ) ^ (thread_id * 1337));

//         std::uniform_int_distribution<int> key_dist(0, num_keys - 1);
//         std::uniform_int_distribution<int> op_dist(0, 2); // 0=Set,1=Get,2=Del

//         std::unordered_set<std::string> local_alive;

//         for (int i = 0; i < ops_per_thread && !stop_flag.load(); ++i) {
//             int op = op_dist(rng);

//             if (op == 0) {
//                 // SET
//                 const std::string key = keys[key_dist(rng)];
//                 bool added = storage->Set(key, "value_" + std::to_string(i));
//                 local_alive.insert(key);
//                 if (added) {
//                     total_sets.fetch_add(1, std::memory_order_relaxed);
//                 }

//             } else if (op == 1) {
//                 // DEL 
//                 if (!local_alive.empty()) {
//                     auto it = local_alive.begin();
//                     std::advance(it, rng() % local_alive.size());
//                     const std::string key = *it;

//                     if (bool removed = storage->Del(key); removed) {
//                         total_dels.fetch_add(1, std::memory_order_relaxed);
//                     }
//                     local_alive.erase(it);
//                 }

//             } else {
//                 // GET 
//                 const std::string key = keys[key_dist(rng)];
//                 auto v = storage->Get(key);
//                 (void)v;
//                 total_gets.fetch_add(1, std::memory_order_relaxed);
//             }

//             if ((i & 0xFF) == 0) {
//                 std::this_thread::sleep_for(std::chrono::microseconds(10));
//             }
//         }
//         } catch (const std::exception &e) {
//             std::ostringstream oss;
//             oss << "[thread " << thread_id << "] fatal exception: " << e.what();
//             safe_log(oss.str());
//             stop_flag.store(true);
//         } catch (...) {
//             safe_log("[thread " + std::to_string(thread_id) + "] fatal unknown exception");
//             stop_flag.store(true);
//         }
//     };

//     std::vector<std::thread> threads;
//     threads.reserve(num_threads);
//     auto start = std::chrono::steady_clock::now();
//     for (int t = 0; t < num_threads; ++t) {
//         threads.emplace_back(worker, t);
//     }

//     for (auto &th : threads) {
//         if (th.joinable()) th.join();
//     }
//     auto end = std::chrono::steady_clock::now();

//     try {
//         storage->Snapshot();
//     } catch (const std::exception &e) {
//         safe_log(std::string("Snapshot failed: ") + e.what());
//     } catch (...) {
//         safe_log("Snapshot failed with unknown exception");
//     }

//     int remaining = 0;
//     for (const auto &k : keys) {
//         try {
//             if (storage->Get(k)) ++remaining;
//         } catch (const std::exception &e) {
//             op_exceptions.fetch_add(1, std::memory_order_relaxed);
//             std::ostringstream oss;
//             oss << "[main] exception during final Get(\"" << k << "\"): " << e.what();
//             safe_log(oss.str());
//         } catch (...) {
//             op_exceptions.fetch_add(1, std::memory_order_relaxed);
//             safe_log("[main] unknown exception during final Get");
//         }
//     }

//     std::chrono::duration<double> dur = end - start;
//     std::cout << "Stress test finished in " << dur.count() << "s\n";
//     std::cout << "Sets: " << total_sets.load() << ", Gets: " << total_gets.load() << ", Dels: " << total_dels.load() << "\n";
//     std::cout << "Exceptions caught: " << op_exceptions.load() << "\n";
//     std::cout << "Remaining keys: " << remaining << " out of " << num_keys << "\n";
//     std::cout << "Total modify operations: " << total_sets.load() + total_dels.load() << "\n";

//     return 0;
// }


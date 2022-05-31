#include "pch.h"
#include "json.h"
#include <chrono>

namespace VgdStd {
    class Timer {
    private:
        decltype(std::chrono::high_resolution_clock::now()) start_time_;
        std::string task_name_;

    public:
        Timer() : start_time_(std::chrono::high_resolution_clock::now()) {}

        template<typename T>
        Timer(T &&task_name) : start_time_(std::chrono::high_resolution_clock::now()),
                               task_name_(std::forward<T>(task_name)) {}


        ~Timer() {
            auto end_time = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
            std::cout << task_name_
                      << "花费了"
                      << double(duration.count()) * std::chrono::microseconds::period::num /
                         std::chrono::microseconds::period::den
                      << "秒" << std::endl;
        }
    };
}// namespace VgdStd

const std::string kTestString = R"({"inbounds":[{"port":10078,"protocol":"vmess","settings":{"clients":[{"id":"\t \n \r \b \f \\ \" \/ / \u0f12 3wertyu\\op"}]}}],"outbounds":[{"protocol":"freedom","settings":{}}]})";

int main() {

    constexpr int kTestNum = 20000;
    std::string s1;
    std::string s2;

    {
        VgdStd::Timer timer("nlohmann 测试" + std::to_string(kTestNum) + "次");

        for (int i = 0; i < kTestNum; ++i) {
            auto haha = nlohmann::json::parse(kTestString);
            s1 = to_string(haha);
        }
    }

    {
        VgdStd::Timer timer("vgd 测试" + std::to_string(kTestNum) + "次");

        for (int i = 0; i < kTestNum; ++i) {
            auto haha = vgd::json::parse_json(kTestString);
            s2 = vgd::json::to_string(haha);
        }
    }

    assert(s2 == kTestString);

    std::cout << "测试通过 " << std::endl;

    return 0;
}

#include "../api/HerkusBus.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Pobierz singleton HerkusBus
    auto& bus = Herkus::HerkusBus::getInstance();

    // Subskrybuj temat "demo_topic"
    bus.Subscribe("demo_topic", [](const std::string& topic, const Herkus::json& msg) {
        std::cout << "Odebrano na [" << topic << "]: " << msg.dump() << std::endl;
    });

    // Publikuj wiadomość na temat "demo_topic"
    Herkus::json payload;
    payload["foo"] = "bar";
    bus.Publish("demo_topic", payload);

    // Poczekaj chwilę, aby callback mógł się wykonać
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return 0;
}
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>

class Monitor {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool event_ready = false;
    std::shared_ptr<std::string> event_data; // Пример несериализуемых данных

public:
    // Функция поставщика
    void provide() {
        for (int i = 1; i <= 5; ++i) { // 5 событий для примера
            // Задержка 1 секунда
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Создаем "несериализуемые" данные
            event_data = std::make_shared<std::string>("Event data " + std::to_string(i));

            {
                std::lock_guard<std::mutex> lock(mtx);
                event_ready = true;
                std::cout << "Поставщик: отправил событие '" << *event_data << "'" << std::endl;
            }

            // Уведомляем потребителя
            cv.notify_one();
        }
    }

    // Функция потребителя
    void consume() {
        for (int i = 1; i <= 5; ++i) {
            std::unique_lock<std::mutex> lock(mtx);

            // Ожидание события с временным освобождением мьютекса
            cv.wait(lock, [this]() { return event_ready; });

            // Обработка события
            std::cout << "Потребитель: получил событие '" << *event_data << "'" << std::endl;
            event_ready = false;

            // Мьютекс автоматически освобождается при выходе из scope
        }
    }
};

int main() {
    Monitor monitor;



    // Запускаем потоки
    std::thread producer_thread(&Monitor::provide, &monitor);
    std::thread consumer_thread(&Monitor::consume, &monitor);

    // Ждем завершения потоков
    producer_thread.join();
    consumer_thread.join();

    return 0;
}
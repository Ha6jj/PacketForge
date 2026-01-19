#include "PacketForge/CommandFactory.hpp"
#include "UserCommands.hpp"

#include <iostream>
#include <iomanip>

int main() {
    // Создаем фабрику команд
    packet_forge::CommandFactory<packet_forge::GameActions_tag> game_factory;
    
    // Регистрируем команды с уникальными заголовками
    game_factory.template registerCommand<PlayerMoveData>(
        packet_forge::CommandType<packet_forge::GameActions_tag>::PlayerMove,
        {'M', 'V'}  // Move header
    );
    
    game_factory.template registerCommand<ChatMessageData>(
        packet_forge::CommandType<packet_forge::GameActions_tag>::ChatMessage,
        {'C', 'M'}  // Chat header
    );
    
    game_factory.template registerCommand<PlayerMoveData>(
        packet_forge::CommandType<packet_forge::GameActions_tag>::ItemPickup,
        {'I', 'P'}  // Item Pickup header
    );

    // Создаем поток данных из нескольких пакетов
    std::vector<uint8_t> data_stream;
    
    // 1. Пакет перемещения игрока
    {
        PlayerMoveData move_data{
            42,               // player_id
            {15, 3},  // position
            3                 // direction (N=0, E=1, S=2, W=3)
        };
        
        auto packet = game_factory.create(
            packet_forge::CommandType<packet_forge::GameActions_tag>::PlayerMove,
            std::move(move_data)
        );
        auto raw_data = packet.build();
        data_stream.insert(data_stream.end(), raw_data.begin(), raw_data.end());
    }
    
    // 2. Пакет сообщения в чат
    {
        ChatMessageData chat_data{
            "Player1337",                    // sender
            "Message with english literals",  // message
            1678945230U                    // timestamp
        };
        
        auto packet = game_factory.create(
            packet_forge::CommandType<packet_forge::GameActions_tag>::ChatMessage,
            std::move(chat_data)
        );
        auto raw_data = packet.build();
        data_stream.insert(data_stream.end(), raw_data.begin(), raw_data.end());
    }
    
    // 3. Пакет подбора предмета
    {
        PlayerMoveData pickup_data{
            42,              // player_id
            {20, 5},         // position
            0                // direction
        };
        
        auto packet = game_factory.create(
            packet_forge::CommandType<packet_forge::GameActions_tag>::ItemPickup,
            std::move(pickup_data)
        );
        auto raw_data = packet.build();
        data_stream.insert(data_stream.end(), raw_data.begin(), raw_data.end());
    }

    // Выводим информацию о потоке
    std::cout << "=== Исходный поток данных ===\n";
    std::cout << "Общий размер: " << data_stream.size() << " байт\n";
    std::cout << "Шестнадцатеричное представление:\n";
    
    for (size_t i = 0; i < data_stream.size(); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex 
                  << (int)data_stream[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << "\n";
    }
    std::cout << std::dec << "\n\n";

    // Десериализуем весь поток
    auto packets = game_factory.deserializeStream(data_stream);
    
    std::cout << "=== Результаты десериализации ===\n";
    std::cout << "Найдено пакетов: " << packets.size() << "\n\n";
    
    // Обрабатываем каждый пакет
    for (size_t i = 0; i < packets.size(); ++i) {
        const auto& packet = packets[i];
        std::cout << "Пакет #" << (i + 1) << ":\n";
        std::cout << "  Размер: " << packet.totalSize << " байт\n";
        
        // Определяем тип команды
        auto cmd_type = packet.command;
        if (cmd_type == packet_forge::CommandType<packet_forge::GameActions_tag>::PlayerMove) {
            std::cout << "  Тип: PlayerMove\n";
            auto& deserializer = static_cast<packet_forge::CommandDeserializer<PlayerMoveData>&>(*packet.deserializer);
            auto args = deserializer.getArgs();
            
            std::cout << "  Данные:\n";
            std::cout << "    Player ID: " << args.player_id << "\n";
            std::cout << "    Position: (" << args.position.x << ", " << args.position.y << ")\n";
            std::cout << "    Direction: " << (int)args.direction << "\n";
        }
        else if (cmd_type == packet_forge::CommandType<packet_forge::GameActions_tag>::ChatMessage) {
            std::cout << "  Тип: ChatMessage\n";
            auto& deserializer = static_cast<packet_forge::CommandDeserializer<ChatMessageData>&>(*packet.deserializer);
            auto args = deserializer.getArgs();
            
            std::cout << "  Данные:\n";
            std::cout << "    Sender: " << args.sender << "\n";
            std::cout << "    Message: " << args.message << "\n";
            std::cout << "    Timestamp: " << args.timestamp << "\n";
        }
        else if (cmd_type == packet_forge::CommandType<packet_forge::GameActions_tag>::ItemPickup) {
            std::cout << "  Тип: ItemPickup\n";
            auto& deserializer = static_cast<packet_forge::CommandDeserializer<PlayerMoveData>&>(*packet.deserializer);
            auto args = deserializer.getArgs();
            
            std::cout << "  Данные:\n";
            std::cout << "    Player ID: " << args.player_id << "\n";
            std::cout << "    Position: (" << args.position.x << ", " << args.position.y << ")\n";
        }
        std::cout << "--------------------------------\n";
    }

    // Демонстрация обработки неполного пакета
    std::vector<uint8_t> incomplete_stream = data_stream;
    incomplete_stream.resize(data_stream.size() - 10); // Обрезаем 10 байт с конца
    
    auto incomplete_packets = game_factory.deserializeStream(incomplete_stream);
    std::cout << "\n=== Обработка неполного потока ===\n";
    std::cout << "Исходный размер: " << data_stream.size() << " байт\n";
    std::cout << "Размер неполного потока: " << incomplete_stream.size() << " байт\n";
    std::cout << "Успешно десериализовано пакетов: " << incomplete_packets.size() << "\n";
    
    if (incomplete_packets.size() < packets.size()) {
        std::cout << "Последний пакет был отброшен, так как данные были неполными\n";
    }

    return 0;
}

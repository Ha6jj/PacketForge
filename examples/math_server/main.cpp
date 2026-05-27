#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <chrono>

#include <PacketForge/CommandFactory.hpp>
#include <PacketForge/macros/PacketStructure.hpp>
#include <PacketForge/macros/CommandSuit.hpp>
#include <PacketForge/serializers/StringSerializers.hpp>
#include <PacketForge/serializers/IntSerializers.hpp>

using namespace packet_forge;

// Определение структур протокола
struct Ping { uint64_t timestamp = 0; };
PACKET_STRUCTURE(Ping, &Ping::timestamp)

struct Pong { uint64_t timestamp = 0; int64_t latency_ms = 0; };
PACKET_STRUCTURE(Pong, &Pong::timestamp, &Pong::latency_ms)

struct EchoRequest { std::string message; };
PACKET_STRUCTURE(EchoRequest, &EchoRequest::message)

struct EchoResponse { std::string message; uint32_t length = 0; };
PACKET_STRUCTURE(EchoResponse, &EchoResponse::message, &EchoResponse::length)

struct MathRequest { double a = 0.0, b = 0.0; char op = '+'; };
PACKET_STRUCTURE(MathRequest, &MathRequest::a, &MathRequest::b, &MathRequest::op)

struct MathResponse { double result = 0.0; std::string error; };
PACKET_STRUCTURE(MathResponse, &MathResponse::result, &MathResponse::error)

// Регистрируем команды
DEFINE_DEFAULT_COMMAND_SUIT(demo, PingCmd, EchoCmd, MathCmd)

// Вспомогательный экстрактор
template <typename T>
T extractPayload(const PacketDescriptor<demo_tag>& desc) {
    return static_cast<PacketDeserializer<T>&>(*desc.deserializer).getArgs();
}

class MathServer {
public:
    explicit MathServer(uint16_t port) : port_(port) {
        // Связываем: структуры данных, тип пакета, заголовок пакета
        factory_.registerCommand<Ping>(CommandType<demo_tag>::PingCmd, {0x01});
        factory_.registerCommand<EchoRequest>(CommandType<demo_tag>::EchoCmd, {0x02});
        factory_.registerCommand<MathRequest>(CommandType<demo_tag>::MathCmd, {0x03});

        std::cout << "[Server] Protocol registered. Listening on port " << port_ << "...\n";
    }

    void start() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) throw std::runtime_error("Socket creation failed");

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) throw std::runtime_error("Bind failed");
        if (listen(server_fd, 5) < 0) throw std::runtime_error("Listen failed");

        std::cout << "[Server] Ready. Test with: nc localhost " << port_ << "\n";

        while (true) {
            sockaddr_in client_addr{};
            socklen_t addrlen = sizeof(client_addr);
            int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
            if (client_fd < 0) continue;

            std::cout << "[Server] Client connected.\n";
            // Фабрика потокобезопасна для чтения после регистрации
            std::thread([this, client_fd]() { handleClient(client_fd); }).detach();
        }
        close(server_fd);
    }

private:
    void handleClient(int client_fd) {
        std::vector<uint8_t> buffer(8192);
        std::vector<uint8_t> leftover;

        while (true) {
            // Получение потока байт
            ssize_t bytes_read = recv(client_fd, buffer.data(), buffer.size(), 0);
            if (bytes_read <= 0) break;
            leftover.insert(leftover.end(), buffer.begin(), buffer.begin() + bytes_read);

            // Парсинг команд
            auto descriptors = factory_.deserializeStream(leftover);

            std::vector<uint8_t> response_buffer;
            int deserialized_total = 0;
            for (const auto& desc : descriptors) {
                // Обработка запроса
                auto resp_bytes = routeCommand(desc);
                response_buffer.insert(response_buffer.end(), resp_bytes.begin(), resp_bytes.end());
                deserialized_total += desc.totalSize;
            }

            // Отправляем ответы
            if (!response_buffer.empty()) {
                send(client_fd, response_buffer.data(), response_buffer.size(), 0);
            }
            leftover.erase(leftover.begin(), leftover.begin() + deserialized_total);
        }

        std::cout << "[Server] Client disconnected.\n";
        close(client_fd);
    }

    std::vector<uint8_t> routeCommand(const PacketDescriptor<demo_tag>& desc) {
        switch (desc.command) {
            case CommandType<demo_tag>::PingCmd: {
                auto req = extractPayload<Ping>(desc);
                Pong resp{req.timestamp, 0};
                return factory_.create(CommandType<demo_tag>::PingCmd, resp).build();
            }
            case CommandType<demo_tag>::EchoCmd: {
                auto req = extractPayload<EchoRequest>(desc);
                EchoResponse resp{req.message, static_cast<uint32_t>(req.message.size())};
                return factory_.create(CommandType<demo_tag>::EchoCmd, resp).build();
            }
            case CommandType<demo_tag>::MathCmd: {
                auto req = extractPayload<MathRequest>(desc);
                MathResponse resp{};
                switch (req.op) {
                    case '+': resp.result = req.a + req.b; break;
                    case '-': resp.result = req.a - req.b; break;
                    case '*': resp.result = req.a * req.b; break;
                    case '/':
                        if (req.b == 0.0) resp.error = "Division by zero";
                        else resp.result = req.a / req.b;
                        break;
                    default: resp.error = "Unknown operator";
                }
                return factory_.create(CommandType<demo_tag>::MathCmd, resp).build();
            }
            default: return {};
        }
    }

    uint16_t port_;
    CommandFactory<demo_tag> factory_;
};

int main() {
    try {
        MathServer server(9090);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

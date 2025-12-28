#include "PacketForge/CommandFactory.hpp"
#include "UserCommands.hpp"

#include <iostream>

int main()
{
    packet_forge::CommandFactory<packet_forge::ComplexSuit_tag> complex_factory;
    complex_factory.registerCommand<ComplexCommandArgs>(packet_forge::CommandType<packet_forge::ComplexSuit_tag>::ComplexCommand, {'r', 's'});

    Position pos = {1, 2};
    SomeNote note = {101, "I am note!"};
    ComplexCommandArgs cmd = {Entity{pos, "entity"}, note};

    auto packet = complex_factory.create(packet_forge::CommandType<packet_forge::ComplexSuit_tag>::ComplexCommand, std::move(cmd));
    std::vector<uint8_t> result = packet.build();

    for (int i = 0; i < result.size(); ++i)
    {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;

    ComplexCommandArgs restored_cmd;
    auto [command_type, deserializer] = complex_factory.deserializePacket(result);

    if (command_type == packet_forge::CommandType<packet_forge::ComplexSuit_tag>::ComplexCommand)
    {
        restored_cmd = static_cast<packet_forge::CommandDeserializer<ComplexCommandArgs>&>(*deserializer.get()).getArgs();
        
        std::cout << restored_cmd.note.note << std::endl;
    }

    return 0;
}
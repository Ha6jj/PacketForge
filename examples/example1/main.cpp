#include "PacketForge/CommandFactory.hpp"
#include "UserCommands.hpp"

#include <iostream>

int main()
{
    packet_forge::CommandFactory factory;
    factory.registerCommand<packet_forge::CommandType, ComplexCommandArgs>(packet_forge::CommandType::COMPLEX_COMMAND, {'r', 's'});

    // REGISTER_COMMAND(factory, CommandType::COMPLEX_COMMAND, ComplexCommandArgs, {0x25, 0x33});

    Position pos = {1, 2};
    SomeNote note = {101, "I am note!"};
    ComplexCommandArgs cmd = {Entity{pos, "entity"}, note};

    packet_forge::Packet packet = factory.create(packet_forge::CommandType::COMPLEX_COMMAND, std::move(cmd));
    std::vector<uint8_t> result = packet.build();

    for (int i = 0; i < result.size(); ++i)
    {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;

    ComplexCommandArgs restored_cmd;
    auto [command_type, deserializer] = factory.deserializePacket(result);

    if (command_type == packet_forge::CommandType::COMPLEX_COMMAND)
    {
        restored_cmd = static_cast<packet_forge::CommandDeserializer<ComplexCommandArgs>&>(*deserializer.get()).getArgs();
        
        std::cout << restored_cmd.note.note << std::endl;
    }

    return 0;
}
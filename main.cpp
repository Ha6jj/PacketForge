#include "Commands.hpp"

#include <iostream>

int main()
{
    CommandFactory factory;
    REGISTER_COMMAND(factory, CommandType::COMPLEX_COMMAND, ComplexCommandArgs);

    Position pos = {1, 2};
    SomeNote note = {101, "I am note!"};
    ComplexCommandArgs cmd = {Entity{pos, "entity"}, note};

    Packet packet = factory.create(CommandType::COMPLEX_COMMAND, std::move(cmd));
    std::vector<uint8_t> result = packet.build();

    for (int i = 0; i < result.size(); ++i)
    {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;

    ComplexCommandArgs restored_cmd;
    auto [command_type, deserializer] = factory.deserializePacket(result);

    if (command_type == CommandType::COMPLEX_COMMAND)
    {
        restored_cmd = static_cast<CommandDeserializer<ComplexCommandArgs>&>(*deserializer.get()).getArgs();
        
        std::cout << restored_cmd.note.note << std::endl;
    }

    return 0;
}
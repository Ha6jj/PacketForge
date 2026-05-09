# PacketForge

**PacketForge** — библиотека низкоуровневой сериализации пакетов на C++.

## Особенности

- **Lock-free пул буферов**: эффективное управление памятью в многопоточных сценариях без блокировок
- **Гибкость**: поддержка пользовательских структур, заголовков и сериализаторов

### Требования

- Компилятор с поддержкой C++17 (GCC ≥ 7.0, Clang ≥ 5.0, MSVC ≥ 14.2)
- CMake ≥ 3.10 (для сборки тестов, бенчмарков и примеров)

# Клонируйте репозиторий
```bash
git clone https://github.com/Ha6jj/PacketForge.git
cd PacketForge
```

## Запуск бенчмарков (Linux)

* Требуется google-benchmark, protobuf (Опционально. Нужно для получения референсных значений).

```bash
chmod +x run_benchmarks.sh
./run_benchmarks.sh
```

Результаты тестирования будут лежать в benchmark_results/

### Если хотите самостоятельно собрать и запустить

* Выбрать бенчмарки для сборки можно в benchmarks/CMakeLists.txt

```bash
cmake -B build -DBUILD_BENCHMARKS=ON
cmake --build build
./build/benchmarks/packetforge/packetforge_benchmark
```

## Сборка и запуск примеров

* Выбрать примеры для сборки можно в examples/CMakeLists.txt

```bash
cmake -B build -DBUILD_EXAMPLES=ON
cmake --build build
./build/examples/example1/example1
```

## Сборка и запуск тестов

* Выбрать тесты для сборки можно в tests/CMakeLists.txt

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
./build/tests/packet_forge_tests
```

### Генерация отчета о покрытии тестами

```bash
chmod +x generate_coverage.sh
./generate_coverage.sh
```

# Быстрый старт

1. Построение схем
```cpp
// Определите структуру
struct Position {
    uint32_t x;
    uint32_t y;
};

// Макрос создаст обработчики для структуры
PACKET_STRUCTURE(Position, &Position::x, &Position::y)
```

2. Создайте группу пакетов
```cpp
DEFINE_DEFAULT_COMMAND_SUIT(SuitName, /*Config (опционально),*/
    Packet1,
    // ... другие пакеты
)
```
3. Зарегистрируйте пакеты в группе
```cpp
// Создайте фабрику для группы пакетов
packet_forge::CommandFactory<packet_forge::SuitName_tag> factory;

// Зарегистрируйте пакет
factory.registerCommand<Position>(
    packet_forge::CommandType<packet_forge::SuitName_tag>::Packet1,
    // Заголовок пакета
    {0x01, 0x02}
);
```

4. Сериализация
```cpp
Position pos = {10, 20};
std::vector<uint8_t> serialized = factory.create(
    packet_forge::CommandType<packet_forge::SuitName_tag>::Packet1, 
    pos
).build();
```

5. Десериализация
```cpp
// Дескриптор хранит тип пакета, его содержимое и размер
packet_forge::PacketDescriptor<packet_forge::SuitName_tag> descriptor =       
    factory.deserializePacket(result).value();

if (descriptor.command == packet_forge::CommandType<packet_forge::SuitName_tag>::Packet1) {
    // Достаем десериализованные данные из дескриптроа
    Position deserialized = static_cast<packet_forge::PacketDeserializer<Position>&>(
        *descriptor.deserializer.get()
    ).getArgs();
    // deserialized.x == 10 && deserialized.y == 20
}
```

-------------------------------

## Идеи для доработки

- Привязать сериалайзеры к тегам
- Упростить определение заголовка пакета на стадии компиляции

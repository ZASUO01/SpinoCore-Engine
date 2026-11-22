# Logs

`Logger` writes messages with `std::format` syntax. The format string is checked at compile time and the file
and line of the call are recorded. Logging never throws.

```cpp
#include "SpinoCore/Logs/Logger.h"

using SpinoCore::Logs::Logger;

Logger::Info("Loaded {} actors", count);
Logger::Warn("Texture {} is missing", name);
Logger::Error("Could not save slot {}", slot);
Logger::Fatal("Out of memory");
Logger::Force("Always written");
```

## Levels

`Info < Warn < Error < Fatal`. Messages below the current level are skipped, except `Force`. `Level::None`
turns off everything except `Force`.

```cpp
Logger::SetLevel(SpinoCore::Logs::Level::Warn);
```

The initial level comes from `LogOptions`, the third argument of `Launch`:

```cpp
SpinoCore::Logs::LogOptions logs{};
logs.level = SpinoCore::Logs::Level::Warn;
return SpinoCore::Launch(std::make_unique<MyGame>(), options, std::move(logs));
```

## Sinks

By default, colored lines with the time, the level, the file and the line are written to the console. A custom
`LogSink` sends the messages elsewhere:

```cpp
#include <fstream>
#include "SpinoCore/Logs/LogSink.h"

class FileSink final : public SpinoCore::Logs::LogSink {
public:
    void Write(const SpinoCore::Logs::LogRecord& record) override {
        mFile << record.location.file_name() << ':' << record.location.line() << ' ' << record.message << '\n';
    }

private:
    std::ofstream mFile{"game.log"};
};

logs.sink = std::make_unique<FileSink>();
```

Calls to `Write` are serialized by the engine. `record.message` is only valid during the call.

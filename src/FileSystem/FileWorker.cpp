#include "FileWorker.h"

namespace SpinoCore::FileSystem {
    FileWorker::FileWorker(ConstructorKey) {}
    FileWorker::~FileWorker() = default;

    std::unique_ptr<FileWorker> FileWorker::Create() {
        return std::make_unique<FileWorker>(ConstructorKey{});
    }
}

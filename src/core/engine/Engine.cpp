#include "Engine.hpp"

#include "core/offsets/Offsets.hpp"

bool Engine::Init() {
    return GetInstance().InitImpl();
}

bool Engine::Run() {
    return GetInstance().RunImpl();
}

ProcessModule Engine::GetClient() {
    return GetInstance().client;
}

std::shared_ptr<pProcess> Engine::GetProcess() {
    return GetInstance().process;
}

bool Engine::InitImpl() {
    process = std::make_shared<pProcess>();

    if (!this->AwaitProcess()) {
        LOGF(FATAL, "Could not find process, please make sure the game is open");
        return false;
    }

    if (!this->AwaitModules()) {
        LOGF(FATAL, "Game took too long to load, please open me again once its fully loaded");
        return false;
    }

    //std::thread(&Engine::Thread, this).detach();

    LOGF(VERBOSE, "Successfully initialized engine...");
    return true;
}

bool Engine::RunImpl() {
    uintptr_t address = client.base + offsets::base + 0xB;

    auto before = process->read<uint8_t>(address);

    if (before != 0 && before != 1) {
        LOGF(WARNING, "Address returened an invalid value, the game probably updated");
        return false;
    }

#ifdef _DEBUG
    LOGF(VERBOSE, "Before patching bytes is {}", before);
#endif

    bool should_flip = before == 0;
    uint8_t byte = should_flip ? 0x01 : 0x0;

    DWORD old;
    VirtualProtectEx(process->handle_, (LPVOID)address, 1, PAGE_EXECUTE_READWRITE, &old);
    process->write_bytes(address, std::vector<uint8_t>{byte});
    VirtualProtectEx(process->handle_, (LPVOID)address, 1, old, &old);

    auto after = process->read<uint8_t>(address);

#ifdef _DEBUG
    LOGF(VERBOSE, "After patching bytes is {}", after);
#endif

    if (after == byte)
        LOGF(INFO, "Patch sucessfull, hack has been {}! run me again to reverse the effect!", (should_flip ? "enabled" : "disabled"));
    else
        LOGF(FATAL, "Failed to patch memory, expected value (0x{:X}) actual value (0x{:X})", byte, after);

    return after == byte;
}

void Engine::Thread() {
    while (true) {
        std::this_thread::sleep_for(1ms);
    }
}

bool Engine::AwaitProcess() {
    if (!process || process->handle_) // Process not initialized, or already attached
        return false;

    do {
        if (process->AttachProcess("cs2.exe", PROCESS_ALL_ACCESS))
            break;

        if (process->pid_ && !process->handle_) {
            LOGF(FATAL, "Insufficient permissions to open a handle to the process. Try running as Administrator.");
            return false;
        }

        static int attempts = 0;

        if (!attempts)
            LOGF(VERBOSE, "Waiting 50s for the game to open...");

        if (attempts > 10)
            return false;
        attempts++;

        std::this_thread::sleep_for(5s);
    } while (true);

    return true;
}

bool Engine::AwaitModules() {
    if (!process || !process->handle_) // Process not initialized, or not attached
        return false;

    LOGF(VERBOSE, "Waiting for the game to open...");

    do {
        this->client = process->GetModule("client.dll");

        if (this->client.base)
            break;

        static int attempts = 0;
        if (attempts > 10)
            return false;
        attempts++;

        std::this_thread::sleep_for(5s);
    } while (true);

    return true;
}

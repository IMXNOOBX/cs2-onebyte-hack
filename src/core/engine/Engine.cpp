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

    // In case its manually provided, we skip sig search
    if (!offsets::cvar_unknown) {
        if (auto address = process->FindSignature(client, offsets::signatures::cvar_unknown, 0x5)) {
            offsets::cvar_unknown = address - client.base + 2; // Shitty math, we do want the offset and not the full address: address (full) - client.base (to get the offset) + 2 (to get to the value we want to modify)
            LOGF(VERBOSE, "Found offset by signature at base 0x{:X}, cvar offset 0x{:X}", address - client.base, offsets::cvar_unknown - client.base);
        }
    }

    if (!offsets::cvar_unknown) {
        LOGF(FATAL, "Failed to find offset with provided signature, please update Offsets.hpp with the latest dump");
        return false;
    }

    //std::thread(&Engine::Thread, this).detach();

    LOGF(VERBOSE, "Successfully initialized engine...");
    return true;
}

bool Engine::RunImpl() {
    uintptr_t patch_addr = client.base + offsets::cvar_unknown;

    auto before = process->read<uint8_t>(patch_addr);

    if (before != 0 && before != 1) {
        LOGF(WARNING, "Address returened an invalid value, the game probably updated");
        return false;
    }

#ifdef _DEBUG
    LOGF(VERBOSE, "Before patching bytes is {}", before);
#endif

    uint8_t patch = before == 0 ? 0x01 : 0x0;

    process->write_bytes(client.base + offsets::cvar_unknown, std::vector<uint8_t>{patch});

    auto after = process->read<uint8_t>(patch_addr);

#ifdef _DEBUG
    LOGF(VERBOSE, "After patching bytes is {}", after);
#endif

    if (after != patch) {
        LOGF(FATAL, "Failed to patch memory, expected value (0x{:X}) actual value (0x{:X})", patch, after);
        return false;
    }
        
    LOGF(INFO, "Patch sucessfull, hack has been '{}'! run me again to reverse the effect!", (after == 1 ? "enabled" : "disabled"));

    if (after == 1)
        LOGF(INFO, "If it doesn't seem to work, open the in-game console and type 'spec_show_xray 1'");

    return true;
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

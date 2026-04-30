#include <iostream>

#include "core/engine/Engine.hpp"

int main()
{
    LogHelper::Init();

    LOGF(INFO, "Compiled {}, Welcome to cs2-onebyte-hack!", __TIMESTAMP__);

    if (!Engine::Init()) {
        LOGF(FATAL, "Engine failed to initialize, cannot continue execution");
        goto exit;
    }

    if (!Engine::Run()) {
        LOGF(FATAL, "Engine failed to patch game memory, im sorry, game updated");
        goto exit;
    }

exit:
    LOGF(VERBOSE, "Thats it, im done, closing in 5s!");
    LogHelper::Destroy();
    std::this_thread::sleep_for(5s);
}
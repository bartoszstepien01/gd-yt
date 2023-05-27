#include "pch.h"
#include "MappedHook.h"

MH_STATUS registerHook(std::uintptr_t address, void* hook) {
    void* trampoline;
    auto status = MH_CreateHook(reinterpret_cast<void**>(address), hook, &trampoline);
    if (status == MH_OK)
        hooks[hook] = trampoline;
    return status;
}

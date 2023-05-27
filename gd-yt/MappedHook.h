#pragma once

inline std::unordered_map<void*, void*> hooks;

MH_STATUS registerHook(std::uintptr_t address, void* hook);

template <typename F>
inline F getOriginal(F self) {
    return reinterpret_cast<F>(hooks[self]);
}

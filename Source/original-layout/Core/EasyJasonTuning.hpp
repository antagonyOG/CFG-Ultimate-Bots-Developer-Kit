#pragma once
#include <cstdint>
// Easy Jason tuning (1.0.3). Easy difficulty (index 0) already halves damage and doubles the
// Rage unlock time; this adds: no AI foot sprint bursts and a 30 s knife cooldown (stock 12 s).
// Normal (1) and Hard (2) are untouched. 0 = both gates compiled out.
#ifndef CFG_EASY_JASON_TUNING
#define CFG_EASY_JASON_TUNING 1
#endif
namespace cfgeasy {
constexpr std::uint32_t EasyDifficulty=0;
constexpr std::uint64_t StockKnifeCooldownMs=12000,EasyKnifeCooldownMs=30000;
inline bool Easy(std::uint32_t difficulty)noexcept{return CFG_EASY_JASON_TUNING&&difficulty==EasyDifficulty;}
inline bool BurstAllowed(std::uint32_t difficulty)noexcept{return !Easy(difficulty);}
inline std::uint64_t KnifeCooldownMs(std::uint32_t difficulty)noexcept{return Easy(difficulty)?EasyKnifeCooldownMs:StockKnifeCooldownMs;}
}

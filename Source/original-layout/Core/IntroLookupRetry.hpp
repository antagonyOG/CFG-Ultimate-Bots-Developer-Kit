#pragma once
// H2E (2026-11-13, live PID8404 Graveyard): the offline intro metadata borrow refused (failure 7) because its first
// freshness predicate (scoped lookup context) failed once right after the native KillerClass assignment; the startup
// publication window had hit lock contention (LockRetries=6, WindowRefusals=1). A refusal there leaves the match in
// PreMatchIntro forever (no ParentIntro). The lookup context is re-checked, only waiting in between (no writes, no
// native calls), until MaxMs has elapsed; then the original refusal applies unchanged.
#include <cstdint>
#ifndef CFG_H2E_INTRO_LOOKUP_RETRY
#define CFG_H2E_INTRO_LOOKUP_RETRY 1
#endif
namespace cfgintroretry {
constexpr unsigned WaitMs=2;constexpr std::uint64_t MaxMs=1500;
template<class Check,class Wait,class Now>bool Retry(Check&& check,Wait&& wait,Now&& now,unsigned& retries){
    if(check())return true;
    const std::uint64_t start=now();
    while(now()-start<MaxMs){wait();++retries;if(check())return true;}
    return false;
}
}

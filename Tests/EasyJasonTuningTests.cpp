#include "EasyJasonTuning.hpp"
#include <cassert>
#include <cstdio>
// Easy (0) loses foot bursts and gets a 30 s knife cooldown; Normal (1) and Hard (2) keep stock behaviour.
int main(){
    static_assert(CFG_EASY_JASON_TUNING==1,"fixture assumes the flag is on by default");
    assert(!cfgeasy::BurstAllowed(0));
    assert(cfgeasy::BurstAllowed(1));
    assert(cfgeasy::BurstAllowed(2));
    assert(cfgeasy::KnifeCooldownMs(0)==30000);
    assert(cfgeasy::KnifeCooldownMs(1)==12000);
    assert(cfgeasy::KnifeCooldownMs(2)==12000);
    assert(cfgeasy::KnifeCooldownMs(7)==12000); // out-of-range value never gets Easy tuning
    assert(cfgeasy::BurstAllowed(7));
    assert(cfgeasy::Easy(0)&&!cfgeasy::Easy(1)&&!cfgeasy::Easy(2));
    std::puts("EasyJasonTuning PASS: Easy no bursts + 30 s knife cooldown; Normal/Hard stock");
    return 0;
}

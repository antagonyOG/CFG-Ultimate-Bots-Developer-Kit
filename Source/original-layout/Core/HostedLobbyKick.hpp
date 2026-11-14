#pragma once
// H2F (user request 2026-11-13): AI Jason must work in every hosted lobby. If AI Jason is on and the lobby already holds
// 8 humans (they joined before AI Jason was turned on), the host's own lobby kick request removes the MOST RECENT joiner
// (last PlayerArray entry that is not the host) once, in the lobby, before any match. Stock path: the host's
// AILLPlayerController::ServerRequestKickPlayer(KickPlayerState) UFunction (same request as the lobby kick button), run
// through UObject::ProcessEvent on the listen host, which runs the game's own _Validate then _Implementation.
// Shipping 4FC4BB54, disk-verified (Research: factory/cache/registration/executor below).
#include <array>
#include <cstdint>
#ifndef CFG_H2F_LOBBY_KICK
#define CFG_H2F_LOBBY_KICK 1
#endif
namespace cfghostedkick {
constexpr std::uintptr_t ControllerReflection=0x0/*CFG3.5 address withheld*/,ControllerConstructor=0x0/*CFG3.5 address withheld*/; // AILLPlayerController
constexpr std::uintptr_t FunctionCache=0x0/*CFG3.5 address withheld*/,FunctionFactory=0x0/*CFG3.5 address withheld*/,Executor=0x0/*CFG3.5 address withheld*/;  // ServerRequestKickPlayer
constexpr std::uintptr_t FunctionOuter=0x20,FunctionExec=0xB0,PlayerArray=0x380;
constexpr unsigned ParameterBytes=8;
constexpr std::array<unsigned char,47> FactoryCode{} /*CFG3.5 code signature withheld*/;
// execServerRequestKickPlayer: read KickPlayerState, vtable+0xD30 (_Validate) then +0xD38 (_Implementation).
constexpr std::array<unsigned char,181> ExecutorCode{} /*CFG3.5 code signature withheld*/;
constexpr unsigned FullHumans=8,TestFullHumans=2,StableSamples=2,MaxAttemptsPerTarget=2;
constexpr std::uint64_t SampleMs=1000,RetryMs=4000,BadgeMs=10000;
// Test switch (dev builds only, file beside the DLL): "full" means 2 humans so a second PC can be the one removed.
constexpr bool Full(unsigned humans,bool test)noexcept{return humans>=(test?TestFullHumans:FullHumans)&&humans<=32;}
enum Refusal : unsigned {None=0,NoTarget=1,TargetType=2,TargetHost=3,TargetExhausted=4,ControllerType=5,Function=6,Veto=7,Changed=8,Dispatch=9};
struct State {
    std::uintptr_t world=0,lastTarget=0;unsigned stable=0,attemptsOnTarget=0;std::uint64_t nextAt=0,lastKickAt=0;
    unsigned attempts=0,dispatched=0,refusals=0,lastRefusal=0,lastHumans=0;bool test=false;
};
// The kicked-joiner badge shows for BadgeMs after a dispatched kick.
constexpr bool BadgeActive(const State& s,std::uint64_t now)noexcept{return s.dispatched&&s.lastKickAt&&now-s.lastKickAt<BadgeMs;}
}

#pragma once
// H2D: while the hosted AI Jason is on, the lobby is capped at 7 players, so the AI always has slot 8.
// The stock lobby beacon host object stores MaxPlayers (NumPublic+NumPrivate = 8) in AILLSessionBeaconHostObject::InitHostBeacon
// (mov [this+3B0h],edi; the session FName is stored at this+3A8h just before). HandleReservationRequest answers
// GameFull(5)/PartyFull(6) once members+new >= this+0x3B0. Lowering 8 to 7 refuses only NEW joins; it never kicks anyone.
// Shipping 4FC4BB54, verified by disassembly.
// H2D3 (2026-11-13): DATA-ONLY. The H2D2 code detour on InitHostBeacon is gone (two live freezes with it, none without;
// and it ran before the host turned AI Jason on, so it never capped). The owner thread now finds the lobby host object by
// its vtable in the object array and compare-writes MaxPlayers 8 -> 7 while AI Jason is on, 7 -> 8 (only its own 7) when off.
// No code is patched; the three code pins below are read-only proof that the layout matches this game build.
#include <array>
#include <cstdint>
#ifndef CFG_H2D_LOBBY_CAP_7
#define CFG_H2D_LOBBY_CAP_7 1
#endif
namespace cfghostedcap {
constexpr std::uintptr_t InitHostBeacon=0x0/*CFG3.5 address withheld*/,MaxPlayersStore=0x0/*CFG3.5 address withheld*/,ReserveCompare=0x0/*CFG3.5 address withheld*/,LobbyHostVtable=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t NameOffset=0x3A8,MaxPlayersOffset=0x3B0;
constexpr std::uint32_t GameSessionName=0x11F;       // FName comparison index of GameSession (low 32 bits at this+3A8h)
constexpr std::int32_t StockMaxPlayers=8,CappedMaxPlayers=7;
constexpr unsigned ScanItemsPerPass=2048,RescanDelayMs=2000;
// Prologue of InitHostBeacon (read-only pin; nothing is patched).
constexpr std::array<unsigned char,24> EntryCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,8> StoreCode{} /*CFG3.5 code signature withheld*/;      // mov [r12+3B0h],edi (hostObj->MaxPlayers)
constexpr std::array<unsigned char,6> CompareCode{} /*CFG3.5 code signature withheld*/;              // cmp eax,[rsi+3B0h]  (reservation capacity test)
enum class Write : unsigned { None, Cap, Restore };
// want: AI Jason is on for this lobby. wrote: the current 7 is ours. Only the stock 8 is ever lowered and only our own 7 is raised back.
constexpr Write PlanWrite(bool want,bool wrote,std::int32_t value)noexcept{
    return want?(value==StockMaxPlayers?Write::Cap:Write::None):((wrote&&value==CappedMaxPlayers)?Write::Restore:Write::None);
}
// AI Jason wanted for this lobby: armed / operator arm / lobby option on, and not disarmed or turned off by the full-lobby guard.
constexpr bool Wanted(bool armed,bool manualArm,bool optionOn,bool disarmed,bool fullLobbyOff)noexcept{
    return (armed||manualArm||optionOn)&&!disarmed&&!fullLobbyOff;
}
struct Counters {
    unsigned installed=0,pinMask=0,scanPasses=0,found=0,capped=0,restored=0,lost=0;
    std::uint64_t object=0;bool active=false; // active: the lobby currently holds our 7
};
}

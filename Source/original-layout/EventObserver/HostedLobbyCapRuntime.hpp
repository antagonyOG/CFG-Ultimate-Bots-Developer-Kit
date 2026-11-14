// H2D3: cap the hosted lobby at 7 while the AI Jason is on (see Core/HostedLobbyCap.hpp). DATA-ONLY: no code detour.
// Owner (game) thread only: the lobby beacon host object is found by vtable in the object array, re-proved every pass
// (registered slot, vtable, GameSession name), and its MaxPlayers is compare-exchanged 8 <-> 7. The engine reads that
// value on the same thread when a reservation arrives, so there is no race and nothing is suspended or patched.
#if CFG_H2D_LOBBY_CAP_7
cfghostedcap::Counters hostedLobbyCap;
struct HostedLobbyCapTrack {std::uintptr_t object=0;std::int32_t cursor=0;bool wrote=false,pinsChecked=false;ULONGLONG nextScanAt=0;};
HostedLobbyCapTrack hostedLobbyCapTrack;
bool HostedLobbyCapPins(std::uintptr_t module){
    auto& c=hostedLobbyCap;auto& t=hostedLobbyCapTrack;
    if(t.pinsChecked)return c.installed!=0;t.pinsChecked=true;
    Reader r;r.cap=32;using namespace cfghostedcap;
    c.pinMask=(cfgbundle::Code(r,module,InitHostBeacon,EntryCode)?1u:0u)|(cfgbundle::Code(r,module,MaxPlayersStore,StoreCode)?2u:0u)|
        (cfgbundle::Code(r,module,ReserveCompare,CompareCode)?4u:0u);
    c.installed=c.pinMask==7u?1u:0u;return c.installed!=0;
}
void ServiceHostedLobbyCap(std::uintptr_t module,bool want){
    if(!module||GetCurrentThreadId()!=observerOwnerThread||!HostedLobbyCapPins(module))return;
    auto& c=hostedLobbyCap;auto& t=hostedLobbyCapTrack;using namespace cfghostedcap;namespace lt=cfgprobe::lifetime;
    Reader r;r.cap=ScanItemsPerPass+64;
    const auto forget=[&]{t.object=0;t.wrote=false;c.active=false;c.object=0;};
    lt::ArrayHeader objects;
    if(!lt::ReadArrayHeader(r,module+lt::ObjectArrayRva,objects)){if(t.object){forget();++c.lost;}return;}
    const std::uint64_t table=module+LobbyHostVtable;
    const auto proven=[&](std::uintptr_t o){lt::SlotObservation slot;std::uint64_t vt=0,name=0;
        return lt::ReadSlot(r,objects,o,slot)&&r.Read(o,vt)&&vt==table&&r.Read(o+NameOffset,name)&&static_cast<std::uint32_t>(name)==GameSessionName;};
    if(t.object&&!proven(t.object)){forget();++c.lost;}
    if(!t.object){
        if(!want)return;                                   // search only while a cap is wanted
        const auto now=GetTickCount64();if(now<t.nextScanAt)return;
        if(t.cursor<0||t.cursor>=objects.count)t.cursor=0;
        const auto remaining=static_cast<unsigned>(objects.count-t.cursor);
        const std::int32_t end=t.cursor+static_cast<std::int32_t>(remaining<ScanItemsPerPass?remaining:ScanItemsPerPass);
        constexpr std::int32_t PerBlock=static_cast<std::int32_t>(16384/lt::ItemBytes);
        std::array<unsigned char,16384> block{};std::uintptr_t hit=0;
        for(std::int32_t i=t.cursor;i<end&&!hit;){
            const std::int32_t n=end-i<PerBlock?end-i:PerBlock;
            if(!r.ReadBytes(objects.data+static_cast<std::uintptr_t>(i)*lt::ItemBytes,block.data(),static_cast<std::size_t>(n)*lt::ItemBytes)){t.cursor=0;return;}
            for(std::int32_t k=0;k<n&&!hit;++k){std::uintptr_t o=0;std::memcpy(&o,block.data()+static_cast<std::size_t>(k)*lt::ItemBytes+lt::ItemObjectOffset,sizeof(o));
                std::uint64_t vt=0;if(o>=0x10000&&o<=lt::LastUserByte&&r.Read(o,vt)&&vt==table&&proven(o))hit=o;}
            i+=n;
        }
        t.cursor=end;++c.scanPasses;
        if(!hit){if(t.cursor>=objects.count){t.cursor=0;t.nextScanAt=now+RescanDelayMs;}return;}
        t.object=hit;t.wrote=false;c.object=hit;++c.found;
    }
    std::int32_t value=0;if(!r.Read(t.object+MaxPlayersOffset,value))return;
    auto* at=reinterpret_cast<volatile long*>(t.object+MaxPlayersOffset);
    switch(PlanWrite(want,t.wrote,value)){
    case Write::Cap:if(_InterlockedCompareExchange(at,CappedMaxPlayers,StockMaxPlayers)==StockMaxPlayers){t.wrote=true;++c.capped;}break;
    case Write::Restore:if(_InterlockedCompareExchange(at,StockMaxPlayers,CappedMaxPlayers)==CappedMaxPlayers){t.wrote=false;++c.restored;}break;
    case Write::None:if(want&&value==CappedMaxPlayers)t.wrote=true;break; // already our 7 from an earlier pass
    }
    c.active=t.wrote;
}
#endif

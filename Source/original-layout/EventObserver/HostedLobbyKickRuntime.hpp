// H2F: remove the most recent joiner once when AI Jason is on and the hosted lobby holds 8 humans (Core/HostedLobbyKick.hpp).
// Runs from ServiceHostedH1 only: owner thread, top level (not nested), after the forwarded original returned. Every pass
// re-proves the typed listen-host lobby (cfghostedlobby::Observe twice), the host-only target, the UFunction (populated
// cache, Outer = AILLPlayerController, Func = pinned executor, executor/factory bytes, 8-byte frame) and the GC/serialize
// veto, then dispatches ONE UObject::ProcessEvent on the host's own controller. A failed dispatch is never replayed blindly.
#if CFG_H2F_LOBBY_KICK
cfghostedkick::State hostedLobbyKick;
inline bool HostedKickTestFilePresent()noexcept{
#if defined(CFG_H2A_REMOTE_TOOLS) && CFG_H2A_REMOTE_TOOLS
    wchar_t path[MAX_PATH]{};HMODULE self=nullptr;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&hostedLobbyKick),&self)||!self)return false;
    const auto length=GetModuleFileNameW(self,path,MAX_PATH);
    if(!length||length>=MAX_PATH-32)return false;
    auto slash=std::wcsrchr(path,L'\\');if(!slash)return false;
    if(wcscpy_s(slash+1,static_cast<rsize_t>(MAX_PATH-(slash+1-path)),L"CFG-KICK-TEST.txt"))return false;
    return GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES;
#else
    return false; // public builds: no test switch
#endif
}
void ServiceHostedLobbyKick(std::uintptr_t module,bool want){
    using namespace cfghostedkick;auto& k=hostedLobbyKick;const auto now=GetTickCount64();
    if(!want){k.stable=0;return;}
    if(now<k.nextAt)return;k.nextAt=now+SampleMs;
    if(!module||GetCurrentThreadId()!=observerOwnerThread)return;
    const auto refuse=[&](unsigned why){k.stable=0;++k.refusals;k.lastRefusal=why;};
    Reader r;r.cap=4096;cfghostedlobby::Context c;cfghostedlobby::Failure failure{};
    if(!cfghostedlobby::Observe(r,module,observerOwnerThread,c,failure)||!c.listenHost||!c.hostFlag){k.stable=0;return;}
    if(c.world!=k.world){k.world=c.world;k.lastTarget=0;k.attemptsOnTarget=0;k.stable=0;}
    cfgspawn::PlayerArray list{};
    if(!r.Read(c.gameState+PlayerArray,list)||list.count<0||list.count>32||list.capacity<list.count||list.capacity>64||
       (list.count&&!cfgprobe::ObjectPointer(list.data))){k.stable=0;return;}
    k.test=HostedKickTestFilePresent();k.lastHumans=static_cast<unsigned>(list.count);
    if(!Full(static_cast<unsigned>(list.count),k.test)){k.stable=0;return;}
    if(++k.stable<StableSamples)return;                       // full on two consecutive samples (no join/leave race)
    std::uintptr_t host=0,target=0;cfgprobe::lifetime::SlotObservation slot{};
    if(!r.Read(c.controller+cfgescape::disk::PlayerState,host)||!cfgprobe::ObjectPointer(host))return refuse(NoTarget);
    for(std::int32_t i=list.count-1;i>=0&&!target;--i){std::uintptr_t state=0;
        if(!r.Read(list.data+8ULL*static_cast<unsigned>(i),state))return refuse(NoTarget);
        if(cfgprobe::ObjectPointer(state)&&state!=host)target=state;}
    if(!target)return refuse(NoTarget);
    if(!cfghostedlobby::Typed(r,c,target,cfghostedlobby::disk::PlayerStateReflection,slot))return refuse(TargetType);
    std::uint8_t hostByte=1;if(!r.Read(target+cfghostedlobby::disk::HostByte,hostByte)||(hostByte&1))return refuse(TargetHost);
    if(target==k.lastTarget){
        if(k.attemptsOnTarget>=MaxAttemptsPerTarget)return refuse(TargetExhausted);
        if(now<k.lastKickAt+RetryMs)return;                   // the previous request is still being processed
    }else{k.lastTarget=target;k.attemptsOnTarget=0;}
    std::uintptr_t reflection=0,constructor=0,owner=0,function=0;std::uint16_t bytes=0;cfgprobe::lifetime::SlotObservation functionSlot{};
    if(!r.Read(module+ControllerReflection,reflection)||!r.Read(module+ControllerConstructor,constructor)||
       !(owner=cfgprobe::MatchingClassCache(reflection,constructor))||!cfgspawn::ObjectIs(r,c.controller,owner))return refuse(ControllerType);
    if(!cfgbundle::Code(r,module,FunctionFactory,FactoryCode)||!cfgbundle::Code(r,module,Executor,ExecutorCode)||
       !r.Read(module+FunctionCache,function)||!cfgprobe::ObjectPointer(function)||                 // populated cache only; no factory call
       !cfgspawn::ObjectSlot(r,c.registry,function,functionSlot)||!cfgobserver::Same(r,function+FunctionOuter,owner)||
       !cfgobserver::Same(r,function+FunctionExec,module+Executor)||
       !r.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,bytes)||bytes!=ParameterBytes)return refuse(Function);
    if(!cfgprimary::LookupVetoClearForObservation(r,module))return refuse(Veto);
    cfghostedlobby::Context again;cfgspawn::PlayerArray listAgain{};
    if(!cfghostedlobby::Observe(r,module,observerOwnerThread,again,failure)||!cfghostedlobby::Same(c,again)||
       !r.Read(c.gameState+PlayerArray,listAgain)||!(listAgain.data==list.data&&listAgain.count==list.count)||
       !cfghostedlobby::Typed(r,c,target,cfghostedlobby::disk::PlayerStateReflection,slot))return refuse(Changed);
    struct Packet {std::uintptr_t kickPlayerState=0;} packet{target};
    ++k.attempts;++k.attemptsOnTarget;k.lastKickAt=now;k.stable=0;
    if(!HostedLeafProcessEvent(module+cfgspawn::GenericDispatcherRva,c.controller,function,&packet)){++k.refusals;k.lastRefusal=cfghostedkick::Refusal::Dispatch;return;}
    ++k.dispatched;k.lastRefusal=None;
}
#endif

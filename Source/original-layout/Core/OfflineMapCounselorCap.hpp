#pragma once
// H2E (2026-11-13, live): offline 10 counselors on Graveyard started, then the native end check ended the match after 11 s
// ("You survived", 8 native participants). 9 counselors on Graveyard works; 10 works on Crystal Lake Small (+ snow) and
// Packanack Small. So 10 is allowed only on those verified maps; every other map (including any unreadable map name)
// uses at most 9 counselors = 8 helper bots + the human. The count is lowered before any bot birth; nothing else changes.
#include <string_view>
#ifndef CFG_H2E_SMALL_MAP_CAP
#define CFG_H2E_SMALL_MAP_CAP 1
#endif
namespace cfgmapcap {
constexpr unsigned SmallMapMaxHelpers=8;
// World object names (the map package's world), never menu labels.
constexpr bool AllowsTen(std::u16string_view world)noexcept{
    return world==u"CrystalLake_Small"||world==u"CrystalLake_Small_Snow"||world==u"Packanack_sm";
}
constexpr unsigned Helpers(unsigned requested,bool nameKnown,bool allowsTen)noexcept{
    return (nameKnown&&allowsTen)?requested:(requested>SmallMapMaxHelpers?SmallMapMaxHelpers:requested);
}
// H2E3 (live: capped helpers alone still ended Graveyard at 12 s, before any deferred helper): map script reads
// NumCounselors from the game mode OptionsString. Return the index of the "10" digits in an exact "NumCounselors=10"
// option (bounded by '?' or end), so it can be rewritten in place to "09" (same length; parses as 9). npos otherwise.
constexpr std::size_t NumCounselorsTenDigits(std::u16string_view s)noexcept{
    constexpr std::u16string_view key=u"NumCounselors=10";
    for(auto p=s.find(key);p!=std::u16string_view::npos;p=s.find(key,p+1)){
        const auto end=p+key.size();
        if((p==0||s[p-1]==u'?')&&(end==s.size()||s[end]==u'?'))return p+key.size()-2;
    }
    return std::u16string_view::npos;
}
}

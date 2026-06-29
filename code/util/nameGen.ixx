export module nameGen;

import std;

// ════════════════════════════════════════════════════════════════════════
// nameGen — 절차생성 지명(도시) 생성기.
//
//   방식 (영어식 합성 지명):
//     - 이름 = 접두 형태소 + 접미 형태소 (Ash+ton=Ashton, Frost+ridge=Frostridge).
//       실제 영어 지명 요소(-ton/-ford/-burg 등)를 써서 "영미권 마을" 톤을 낸다.
//     - 각 형태소가 (라틴, 한글) 렌더를 동시에 보유 → 한국어는 형태소별 음차를 이어붙임
//       (Ash→애시, ton→턴 ⇒ 애시턴). 새 언어 = 형태소에 그 언어 음차 열을 추가하면 끝.
//     - seed로 완전 결정론(난수 전역상태 X). 같은 seed면 항상 같은 형태소 쌍이라
//       매 프레임 동일(지도 무깜빡임), 언어 전환 시 렌더만 바뀐다.
//       → 세이브엔 seed만 있으면 됨(이름 문자열 저장 불필요).
//
//   향후:
//     - 형태소 표를 language/<팩>/cityName.tsv 등으로 외부화(팩별 음차).
//     - 접두/접미 풀 확장으로 조합 수 증가(현재 38×30 ≈ 1140).
// ════════════════════════════════════════════════════════════════════════

export namespace nameGen
{
    //절차도시 이름을 그릴 문자 체계. 같은 seed면 어느 Script든 동일 형태소 쌍 → 서로 음차 관계.
    enum class Script : std::uint8_t { Latin, Hangul };
}

namespace nameGen
{
    //지명 형태소 — 각 형태소가 (라틴, 한글) 렌더를 동시에 보유.
    struct Morph { const wchar_t* latin; const wchar_t* hangul; };

    //접두 형태소 (라틴은 대문자 시작). 접미와 겹치는 요소(Wood/Brook)는 어색한 중복 피해 제외.
    constexpr Morph PREFIX[] = {
        {L"Ash",L"애시"},{L"Black",L"블랙"},{L"North",L"노스"},{L"South",L"사우스"},{L"East",L"이스트"},
        {L"West",L"웨스트"},{L"Red",L"레드"},{L"White",L"화이트"},{L"Gray",L"그레이"},{L"Green",L"그린"},
        {L"Stone",L"스톤"},{L"Iron",L"아이언"},{L"Frost",L"프로스트"},{L"Pine",L"파인"},{L"Oak",L"오크"},
        {L"Bay",L"베이"},{L"Fort",L"포트"},{L"Salt",L"솔트"},{L"Wolf",L"울프"},{L"Crow",L"크로우"},
        {L"Raven",L"레이븐"},{L"Hill",L"힐"},{L"Long",L"롱"},{L"New",L"뉴"},{L"Old",L"올드"},
        {L"High",L"하이"},{L"Fair",L"페어"},{L"Mill",L"밀"},{L"King",L"킹"},{L"Wind",L"윈드"},
        {L"Rock",L"록"},{L"Cold",L"콜드"},{L"Gold",L"골드"},{L"Silver",L"실버"},{L"Clear",L"클리어"},
        {L"Deer",L"디어"},{L"Fox",L"폭스"},{L"Bram",L"브램"},
    };

    //접미 형태소 (라틴은 소문자) — 영어 지명 어미.
    constexpr Morph SUFFIX[] = {
        {L"ton",L"턴"},{L"ford",L"퍼드"},{L"field",L"필드"},{L"haven",L"헤이븐"},{L"burg",L"버그"},
        {L"vale",L"베일"},{L"port",L"포트"},{L"wood",L"우드"},{L"ridge",L"리지"},{L"dale",L"데일"},
        {L"moor",L"무어"},{L"stead",L"스테드"},{L"ham",L"햄"},{L"wick",L"윅"},{L"bury",L"베리"},
        {L"worth",L"워스"},{L"gate",L"게이트"},{L"brook",L"브룩"},{L"cliff",L"클리프"},{L"mere",L"미어"},
        {L"well",L"웰"},{L"side",L"사이드"},{L"bridge",L"브리지"},{L"crest",L"크레스트"},{L"hollow",L"할로"},
        {L"mont",L"몬트"},{L"borough",L"버러"},{L"mouth",L"머스"},{L"fall",L"폴"},{L"glen",L"글렌"},
    };

    //splitmix64 스트림 — seed에서 결정론적 의사난수. 공유 randomEngine(전역상태) 안 씀.
    struct Rng
    {
        std::uint64_t state;
        std::uint64_t next() noexcept
        {
            state += 0x9E3779B97F4A7C15ULL;
            std::uint64_t z = state;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return z ^ (z >> 31);
        }
    };
}

export namespace nameGen
{
    //절차생성 도시 이름 — seed로 결정론, script로 렌더. 접두 + 접미 형태소.
    std::wstring placeName(std::uint64_t seed, Script script)
    {
        Rng rng{ seed };
        std::wstring out;
        const auto pick = [&](const auto& pool) { const Morph& m = pool[rng.next() % std::size(pool)]; out += (script == Script::Hangul) ? m.hangul : m.latin; };
        pick(PREFIX);
        pick(SUFFIX);
        return out;
    }
}

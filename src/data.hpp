#pragma once
#include <Geode/Geode.hpp>
using namespace geode::prelude;

namespace sk {
    struct Note { int pct = 0; std::string text; };

    struct Level {
        std::string key;
        std::array<int, 100> hist{};
        int deaths = 0, best = 0;
        std::vector<Note> notes;
        std::vector<int> recent;
        std::vector<std::pair<int, int>> days;
    };

    struct Session {
        int deaths = 0, best = 0;
        float seconds = 0.f, sinceBreak = 0.f;
    };

    inline Level cur;
    inline Session ses;
    inline double gTime = 0.0;
    inline int gDeaths = 0;
    inline std::vector<int> demons;
    inline bool gLoaded = false;
    inline bool retryPending = false;
    inline float origMusic = -1.f;

    inline std::vector<int> ints(std::string const& s) {
        std::vector<int> v;
        int n = 0;
        bool any = false;
        for (char c : s) {
            if (c >= '0' && c <= '9') { n = n * 10 + (c - '0'); any = true; }
            else if (any) { v.push_back(n); n = 0; any = false; }
        }
        if (any) v.push_back(n);
        return v;
    }

    inline int today() {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);
        return (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
    }

    inline void saveGlobal() {
        auto m = Mod::get();
        m->setSavedValue<std::string>("g-time", std::to_string((long long)gTime));
        m->setSavedValue<std::string>("g-deaths", std::to_string(gDeaths));
        std::string d;
        for (int id : demons) d += std::to_string(id) + ",";
        m->setSavedValue<std::string>("g-demons", d);
    }

    inline void loadGlobal() {
        if (gLoaded) return;
        gLoaded = true;
        auto m = Mod::get();
        auto t = ints(m->getSavedValue<std::string>("g-time", ""));
        if (!t.empty()) gTime = t[0];
        auto d = ints(m->getSavedValue<std::string>("g-deaths", ""));
        if (!d.empty()) gDeaths = d[0];
        demons = ints(m->getSavedValue<std::string>("g-demons", ""));
    }

    inline void save() {
        if (cur.key.empty()) return;
        std::string h;
        for (int i = 0; i < 100; i++) { if (i) h += ","; h += std::to_string(cur.hist[i]); }
        auto m = Mod::get();
        m->setSavedValue<std::string>("h-" + cur.key, h);
        m->setSavedValue<std::string>("s-" + cur.key, fmt::format("{},{}", cur.deaths, cur.best));
        std::string n;
        for (auto const& note : cur.notes) n += fmt::format("{}|{}\n", note.pct, note.text);
        m->setSavedValue<std::string>("n-" + cur.key, n);
        std::string rc;
        for (int v : cur.recent) rc += std::to_string(v) + ",";
        m->setSavedValue<std::string>("r-" + cur.key, rc);
        std::string dy;
        for (auto const& [day, best] : cur.days) dy += fmt::format("{},{},", day, best);
        m->setSavedValue<std::string>("d-" + cur.key, dy);
    }

    inline void load(std::string const& key) {
        cur = Level{};
        cur.key = key;
        auto m = Mod::get();
        auto h = ints(m->getSavedValue<std::string>("h-" + key, ""));
        for (size_t i = 0; i < h.size() && i < 100; i++) cur.hist[i] = h[i];
        auto s = ints(m->getSavedValue<std::string>("s-" + key, ""));
        if (s.size() >= 2) { cur.deaths = s[0]; cur.best = s[1]; }
        cur.recent = ints(m->getSavedValue<std::string>("r-" + key, ""));
        auto dv = ints(m->getSavedValue<std::string>("d-" + key, ""));
        for (size_t i = 0; i + 1 < dv.size(); i += 2) cur.days.push_back({dv[i], dv[i + 1]});
        std::string n = m->getSavedValue<std::string>("n-" + key, "");
        size_t pos = 0;
        while (pos < n.size()) {
            size_t end = n.find('\n', pos);
            if (end == std::string::npos) end = n.size();
            std::string line = n.substr(pos, end - pos);
            pos = end + 1;
            size_t bar = line.find('|');
            if (bar == std::string::npos) continue;
            auto p = ints(line.substr(0, bar));
            if (p.empty()) continue;
            cur.notes.push_back({std::min(p[0], 100), line.substr(bar + 1)});
        }
    }

    inline void record(int pct) {
        cur.recent.push_back(pct);
        if (cur.recent.size() > 30) cur.recent.erase(cur.recent.begin());
        int day = today();
        if (cur.days.empty() || cur.days.back().first != day) cur.days.push_back({day, pct});
        else cur.days.back().second = std::max(cur.days.back().second, pct);
        if (cur.days.size() > 7) cur.days.erase(cur.days.begin());
    }

    inline void addDeath(float pct) {
        int b = std::clamp((int)pct, 0, 99);
        cur.hist[b]++;
        cur.deaths++;
        cur.best = std::max(cur.best, b);
        ses.deaths++;
        ses.best = std::max(ses.best, b);
        gDeaths++;
        record(b);
        save();
        saveGlobal();
    }

    inline void addComplete(int levelId, bool demon) {
        cur.best = 100;
        ses.best = 100;
        record(100);
        if (demon && levelId > 0 && std::find(demons.begin(), demons.end(), levelId) == demons.end()) demons.push_back(levelId);
        save();
        saveGlobal();
    }

    inline std::array<int, 10> consistency() {
        std::array<int, 10> out{};
        for (int s = 0; s < 10; s++) {
            int entered = 0, passed = 0;
            for (int i = (int)cur.recent.size() - 1; i >= 0 && entered < 10; i--) {
                int r = cur.recent[i];
                if (r < s * 10) continue;
                entered++;
                if (r >= (s + 1) * 10) passed++;
            }
            out[s] = entered >= 3 ? passed * 100 / entered : -1;
        }
        return out;
    }

    inline char const* rankName() {
        int n = (int)demons.size();
        if (n >= 50) return "Legend";
        if (n >= 15) return "Elite";
        if (n >= 5) return "Slayer";
        if (n >= 1) return "Hunter";
        return "Rookie";
    }

    inline std::vector<std::string> badges() {
        std::vector<std::string> b;
        if (demons.size() >= 1) b.push_back("First demon");
        if (demons.size() >= 10) b.push_back("10 demons");
        if (demons.size() >= 100) b.push_back("100 demons");
        if (gDeaths >= 1000) b.push_back("1000 deaths");
        if (gTime >= 36000.0) b.push_back("10 hours");
        return b;
    }

    inline void addNote(int pct, std::string text) {
        for (auto& c : text) if (c == '|' || c == '\n') c = ' ';
        cur.notes.push_back({pct, text});
        std::sort(cur.notes.begin(), cur.notes.end(), [](Note const& a, Note const& b) { return a.pct < b.pct; });
        save();
    }

    inline void removeNote(size_t i) {
        if (i < cur.notes.size()) cur.notes.erase(cur.notes.begin() + i);
        save();
    }
}

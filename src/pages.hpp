#pragma once
#include "ui.hpp"

namespace sk {
    inline void deathsPage(CCNode* p) {
        auto& L = cur;
        int mx = 0, total = 0;
        long long sum = 0;
        for (int i = 0; i < 100; i++) { mx = std::max(mx, L.hist[i]); total += L.hist[i]; sum += (long long)i * L.hist[i]; }
        if (mx == 0) {
            text(p, "No deaths recorded yet", "bigFont.fnt", .42f, {180.f, 160.f}, SNOW);
            text(p, "Deaths in practice mode are not counted", "chatFont.fnt", .6f, {180.f, 134.f}, GRY);
            return;
        }
        std::vector<int> order(100);
        for (int i = 0; i < 100; i++) order[i] = i;
        std::sort(order.begin(), order.end(), [&](int a, int b) { return L.hist[a] > L.hist[b]; });
        auto d = CCDrawNode::create();
        p->addChild(d);
        for (int i = 0; i < 100; i++) {
            if (!L.hist[i]) continue;
            bool top = i == order[0] || i == order[1] || i == order[2];
            float h = std::max(2.f, 86.f * L.hist[i] / mx);
            rect(d, 36.f + i * 3.f, 98.f, 2.2f, h, top ? f4(RED) : f4({175, 183, 195}));
        }
        rect(d, 34.f, 96.f, 304.f, 1.5f, f4(SNOW, 0.6f));
        text(p, "0%", "chatFont.fnt", .5f, {36.f, 86.f}, GRY, 0.f);
        text(p, "50%", "chatFont.fnt", .5f, {186.f, 86.f}, GRY);
        text(p, "100%", "chatFont.fnt", .5f, {336.f, 86.f}, GRY, 1.f);
        std::string hard = "Hardest:";
        for (int i = 0; i < 3; i++) {
            if (L.hist[order[i]] > 0) hard += fmt::format("   {}% (x{})", order[i], L.hist[order[i]]);
        }
        text(p, hard.c_str(), "chatFont.fnt", .6f, {180.f, 214.f}, SNOW);
        text(p, fmt::format("Deaths {}    Best {}%    Average {}%", L.deaths, L.best, total ? (int)(sum / total) : 0).c_str(),
             "chatFont.fnt", .6f, {180.f, 72.f}, GRY);
    }

    inline void stat(CCNode* p, float x, float y, std::string const& v, char const* cap, ccColor3B c) {
        text(p, v.c_str(), "bigFont.fnt", .6f, {x, y}, c);
        text(p, cap, "chatFont.fnt", .55f, {x, y - 18.f}, {200, 205, 213});
    }

    inline void sessionPage(CCNode* p) {
        int sec = (int)ses.seconds;
        stat(p, 100.f, 208.f, std::to_string(ses.deaths), "deaths this session", MUS);
        stat(p, 260.f, 208.f, fmt::format("{}:{:02}", sec / 60, sec % 60), "play time", SNOW);
        stat(p, 100.f, 164.f, fmt::format("{}%", ses.best), "best this session", SNOW);
        stat(p, 260.f,164.f, std::to_string(cur.deaths), "deaths on this level", SNOW);
        text(p, fmt::format("Rank: {}    {} demons beaten", rankName(), demons.size()).c_str(), "chatFont.fnt", .65f, {180.f, 122.f}, MUS);
        auto b = badges();
        std::string bl = "Badges: ";
        if (b.empty()) bl += "none yet";
        for (size_t i = 0; i < b.size(); i++) bl += (i ? ", " : "") + b[i];
        auto l = text(p, bl.c_str(), "chatFont.fnt", .55f, {180.f, 102.f}, SNOW);
        float w = l->getContentSize().width * l->getScale();
        if (w > 300.f) l->setScale(l->getScale() * 300.f / w);
        bool on = Mod::get()->getSettingValue<bool>("break-reminder");
        int every = (int)Mod::get()->getSettingValue<int64_t>("break-minutes");
        std::string line = on ? fmt::format("Break reminder every {} min, next in {} min", every,
                                            std::max(0, every - (int)(ses.sinceBreak / 60.f)))
                              : "Break reminder is off in settings";
        text(p, line.c_str(), "chatFont.fnt", .55f, {180.f, 78.f}, GRY);
    }

    inline void progressPage(CCNode* p) {
        auto d = CCDrawNode::create();
        p->addChild(d);
        text(p, "Consistency, last 10 tries in each 10%", "chatFont.fnt", .55f, {180.f, 220.f}, GRY);
        auto c = consistency();
        int weak = -1;
        bool any = false;
        for (int i = 0; i < 10; i++) {
            ccColor3B col = {84, 94, 108};
            if (c[i] >= 0) { any = true; col = c[i] >= 80 ? ccColor3B{120, 190, 130} : c[i] >= 50 ? MUS : RED; }
            if (c[i] >= 0 && c[i] < 80 && (weak < 0 || c[i] < c[weak])) weak = i;
            float x = 33.f + i * 30.5f;
            roundRect(d, x, 186.f, 28.f, 20.f, 4.f, f4(col));
            if (c[i] >= 0) text(p, std::to_string(c[i]).c_str(), "chatFont.fnt", .5f, {x + 14.f, 196.f}, INK);
        }
        text(p, "0%", "chatFont.fnt", .45f, {33.f, 176.f}, GRY, 0.f);
        text(p, "100%", "chatFont.fnt", .45f, {337.f, 176.f}, GRY, 1.f);
        std::string rd = !any ? "Full run: play more, not enough data"
                       : weak < 0 ? "Full run: ready"
                       : fmt::format("Full run: not yet, weakest part {}-{}%", weak * 10, weak * 10 + 10);
        text(p, rd.c_str(), "chatFont.fnt", .65f, {180.f, 158.f}, SNOW);
        text(p, "Best percent per day", "chatFont.fnt", .55f, {180.f, 138.f}, GRY);
        if (cur.days.empty()) {
            text(p, "No runs yet", "chatFont.fnt", .6f, {180.f, 108.f}, GRY);
            return;
        }
        for (size_t i = 0; i < cur.days.size(); i++) {
            int day = cur.days[i].first, best = cur.days[i].second;
            float x = 52.f + i * 40.f, h = std::max(2.f, 28.f * best / 100.f);
            rect(d, x, 92.f, 26.f, h, f4(best >= 100 ? ccColor3B{120, 190, 130} : MUS));
            text(p, fmt::format("{}%", best).c_str(), "chatFont.fnt", .45f, {x + 13.f, 92.f + h + 7.f}, SNOW);
            text(p, fmt::format("{}/{}", (day / 100) % 100, day % 100).c_str(), "chatFont.fnt", .45f, {x + 13.f, 82.f}, GRY);
        }
    }
}

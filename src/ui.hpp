#pragma once
#include "data.hpp"

namespace sk {
    constexpr ccColor3B MUS = {212, 166, 10};
    constexpr ccColor3B GRY = {201, 203, 204};
    constexpr ccColor3B SLT = {107, 118, 133};
    constexpr ccColor3B INK = {42, 49, 61};
    constexpr ccColor3B SNOW = {247, 247, 245};
    constexpr ccColor3B RED = {214, 72, 60};

    inline ccColor4F f4(ccColor3B c, float a = 1.f) { return {c.r / 255.f, c.g / 255.f, c.b / 255.f, a}; }

    inline void rect(CCDrawNode* d, float x, float y, float w, float h, ccColor4F c) {
        CCPoint v[4] = {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
        d->drawPolygon(v, 4, c, 0.f, c);
    }

    inline void roundRect(CCDrawNode* d, float x, float y, float w, float h, float r, ccColor4F c) {
        r = std::min(r, std::min(w, h) / 2.f);
        rect(d, x + r, y, w - 2 * r, h, c);
        rect(d, x, y + r, w, h - 2 * r, c);
        d->drawDot({x + r, y + r}, r, c);
        d->drawDot({x + w - r, y + r}, r, c);
        d->drawDot({x + r, y + h - r}, r, c);
        d->drawDot({x + w - r, y + h - r}, r, c);
    }

    inline void tri(CCDrawNode* d, CCPoint a, CCPoint b, CCPoint c, ccColor3B col) {
        CCPoint v[3] = {a, b, c};
        d->drawPolygon(v, 3, f4(col), 0.f, f4(col));
    }

    inline void quad(CCDrawNode* d, CCPoint a, CCPoint b, CCPoint c, CCPoint e, ccColor3B col) {
        CCPoint v[4] = {a, b, c, e};
        d->drawPolygon(v, 4, f4(col), 0.f, f4(col));
    }

    inline CCNode* pill(char const* text, ccColor3B fill, ccColor3B ink, float w, float h = 26.f, float scale = .38f) {
        auto n = CCNode::create();
        n->setContentSize({w, h});
        n->setAnchorPoint({0.5f, 0.5f});
        auto d = CCDrawNode::create();
        n->addChild(d);
        roundRect(d, 0.f, -2.f, w, h, h * 0.3f, f4(INK, 0.35f));
        roundRect(d, 0.f, 0.f, w, h, h * 0.3f, f4(fill));
        auto l = CCLabelBMFont::create(text, "bigFont.fnt");
        l->setTag(7);
        l->setColor(ink);
        l->setScale(scale);
        l->setPosition({w / 2, h / 2 + 1.f});
        n->addChild(l, 2);
        return n;
    }

    inline CCLabelBMFont* text(CCNode* p, char const* s, char const* font, float scale, CCPoint pos, ccColor3B c, float ax = .5f) {
        auto l = CCLabelBMFont::create(s, font);
        l->setScale(scale);
        l->setPosition(pos);
        l->setColor(c);
        l->setAnchorPoint({ax, .5f});
        p->addChild(l);
        return l;
    }

    inline void wall(CCNode* parent) {
        auto d = CCDrawNode::create();
        parent->addChild(d, 0);
        roundRect(d, -2.f, -2.f, 364.f, 284.f, 12.f, f4(INK, 0.9f));
        roundRect(d, 0.f, 0.f, 360.f, 280.f, 10.f, f4(SNOW));
        tri(d, {8.f, 272.f}, {96.f, 272.f}, {8.f, 214.f}, RED);
        quad(d, {250.f, 272.f}, {352.f, 272.f}, {352.f, 232.f}, {290.f, 232.f}, GRY);
        quad(d, {338.f, 224.f}, {352.f, 224.f}, {352.f, 150.f}, {338.f, 160.f}, MUS);
        tri(d, {8.f, 8.f}, {40.f, 8.f}, {8.f, 52.f}, MUS);
        quad(d, {352.f, 8.f}, {300.f, 8.f}, {320.f, 54.f}, {352.f, 54.f}, GRY);
        roundRect(d, 16.f, 58.f, 328.f, 182.f, 6.f, f4(SNOW));
        roundRect(d, 20.f, 62.f, 320.f, 174.f, 4.f, f4(SLT));
    }
}

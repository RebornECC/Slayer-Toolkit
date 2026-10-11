#pragma once
#include "pages.hpp"

class SlayerPopup : public geode::Popup {
public:
    static inline int s_open = 0;
    SlayerPopup() { s_open++; }
    ~SlayerPopup() override { s_open--; }

protected:
    std::array<CCNode*, 4> m_pages{};
    CCDrawNode* m_ul = nullptr;
    CCNode* m_list = nullptr;
    TextInput* m_pct = nullptr;
    TextInput* m_txt = nullptr;
    int m_del = -1;

    void showPage(int i) {
        for (int j = 0; j < 4; j++) m_pages[j]->setVisible(j == i);
        m_pct->setEnabled(i == 1);
        m_txt->setEnabled(i == 1);
        m_ul->clear();
        sk::rect(m_ul, 72.f + i * 70.f, 239.f, 56.f, 3.f, sk::f4(sk::RED));
    }

    void rebuildNotes() {
        m_list->removeAllChildren();
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        m_list->addChild(menu, 2);
        auto& N = sk::cur.notes;
        if (N.empty()) sk::text(m_list, "No notes yet, add one below", "chatFont.fnt", .6f, {180.f, 170.f}, sk::GRY);
        for (size_t i = 0; i < N.size() && i < 5; i++) {
            float y = 214.f - i * 21.f;
            sk::text(m_list, fmt::format("{}%", N[i].pct).c_str(), "bigFont.fnt", .4f, {34.f, y}, sk::MUS, 0.f);
            auto l = sk::text(m_list, N[i].text.c_str(), "chatFont.fnt", .6f, {86.f, y}, sk::SNOW, 0.f);
            float w = l->getContentSize().width * l->getScale();
            if (w > 210.f) l->setScale(l->getScale() * 210.f / w);
            auto btn = CCMenuItemExt::createSpriteExtra(sk::pill("x", sk::RED, sk::SNOW, 28.f, 18.f, .32f), [this, i](auto) {
                m_del = (int)i;
                this->runAction(CCSequence::create(CCDelayTime::create(0.f),
                    CCCallFunc::create(this, callfunc_selector(SlayerPopup::delNote)), nullptr));
            });
            btn->setPosition({318.f, y});
            menu->addChild(btn);
        }
    }

    void delNote() {
        sk::removeNote((size_t)m_del);
        rebuildNotes();
    }

    void addNote() {
        auto n = sk::ints(m_pct->getString());
        std::string t = m_txt->getString();
        if (n.empty() || n[0] < 1 || n[0] > 100) {
            Notification::create("Enter a percent from 1 to 100", NotificationIcon::Warning)->show();
            return;
        }
        if (t.empty()) {
            Notification::create("Write the note text", NotificationIcon::Warning)->show();
            return;
        }
        if (sk::cur.notes.size() >= 5) {
            Notification::create("Up to 5 notes per level", NotificationIcon::Warning)->show();
            return;
        }
        sk::addNote(n[0], t);
        m_pct->setString("");
        m_txt->setString("");
        rebuildNotes();
    }

    bool init() {
        if (!Popup::init(360.f, 280.f, "GJ_square05.png")) return false;
        m_bgSprite->setVisible(false);
        sk::wall(m_mainLayer);
        m_ul = CCDrawNode::create();
        m_mainLayer->addChild(m_ul, 1);
        for (int i = 0; i < 4; i++) {
            m_pages[i] = CCNode::create();
            m_mainLayer->addChild(m_pages[i], 2);
        }
        sk::deathsPage(m_pages[0]);
        sk::progressPage(m_pages[2]);
        sk::sessionPage(m_pages[3]);

        m_list = CCNode::create();
        m_pages[1]->addChild(m_list);
        m_pct = TextInput::create(54.f, "%", "bigFont.fnt");
        m_pct->setFilter("0123456789");
        m_pct->setMaxCharCount(3);
        m_pct->setPosition({58.f, 90.f});
        m_pages[1]->addChild(m_pct);
        m_txt = TextInput::create(180.f, "note", "chatFont.fnt");
        m_txt->setMaxCharCount(40);
        m_txt->setPosition({180.f, 90.f});
        m_pages[1]->addChild(m_txt);
        auto nm = CCMenu::create();
        nm->setPosition({0.f, 0.f});
        m_pages[1]->addChild(nm, 5);
        auto add = CCMenuItemExt::createSpriteExtra(sk::pill("Add", sk::MUS, sk::INK, 56.f, 28.f), [this](auto) { this->addNote(); });
        add->setPosition({312.f, 90.f});
        nm->addChild(add);
        rebuildNotes();

        auto tabs = CCMenu::create();
        tabs->setPosition({0.f, 0.f});
        m_mainLayer->addChild(tabs, 10);
        char const* names[4] = {"Deaths", "Notes", "Progress", "Session"};
        for (int i = 0; i < 4; i++) {
            auto b = CCMenuItemExt::createSpriteExtra(sk::pill(names[i], sk::GRY, sk::INK, 66.f, 26.f, .3f), [this, i](auto) { this->showPage(i); });
            b->setPosition({100.f + i * 70.f, 257.f});
            tabs->addChild(b);
        }
        showPage(0);
        return true;
    }

public:
    static SlayerPopup* create() {
        auto ret = new SlayerPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "popup.hpp"

namespace sk {
    inline int lastPct = -1;
}

class $modify(SkPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        int id = level->m_levelID.value();
        std::string key = id > 0 ? std::to_string(id) : std::string("local_") + std::string(level->m_levelName);
        sk::loadGlobal();
        sk::load(key, std::string(level->m_levelName));
        sk::ses.goalHit = false;
        sk::lastPct = -1;
        sk::retryPending = false;
        auto win = CCDirector::get()->getWinSize();
        auto hint = CCLabelBMFont::create("", "bigFont.fnt");
        hint->setID("slayer-hint"_spr);
        hint->setColor(sk::MUS);
        hint->setScale(.4f);
        hint->setPosition({win.width / 2, win.height - 30.f});
        hint->setOpacity(0);
        this->addChild(hint, 1000);
        auto timer = CCLabelBMFont::create("0:00", "bigFont.fnt");
        timer->setID("slayer-timer"_spr);
        timer->setScale(.4f);
        timer->setOpacity(200);
        timer->setAnchorPoint({0.f, 1.f});
        timer->setPosition({10.f, win.height - 8.f});
        timer->setVisible(false);
        this->addChild(timer, 1000);
        auto mod = Mod::get();
        auto c = sk::cfg();
        if (mod->getSettingValue<bool>("heat-bar")) {
            auto heat = CCDrawNode::create();
            heat->setID("slayer-heat"_spr);
            heat->setPosition({win.width / 2 - 100.f, win.height - 5.f});
            this->addChild(heat, 1000);
            sk::drawHeat(heat);
        }
        if (c.stream && m_attemptLabel) {
            m_attemptLabel->setVisible(false);
            timer->setVisible(true);
        }
        if (c.silent) {
            auto engine = FMODAudioEngine::sharedEngine();
            if (sk::origMusic < 0.f) sk::origMusic = engine->m_musicVolume;
            engine->setBackgroundMusicVolume(0.f);
        }
        return true;
    }

    void restoreMusic() {
        if (sk::origMusic >= 0.f) {
            FMODAudioEngine::sharedEngine()->setBackgroundMusicVolume(sk::origMusic);
            sk::origMusic = -1.f;
        }
    }

    void doRetry() {
        sk::retryPending = false;
        PlayLayer::delayedResetLevel();
    }

    void delayedResetLevel() {
        float d = sk::cfg().delay;
        if (d <= 0.01f) {
            PlayLayer::delayedResetLevel();
            return;
        }
        if (sk::retryPending) return;
        sk::retryPending = true;
        this->runAction(CCSequence::create(CCDelayTime::create(d),
            CCCallFunc::create(this, callfunc_selector(SkPlayLayer::doRetry)), nullptr));
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (m_isPracticeMode || m_isTestMode) return;
        sk::addComplete(m_level->m_levelID.value(), m_level->m_demon.value() > 0);
    }

    void onQuit() {
        sk::retryPending = false;
        restoreMusic();
        sk::saveGlobal();
        PlayLayer::onQuit();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        sk::lastPct = -1;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!m_player1 || m_player1->m_isDead) return;
        sk::ses.seconds += dt;
        sk::ses.sinceBreak += dt;
        sk::gTime += dt;
        if (auto timer = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("slayer-timer"_spr))) {
            if (timer->isVisible()) {
                int sec = (int)sk::ses.seconds;
                timer->setString(fmt::format("{}:{:02}", sec / 60, sec % 60).c_str());
            }
        }
        auto mod = Mod::get();
        auto c = sk::cfg();
        int p = (int)this->getCurrentPercent();
        int goal = (int)mod->getSettingValue<int64_t>("session-goal");
        if (goal > 0 && !sk::ses.goalHit && p >= goal) {
            sk::ses.goalHit = true;
            Notification::create(fmt::format("Goal reached: {}%", goal), NotificationIcon::Success)->show();
        }
        if (mod->getSettingValue<bool>("break-reminder")) {
            float limit = (float)c.breakMin * 60.f;
            if (sk::ses.sinceBreak >= limit) {
                sk::ses.sinceBreak = 0.f;
                Notification::create(fmt::format("You have been playing for {} min. Time for a break", (int)(limit / 60.f)),
                                     NotificationIcon::Info)->show();
            }
        }
        if (!c.hints || sk::cur.notes.empty()) return;
        if (p == sk::lastPct) return;
        sk::lastPct = p;
        auto hint = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("slayer-hint"_spr));
        if (!hint) return;
        for (auto const& n : sk::cur.notes) {
            if (n.pct - p != 3) continue;
            hint->setString(fmt::format("{}%: {}", n.pct, n.text).c_str());
            hint->setScale(.4f);
            float w = hint->getContentSize().width * hint->getScale();
            float maxW = CCDirector::get()->getWinSize().width - 40.f;
            if (w > maxW) hint->setScale(hint->getScale() * maxW / w);
            hint->stopAllActions();
            hint->setOpacity(255);
            hint->runAction(CCSequence::create(CCDelayTime::create(2.2f), CCFadeOut::create(0.5f), nullptr));
            break;
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        bool wasDead = player->m_isDead;
        PlayLayer::destroyPlayer(player, object);
        if (wasDead || !player->m_isDead || player != m_player1) return;
        if (m_isPracticeMode || m_isTestMode) return;
        sk::addDeath(this->getCurrentPercent());
        if (auto heat = typeinfo_cast<CCDrawNode*>(this->getChildByID("slayer-heat"_spr))) sk::drawHeat(heat);
    }
};

class $modify(SkPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        if (sk::cur.key.empty()) return;
        auto spr = ButtonSprite::create("Slayer", "goldFont.fnt", "GJ_button_01.png", .8f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SkPause::onSlayer));
        auto menu = CCMenu::create();
        menu->setID("slayer-menu"_spr);
        auto win = CCDirector::get()->getWinSize();
        menu->setPosition({win.width - 62.f, 26.f});
        menu->addChild(btn);
        this->addChild(menu, 50);
    }

    void onSlayer(CCObject*) {
        if (SlayerPopup::s_open > 0) return;
        if (auto p = SlayerPopup::create()) p->show();
    }
};

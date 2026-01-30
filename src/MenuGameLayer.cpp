#include "MenuGameLayer.hpp"
#include "GameStatsManager.hpp"
#include "PlayerObject.hpp"

bool ProMenuGameLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    cocos2d::CCPoint touchPos = touch->getLocation();

    auto f = m_fields.self();

    for (PlayerObject* player : f->m_players) {
        cocos2d::CCPoint pos = player->getPosition();

        if (getScaleX() < 0)
            pos.x = abs(CCDirector::get()->getWinSize().width - pos.x);

        if (pos.x <= 50) continue;

        if (ccpDistance(touchPos, pos) < 30) {

            // lazy ass to rewrite original, sorry

            PlayerObject* ogPlayer = m_playerObject;
            m_playerObject = player;

            static_cast<ProGameStatsManager*>(GameStatsManager::get())->m_fields->m_dontIncrement = true;

            destroyPlayer();

            static_cast<ProGameStatsManager*>(GameStatsManager::get())->m_fields->m_dontIncrement = false;

            m_playerObject = ogPlayer;
        }
    }

    return MenuGameLayer::ccTouchBegan(touch, event);
}

void ProMenuGameLayer::updatePlayers(float dt) {
    // if (dt > 0) {
    //     for (int i = 0; i < 10; i++)
    //         updatePlayers(0.f);
        
    //     return;
    // }

    auto f = m_fields.self();

    int currentPlayers = static_cast<int>(f->m_players.size()) + 1;

    f->m_isEmpty = f->m_targetPlayers == 0 && currentPlayers == 1 && !f->m_isEmpty;

    if (currentPlayers == f->m_targetPlayers) return;

    if (currentPlayers < f->m_targetPlayers) {
        if (!f->m_reusablePlayers.empty()) {
            f->m_players.push_back(f->m_reusablePlayers.back());
            f->m_reusablePlayers.pop_back();
            return;
        }

        PlayerObject* player = PlayerObject::create(1, 1, nullptr, this, false);
        player->setTag(12879384);
        player->addAllParticles();

        addChild(player, 60);

        f->m_players.push_back(player);

        static_cast<ProPlayerObject*>(player)->reset(m_videoOptionsOpen);
        static_cast<ProPlayerObject*>(player)->reset(m_videoOptionsOpen); // fix black bug hehiehi

        return;
    }

    if (f->m_players.empty()) return;

    ProPlayerObject* player = static_cast<ProPlayerObject*>(f->m_players.back());

    player->m_fields->m_shouldKill = true;
    player->m_fields->m_menuGameLayer = this;

    f->m_deathRowPlayers.push_back(player);
    f->m_players.pop_back();
}

void ProMenuGameLayer::updateTargetPlayers() {
    int value = Mod::get()->getSettingValue<int64_t>("icons");
    setScaleX(getScaleX() * (value < 0 ? -1 : 1));

    m_fields->m_targetPlayers = abs(value);
}

bool ProMenuGameLayer::init() {
    if (!MenuGameLayer::init()) return false;

    updateTargetPlayers();

    schedule(schedule_selector(ProMenuGameLayer::updatePlayers), 0, kCCRepeatForever, 0.1f);

    return true;
}

void ProMenuGameLayer::tryJump(float dt) {
    MenuGameLayer::tryJump(dt);

    float bottom = CCDirector::get()->getScreenBottom();

    auto f = m_fields.self();

    for (PlayerObject* player : f->m_players)
        static_cast<ProPlayerObject*>(player)->tryJump(m_isDestroyingPlayer);

    for (PlayerObject* player : f->m_deathRowPlayers)
        static_cast<ProPlayerObject*>(player)->tryJump(m_isDestroyingPlayer);
}

void ProMenuGameLayer::update(float dt) {
    MenuGameLayer::update(dt);

    auto f = m_fields.self();

    if (f->m_isEmpty)
        m_playerObject->setPosition({-500, 105});

    for (PlayerObject* player : f->m_players)
        static_cast<ProPlayerObject*>(player)->updateMenu(dt, m_deltaCount, m_videoOptionsOpen);

    for (PlayerObject* player : f->m_deathRowPlayers)
        static_cast<ProPlayerObject*>(player)->updateMenu(dt, m_deltaCount, m_videoOptionsOpen);
}

void ProMenuGameLayer::playerKilledItself(PlayerObject* player) {
    auto f = m_fields.self();

    static_cast<ProPlayerObject*>(player)->m_fields->m_shouldKill = false;

    f->m_deathRowPlayers.erase(std::remove(f->m_deathRowPlayers.begin(), f->m_deathRowPlayers.end(), player), f->m_deathRowPlayers.end());
    f->m_reusablePlayers.push_back(player);
}
#include "KnownPlayers.hpp"
#include "PlayerObject.hpp"

void ProPlayerObject::updateMenu(float dt, float deltaCount, bool videoOptionsOpen) {
    m_shipRotation = getPosition();

    updateInternalActions(dt);
    update(dt * 60.f);

    m_totalTime = deltaCount;

    m_waveTrail->m_pulseSize = 1.4f;

    float height = 0.f;

    if (m_vehicleSize != 1.f)
        height = (1.f - m_vehicleSize) * m_unkAngle1 * 0.5;

    height = (CCDirector::get()->getScreenBottom() + 90.f + m_unkAngle1 * 0.5) - height;

    if (getPosition().y < height) {
        setPosition({getPosition().x, height});
        hitGround(nullptr, false);
    }

    updateRotation(dt * 60.f);

    if (getPosition().x >= CCDirector::get()->getWinSize().width + 100.f)
        m_fields->m_shouldKill ? meetGrimReaper() : reset(videoOptionsOpen);

    m_waveTrail->updateStroke(dt);
}

void ProPlayerObject::tryJump(bool isDestroyingPlayer) {
    float chance = 0.2f;
    float bottom = CCDirector::get()->getScreenBottom();

    if (isDestroyingPlayer) chance = 0.4f;

    float y = getPosition().y + 4.f;

    if (m_isShip) {
        float threshold = bottom + 205.f;
        chance = y <= threshold && y != threshold ? 0.3f : 0.f;
    } 
    else if (m_isBird) {
        float threshold = bottom + 305.f;
        chance = y <= threshold && y != threshold ? 0.4f : 0.f;
    } 
    else if (m_isDart) {
        float threshold = bottom + 165.f;
        chance = y <= threshold && y != threshold ? 1.f : (y <= CCDirector::get()->getWinSize().height - 75.f ? 0.5f : 0.f);
    } 
    else if (m_isSwing) {
        if (!m_isUpsideDown)
            chance = y <= 150.f && y != 150.f ? 0.9f : 0.f;
        else
            chance = y > 210.f ? 0.9f : 0.f;
    } 
    else if (m_isBall || m_isSpider)
        chance = -1.f;

    static_cast<float>(rand()) / RAND_MAX <= chance && (
        m_isOnGround2
        || m_isShip
        || m_isBird
        || m_isDart
        || m_isSwing
    )
    ? pushButton(PlayerButton::Jump)
    : releaseButton(PlayerButton::Jump);
}

void ProPlayerObject::reset(bool videoOptionsOpen) {
    GameManager* gm = GameManager::get();

    m_regularTrail->stopStroke();

    if (m_fadeOutStreak) {
        m_fadeOutStreak = false;
        fadeOutStreak2(m_playEffects ? 0.2f : 0.6f);
    }

    if (m_hasGroundParticles) {
        m_playerGroundParticles->stopSystem();
        m_hasGroundParticles = false;
    }

    setPosition({
        -100.f - static_cast<float>(rand()) / RAND_MAX * 500.f,
        getPosition().y + 4.f
    });

    resetAllParticles();
    toggleFlyMode(false, false);
    toggleBirdMode(false, false);
    toggleRollMode(false, false);
    toggleDartMode(false, false);
    toggleRobotMode(false, false);
    toggleSpiderMode(false, false);
    toggleSwingMode(false, false);

    if (geode::Loader::get()->isModLoaded("iandyhd3.known_players")) {
        auto event = known_players::events::NextIconModifyPlayerObject(this);
        event.send(this);

        // if (event.done) return;
        return;
    }

    m_hasGlow = static_cast<float>(rand()) / RAND_MAX > 0.8f;

    int colorGlow = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 108.f);
    int color1 = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 108.f);
    int color2 = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 108.f);

    if (m_hasGlow)
        enableCustomGlowColor(gm->colorForIdx(colorGlow));
    else
        disableCustomGlowColor();

    setColor(gm->colorForIdx(color1));
    setSecondColor(gm->colorForIdx(color2));
    flipGravity(false, false);
    update(0.f);

    float r = static_cast<float>(rand()) / RAND_MAX;

    if (!videoOptionsOpen) {
        if (r < 0.12f && !m_isShip) {
            toggleFlyMode(true, false);
            int frame = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 169.f + 0.5f);
            updatePlayerShipFrame(frame);
        } else if (r < 0.24f && !m_isBall) {
            toggleRollMode(true, false);
            releaseButton(PlayerButton::Jump);
            int frame = std::max(1, static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 118.f + 0.5f));
            updatePlayerRollFrame(frame);
        } else if (r < 0.36f && !m_isBird) {
            toggleBirdMode(true, false);
            int frame = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 149.f + 0.5f);
            updatePlayerBirdFrame(frame);
        } else if (r < 0.48f || m_isDart) {
            toggleDartMode(true, false);
            int frame = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 96.f + 0.5f);
            updatePlayerDartFrame(frame);
        } else if (r < 0.6f || m_isRobot) {
            toggleRobotMode(true, false);
            int frame = std::max(1, std::min(68, static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 68.f + 0.5f)));
            createRobot(frame);
        } else if (r < 0.7f || m_isSpider) {
            toggleSpiderMode(true, false);
            releaseButton(PlayerButton::Jump);
            int frame = std::max(1, std::min(69, static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 69.f + 0.5f)));
            createSpider(frame);
        } else if (r < 0.8f || m_isSwing) {
            toggleSwingMode(true, false);
            releaseButton(PlayerButton::Jump);
            int frame = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 43.f + 0.5f);
            updatePlayerSwingFrame(frame);
        } else {
            toggleFlyMode(false, false);
            toggleBirdMode(false, false);
            toggleRollMode(false, false);
            toggleDartMode(false, false);
            toggleRobotMode(false, false);
            toggleSpiderMode(false, false);
            toggleSwingMode(false, false);

            updatePlayerFrame(
                std::max(1, static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 485.f + 0.5f))
            );
        }

        togglePlayerScale((static_cast<float>(rand()) / RAND_MAX) <= 0.1f, false);
    }

    updateGlowColor();

    float timeModRand = static_cast<float>(rand()) / RAND_MAX;
    float timeMod = 1.3f;
    if (timeModRand >= 0.65f) timeMod = 0.9f;
    else if (timeModRand >= 0.6f) timeMod = 1.6f;
    else if (timeModRand >= 0.4f) timeMod = 1.1f;

    updateTimeMod(timeMod, false);

    if (m_isDart) {
        resetStreak();
        placeStreakPoint();
    }

    m_waveTrail->updateStroke(0.f);
}

void ProPlayerObject::meetGrimReaper() {
    auto f = m_fields.self();

    if (f->m_menuGameLayer)
        f->m_menuGameLayer->playerKilledItself(static_cast<PlayerObject*>(this));

    m_regularTrail->stopStroke();

    if (m_fadeOutStreak)
        m_fadeOutStreak = false;

    if (m_hasGroundParticles) {
        m_playerGroundParticles->stopSystem();
        m_hasGroundParticles = false;
    }

    if (m_isDart) {
        toggleDartMode(false, false);
        m_isDart = false;
    }
    
    resetStreak();
    placeStreakPoint();
    m_waveTrail->updateStroke(0.f);
    
    resetAllParticles();
}

#include "Turret.h"
#include "Root/Root.h"
#include "World/WorldManager.h"
#include "Common/Utils.h"
#include "Effects/VFXEmitter.h"
#include "Objects/Throwable/ThunderCharge.h"
#include "Soldier.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Turret, "Overkill/Characters", "Turret")

	META_DATA_DESC(Turret)
		BASE_SCENE_ENTITY_PROP(Turret)
        FLOAT_PROP(Turret, leftAngleVaule,    0.f, "Turret", "leftAngle", "Left Angle")
        FLOAT_PROP(Turret, rightAngleVaule, 360.f, "Turret", "rightAngle", "Right Angle")
        BOOL_PROP(Turret, clampEditorVisualization, true, "Turret", "clampEditorVisualization", "Clamp Editor Visualization")

        INT_PROP(Turret, killScore, 250, "Turret", "killScore", "Kill Score")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(Turret, alwaysViewRadius, 50.0f, "Turret", "alwaysViewRadius", "Always View Radius")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(Turret, keepPlayerVisibleMaxTime, 0.25f, "Turret", "keepPlayerVisibleMaxTime", "Keep player is sight during time")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(Turret, rotateSpeed, 600.0f, "Turret", "rotateSpeed", "Rotate Speed")
        MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(Turret, agroTime, 1.0f, "Turret", "agroTime", "Agro Time")
        MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

    void Turret::Init()
	{
        Enemy::Init();

        if (prefabInstance)
        {
            Tasks(true)->AddTask(0, this, (Object::Delegate)&Turret::EditorDraw);
        }
	}

    void Turret::Play()
    {
        Enemy::Play();

        baseSprite = FindChild<SpriteEntity>("Base");

        if (baseSprite)
        {
            baseElectro = baseSprite->FindChild<AnimGraph2D>("electro");
        }

        gunSprite  = FindChild<SpriteEntity>("Gun");

        if (gunSprite)
        {
            gunElectro = gunSprite->FindChild<AnimGraph2D>("electro");
        }

        shootNode = FindChild<Node2D>("shoot");
        searchLightNode = FindChild<Node2D>("searchLight");
        shootFlash = FindChild<PointLight2D>("flash");
		shootMuzzle = FindChild<SpriteEntity>("muzzle");
        shells = FindChild<VFXEmitter>("shells");

		if (shootFlash)
		{
			shootFlash->SetVisiblity(false);
		}

		if (shootMuzzle)
		{
			shootMuzzle->SetVisiblity(false);
		}

        if (shells)
		{
			shells->ResetAndStop();
		}

        leftAngle = Utils::Angle::Degrees(leftAngleVaule - transform.rotation.z);
        rightAngle = Utils::Angle::Degrees(rightAngleVaule - transform.rotation.z);

        if (leftAngle > rightAngle)
        {
            eastl::swap(leftAngle, rightAngle);
        }

        aimingAngle = leftAngle;

        controller->SetPosition(transform.position + controller->GetTransform().position);

        if (PhysTriger2D* trigger = FindChild<PhysTriger2D>())
        {
            trigger->SetPhysGroup(PhysGroup::DeathZone);
            trigger->AddDelegate<eastl::function<void(int, SceneEntity*, int)>>("OnContactStart", this,
                [this](int index, SceneEntity* entity, int contactIndex)
                {
                    if (Player* player = dynamic_cast<Player*>(entity->GetParent()))
                    {
                        if (!WorldManager::instance->IsPlayerKilled() && !killed)
                        {
                            WorldManager::instance->KillPlayer({}, DeathSource::Elecro);

                            if (baseElectro)
                            {
                                baseElectro->anim.GotoNode("Anim", true);
                            }

                            if (gunElectro)
                            {
                                gunElectro->anim.GotoNode("Anim", true);
                            }
                        }
                    }
                });
        }
    }

    Math::Vector3 Turret::GetAimPos()
    {
        if (searchLightNode)
		{
			return Sprite::ToPixels(searchLightNode->GetTransform().GetGlobal().Pos());
		}

        return Sprite::ToPixels(gunSprite->GetTransform().GetGlobal().Pos());
    }

    bool Turret::IsPlayerVisibleInSector()
    {      
        auto pos = transform.position;

        auto playerPos = WorldManager::instance->GetPlayerPos();
        auto halfScreenSize = Sprite::GetHalfScreenSize();
        halfScreenSize.x = Sprite::GetPixelsHeight() * 0.5f * 16.0f / 9.0f - 36.0f * 1.75f;

        float upMargin = -36.0f * 4.0f;
        float downMargin = 18.0f;

        if (playerPos.x - halfScreenSize.x > pos.x || pos.x > playerPos.x + halfScreenSize.x ||
            playerPos.y - halfScreenSize.y + downMargin > pos.y || pos.y > playerPos.y + halfScreenSize.y - upMargin)
        {
            return false;
        }

        const Utils::Angle worldAngle = aimingAngle + Utils::Angle::Degrees(transform.rotation.z);
        return WorldManager::instance->IsPlayerVisibleInSector(GetAimPos(), viewDistance, worldAngle.ToSignedDegrees(), viewAngle, alwaysViewRadius);
    }

    Utils::Angle Turret::GetAngleToPlayer()
    {
		const Utils::Angle angleToPlayer = WorldManager::instance->GetAngleToPlayer(GetAimPos());
        return angleToPlayer - Utils::Angle::Degrees(transform.rotation.z);
    }

    void Turret::UpdateGunSpriteScale(Utils::Angle centerAngle)
    {
        if (!gunSprite)
        {
            return;
        }

        auto& gunTm = gunSprite->GetTransform();

        Math::Vector3 rotation = gunTm.rotation;

        /*float angle = centerAngle.ToSignedDegrees() + transform.rotation.z;
        if (angle < 0.0f)
        {
            angle += 360.0f;
        }

        flipped = 90.0f < angle && angle < 270.0f;*/

        Utils::Angle worldAngle = Utils::AdjustAngleByDirection(aimingAngle, GetDirection());
        gunTm.rotation = Utils::Vector::xyV(rotation, worldAngle.ToSignedDegrees());

        auto scale = gunTm.scale;
        gunTm.scale = Math::Vector3(GetDirection(), 1.0f, 1.0f);
    }

    float Turret::GetTimeToAttack()
    {
        return 1.0f * WorldManager::instance->GetDifficulty()->reactionMul;
    }

    void Turret::EditorDraw(float dt)
    {
        if (!GetScene()->IsPlaying())
        {
            Utils::Angle left = Utils::Angle::Degrees(leftAngleVaule);
            Utils::Angle right = Utils::Angle::Degrees(rightAngleVaule);

            Utils::Angle centerAngle = (left + right) * 0.5f;

            if (left > right)
            {
                eastl::swap(left, right);
            }

            if (!gunSprite)
            {
                gunSprite = FindChild<SpriteEntity>("Gun");
            }

            UpdateGunSpriteScale(centerAngle);

            if (!searchLightNode)
            {
                searchLightNode = FindChild<Node2D>("searchLight");
            }

            auto center = Sprite::ToPixels(gunSprite ? gunSprite->GetTransform().GetGlobal().Pos() : GetTransform().GetGlobal().Pos());            
            auto offset = searchLightNode ? (Sprite::ToPixels(searchLightNode->GetTransform().GetGlobal().Pos()) - center).Length() : 0.0f;            

            float dist = clampEditorVisualization ? editorViewDistance : viewDistance;

            Utils::DebugTrinagle(center, left, right, dist, COLOR_CYAN);
            Sprite::DebugLine(center + left.ToDirection() * offset, center + right.ToDirection() * offset, COLOR_CYAN);

            Utils::DebugTrinagle(center + left.ToDirection() * offset, left - Utils::Angle::Degrees(22.5f), left + Utils::Angle::Degrees(22.5f), dist, COLOR_YELLOW);
            Utils::DebugTrinagle(center + right.ToDirection() * offset, right - Utils::Angle::Degrees(22.5f), right + Utils::Angle::Degrees(22.5f), dist, COLOR_YELLOW);
        }
    }

    void Turret::Kill(Math::Vector3 killDir, DeathSource deathSource)
    {
        gunSprite->SetVisiblity(false);

        Enemy::Kill(killDir, deathSource);
    }

    void Turret::StartRageAttack()
    {
        state = State::RageAttacking;

        waitTime = 1.0f;
        timeToAttackPlayer = 0.0f;

        shootTime = shootTimeDelay;
        shootsBeforeStop = shootsMaxBeforeStop;

        curShootCount = 0;     

        tragetAimingAngle = aimingAngle;
        
        float distance = 36.0f * 10.0f;
        auto center = GetAimPos();

        eastl::vector<PhysScene::BodyUserData*> hitBodies;
        GetRoot()->GetPhysScene()->OverlapWithSphere(Sprite::ToUnits(center), Sprite::ToUnits(distance), PhysGroup::Enemy_, hitBodies);

        const Utils::Angle worldAngle = aimingAngle + Utils::Angle::Degrees(transform.rotation.z);
        float angle = worldAngle.ToSignedDegrees();

        bool targetFound = false;

        float totalAngleToEnemy = 0.0f;
        int hitEnemies = 0;

        for (auto& body : hitBodies)
        {
            if (auto* enemy = dynamic_cast<Soldier*>(body->object->GetParent()))
            {
                auto targetPos = enemy->GetTransform().position;

                if (Utils::RayCastLine(Utils::Vector::xy(center), Utils::Vector::xy(targetPos + Math::Vector3(0.0f, 25.0f, 0.0f)), PhysGroup::WorldDoors))
                {
                    continue;
                }
                    
                if (Math::IsPointInSector(Utils::Vector::xy(targetPos), Utils::Vector::xy(center), Utils::ToRadian(angle), distance, Utils::ToRadian(viewAngle), false) ||
                    Math::IsPointInSector(Utils::Vector::xy(targetPos + Math::Vector3(0.0f, 25.0f, 0.0f)), Utils::Vector::xy(center), Utils::ToRadian(angle), distance, Utils::ToRadian(viewAngle), false) ||
                    Math::IsPointInSector(Utils::Vector::xy(targetPos + Math::Vector3(0.0f, 50.0f, 0.0f)), Utils::Vector::xy(center), Utils::ToRadian(angle), distance, Utils::ToRadian(viewAngle), false))
                {
                    const Math::Vector3 dir = targetPos + Math::Vector3(0.0f, 25.0f, 0.0f) - center;                    
                    totalAngleToEnemy += Math::SafeAtan2(dir.y, dir.x);
                    hitEnemies++;

                    targetFound = true;
                }
            }
        }

        if (targetFound && hitEnemies == 1)
        {
            const Utils::Angle angleToEnemy = Utils::Angle::Radian(totalAngleToEnemy / (float)hitEnemies);
            tragetAimingAngle = angleToEnemy - Utils::Angle::Degrees(transform.rotation.z);
        }        
    }

    void Turret::Update(float dt)
    {
        if (WorldManager::instance->GetPlayer() && WorldManager::instance->GetFrontendState() != FrontendState::Gameplay)
        {
            return;
        }

        if (!gunSprite)
		{
			return;
		}

        if (killed)
        {
            return;
        }

        if (state != State::Attacking || isStunned)
		{
			if (shootFlash)
			{
				shootFlash->SetVisiblity(false);
			}

			if (shootMuzzle)
			{
				shootMuzzle->SetVisiblity(false);
			}

            if (shells)
			{
				shells->Stop();
			}
		}

        if (stunTimer > 0.f)
		{
			stunTimer -= dt;

			if (stunTimer <= 0.f)
			{
				stunTimer = -1.f;
				isStunned = false;
                stunSource->OnThunderDisposed();

                state = State::Watching;
			}
		}

        if (isStunned)
        {
            return;
        }

        bool playerVisibile = IsPlayerVisibleInSector();
        if (playerVisibile || !Utils::RayHitLine(GetAimPos(), WorldManager::instance->GetPlayerPos(), PhysGroup::WorldDoors))
        {
            lastPlayerAngle = GetAngleToPlayer();
            lastPlayerSnapshot = WorldManager::instance->GetPlayerSnapshot();
        }

        if (playerVisibile)
		{
			keepPlayerVisibleTime = keepPlayerVisibleMaxTime;
		}

		if (keepPlayerVisibleTime > 0.f)
		{
			keepPlayerVisibleTime -= dt;
			playerVisibile = true;
		}

		if (state == State::Rotating)
        {
            aimingAngle = aimingAngle.MoveTo(toLeftAngle ? leftAngle : rightAngle, Utils::ToRadian(35.0f) * dt);

            if (toLeftAngle)
            {
                aimingAngle = aimingAngle.Clamp(aimingAngle, rightAngle);
            }
            else
            {
                aimingAngle = aimingAngle.Clamp(leftAngle, aimingAngle);
            }

            if (aimingAngle.IsEqual(leftAngle) || aimingAngle.IsEqual(rightAngle))
            {
                toLeftAngle = !toLeftAngle;

                waitTime = 4.0f;

                state = State::Watching;
            }

			if (playerVisibile)
            {
                Utils::PlaySoundEvent("event:/Turret/Turret trig", &transform.position);

                timeToAttackPlayer = GetTimeToAttack();
				state = State::Wondering;
                WorldManager::instance->ChangeAlarmCount(true);
                tragetAimingAngle = lastPlayerAngle;
            }
        }
		else if (state == State::Watching)
        {
			waitTime -= dt;

			if (waitTime < 0.0f)
            {
				state = State::Rotating;
            }

			if (playerVisibile)
            {
                Utils::PlaySoundEvent("event:/Turret/Turret trig", &transform.position);

                timeToAttackPlayer = GetTimeToAttack();
                state = State::Wondering;
                WorldManager::instance->ChangeAlarmCount(true);
                tragetAimingAngle = lastPlayerAngle;
            }
        }
		else if (state == State::Wondering)
        {
            if (playerVisibile)
            {
                timeToAttackPlayer -= dt / agroTime;

                tragetAimingAngle = lastPlayerAngle;

                if (timeToAttackPlayer < 0.0f)
                {
                    state = State::Attacking;
                    shootTime = shootTimeDelay;
                    shootsBeforeStop = shootsMaxBeforeStop;
                    curShootCount = 0;
                }
            }
            else
            {
                timeToAttackPlayer += dt * 0.66f;

                if (timeToAttackPlayer > GetTimeToAttack())
                {
                    toLeftAngle = aimingAngle.Distance(leftAngle) < aimingAngle.Distance(rightAngle);

                    state = State::Rotating;
                    WorldManager::instance->ChangeAlarmCount(false);
                }
            }
        }
		else if (state == State::Attacking || state == State::RageAttacking)
        {
			if (state == State::Attacking && shootsBeforeStop == 0 && !playerVisibile)
            {
                timeToAttackPlayer = 0.0f;
			    state = State::Wondering;
            }
            else
            if (state == State::RageAttacking)
            {
                waitTime -= dt;

                if (waitTime <= 0.0f)
                {
                    state = State::Wondering;
                }
            }

            bool isShooting = false;

            if (shootsBeforeStop > 0)
            {
                if (state == State::Attacking)
                {
                    tragetAimingAngle = lastPlayerAngle;
                }

                shootTime -= dt;

                isShooting = shootTime < 0.01f;
			    if (isShooting)
                {
                    if (state == State::Attacking)
                    {
                        if (!playerVisibile)
                        {
                            shootsBeforeStop--;
                        }
                        else
                        {
                            shootsBeforeStop = shootsMaxBeforeStop;
                        }
                    }

                    shootTime = shootTimeDelay;
                    shootEffectTime = shootEffectMaxTime;

                    const Math::Vector3 shootPos = Sprite::ToPixels(shootNode->GetTransform().GetGlobal().Pos());
                    const Math::Vector3 shootDir = (state == State::Attacking) ? WorldManager::instance->CalcShootingDirToPlayer(shootPos, curShootCount, lastPlayerSnapshot) : (aimingAngle + Utils::Angle::Degrees(transform.rotation.z)).ToDirection();
                    curShootCount++;

                    WorldManager::instance->AddBulletProjectile(shootPos, shootDir * WorldManager::instance->maxBulletSpeed, 1500.f, true);

                    auto dir = WorldManager::instance->GetPlayerPos() - transform.position;
                    auto len = dir.Normalize();

                    Utils::PlaySoundEvent("event:/Turret/Turret shoot", &transform.position);
                }
            }

            const float isShootEffectVisible = shootEffectTime > 0.f;
            shootEffectTime -= dt;

            if (shootFlash)
            {
                shootFlash->SetVisiblity(isShootEffectVisible);
            }

            if (shootMuzzle)
            {
                shootMuzzle->SetVisiblity(isShootEffectVisible);
            }

            if (shells && isShooting)
			{
				shells->Start();
			}
        }

		if ((state == State::Attacking || state == State::RageAttacking || state == State::Wondering) && !aimingAngle.IsEqual(tragetAimingAngle))
        {
            aimingAngle = aimingAngle.MoveTo(tragetAimingAngle, Utils::ToRadian(rotateSpeed) * dt);
        }        

        UpdateGunSpriteScale(aimingAngle);

        float agrometr = 0.0f;

        if (state == State::Wondering)
        {
            agrometr = 1.0f - timeToAttackPlayer / GetTimeToAttack();
        }
        else
        if (state == State::Attacking || state == State::RageAttacking)
        {
            agrometr = 1.0f;
        }
        
        const Utils::Angle worldAngle = aimingAngle + Utils::Angle::Degrees(transform.rotation.z);
        WorldManager::instance->AddViewCone(GetAimPos(), worldAngle, Utils::Angle::Degrees(22.5f), viewDistance, agrometr);
    }

    void Turret::OnRestart(const EventOnRestart &evt)
	{
		Enemy::OnRestart(evt);

        aimingAngle = leftAngle;
        toLeftAngle = false;
        tragetAimingAngle = aimingAngle;
		state = State::Rotating;
        curShootCount = 0;
        shootEffectTime = -1.f;
        keepPlayerVisibleTime = 0.f;

        float angle = aimingAngle.ToSignedDegrees() + transform.rotation.z;
        if (angle < 0.0f)
        {
            angle += 360.0f;
        }

        flipped = 90.0f < angle && angle < 270.0f;

        stunTimer = -1.f;
        isStunned = false;

        gunSprite->SetVisiblity(true);

        if (shootFlash)
		{
			shootFlash->SetVisiblity(false);
		}

		if (shootMuzzle)
		{
			shootMuzzle->SetVisiblity(false);
		}

        if (shells)
		{
			shells->ResetAndStop();
		}

        if (baseElectro)
        {
            baseElectro->anim.Reset();
        }

        if (gunElectro)
        {
            gunElectro->anim.Reset();
        }

        controller->SetPosition(Sprite::ToPixels(controller->GetTransform().GetGlobal().Pos()));
    }

    void Turret::OnThunderHit(Math::Vector3 hitDir, ThunderCharge *source)
	{
		Utils::AddCameraShake(0.35f);

		isStunned = true;
		stunTimer = 5.f;
        stunSource = source;
	}
}
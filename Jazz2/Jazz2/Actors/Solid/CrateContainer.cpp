﻿#include "CrateContainer.h"
#include "../../LevelInitialization.h"
#include "../../ILevelHandler.h"
#include "../../Tiles/TileMap.h"

#include "../Player.h"
#include "../Weapons/ShotBase.h"

namespace Jazz2::Actors::Solid
{
	CrateContainer::CrateContainer()
	{
	}

	void CrateContainer::Preload(const ActorActivationDetails& details)
	{
		PreloadMetadataAsync("Object/CrateContainer"_s);
	}

	Task<bool> CrateContainer::OnActivatedAsync(const ActorActivationDetails& details)
	{
		Movable = true;
		CollisionFlags |= CollisionFlags::SkipPerPixelCollisions;

		EventType eventType = (EventType)*(uint16_t*)&details.Params[0];
		int count = (int)*(uint16_t*)&details.Params[2];
		if (eventType != EventType::Empty && count > 0) {
			AddContent(eventType, count, &details.Params[4], 16 - 4);
		}

		co_await RequestMetadataAsync("Object/CrateContainer"_s);

		SetAnimation(AnimState::Idle);

		co_return true;
	}

	bool CrateContainer::OnHandleCollision(ActorBase* other)
	{
		if (_health == 0) {
			return SolidObjectBase::OnHandleCollision(other);
		}

		if (auto shotBase = dynamic_cast<Weapons::ShotBase*>(other)) {
			DecreaseHealth(shotBase->GetStrength(), other);
			return true;
		} /*else if (auto shotTnt = dynamic_cast<Weapons::ShotTNT*>(other)) {
			// TODO: TNT
		}*/ else if (auto player = dynamic_cast<Player*>(other)) {
			if (player->CanBreakSolidObjects()) {
				DecreaseHealth(INT32_MAX, other);
				return true;
			}
		}

		return SolidObjectBase::OnHandleCollision(other);
	}

	bool CrateContainer::OnPerish(ActorBase* collider)
	{
		CollisionFlags = CollisionFlags::None;

		CreateParticleDebris();

		PlaySfx("Break"_s);

		CreateSpriteDebris("CrateShrapnel1"_s, 3);
		CreateSpriteDebris("CrateShrapnel2"_s, 2);

		SetTransition(AnimState::TransitionDeath, false, [this, collider]() {
			GenericContainer::OnPerish(collider);
		});
		SpawnContent();
		return true;
	}
}
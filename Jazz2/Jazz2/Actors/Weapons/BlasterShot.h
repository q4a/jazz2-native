﻿#pragma once

#include "ShotBase.h"

namespace Jazz2::Actors::Weapons
{
	class BlasterShot : public ShotBase
	{
	public:
		BlasterShot();
		void OnFire(const std::shared_ptr<ActorBase>& owner, Vector2f gunspotPos, Vector2f speed, float angle, bool isFacingLeft);
		bool OnHandleCollision(ActorBase* other) override;

		WeaponType GetWeaponType() override {
			return WeaponType::Blaster;
		}

	protected:
		Task<bool> OnActivatedAsync(const ActorActivationDetails& details) override;
		void OnUpdate(float timeMult) override;
		void OnUpdateHitbox() override;
		void OnEmitLights(SmallVectorImpl<LightEmitter>& lights) override;
		bool OnPerish(ActorBase* collider) override;
		void OnHitWall() override;
		void OnRicochet() override;

	private:
		Vector2f _gunspotPos;
		bool _fired;
	};
}
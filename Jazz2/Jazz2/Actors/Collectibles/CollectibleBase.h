﻿#pragma once

#include "../../ActorBase.h"

namespace Jazz2::Actors
{
	class Player;
}

namespace Jazz2::Actors::Collectibles
{
	class CollectibleBase : public ActorBase
	{
	public:
		CollectibleBase();

	protected:
		static constexpr int IlluminateLightCount = 20;

		struct IlluminateLight {
			float Intensity;
			float Distance;
			float Phase;
			float Speed;
		};

		bool _untouched;
		uint32_t _scoreValue;

		Task<bool> OnActivatedAsync(const ActorActivationDetails& details) override;
		void OnUpdate(float timeMult) override;
		void OnEmitLights(SmallVectorImpl<LightEmitter>& lights) override;
		bool OnHandleCollision(ActorBase* other) override;

		virtual void OnCollect(Player* player);

		void SetFacingDirection();

	private:
		float _phase, _timeLeft;
		float _startingY;
		SmallVector<IlluminateLight, 0> _illuminateLights;
	};
}
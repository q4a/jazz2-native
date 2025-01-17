﻿#pragma once

#include "EnemyBase.h"

namespace Jazz2::Actors::Enemies
{
	class Dragonfly : public EnemyBase
	{
	public:
		Dragonfly();
		~Dragonfly();

		static void Preload(const ActorActivationDetails& details);

	protected:
		Task<bool> OnActivatedAsync(const ActorActivationDetails& details) override;
		void OnUpdate(float timeMult) override;
		void OnHitWall() override;
		void OnHitFloor() override;
		void OnHitCeiling() override;
		bool OnPerish(ActorBase* collider) override;

	private:
		static constexpr int StateIdle = 0;
		static constexpr int StateAttacking = 1;
		static constexpr int StateBraking = 2;

		int _state = StateIdle;
		float _idleTime;
		float _attackCooldown;
		Vector2f _direction;
		std::shared_ptr<AudioBufferPlayer> _noise;
	};
}
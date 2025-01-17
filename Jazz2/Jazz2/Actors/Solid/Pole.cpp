﻿#include "Pole.h"
#include "../../LevelInitialization.h"
#include "../../ILevelHandler.h"
#include "../Weapons/ShotBase.h"

namespace Jazz2::Actors::Solid
{
	Pole::Pole()
		:
		_fall(FallDirection::None),
		_angleVel(0.0f),
		_angleVelLast(0.0f),
		_fallTime(0.0f),
		_bouncesLeft(BouncesMax)
	{
	}

	void Pole::Preload(const ActorActivationDetails& details)
	{
		uint8_t theme = details.Params[0];
		switch (theme) {
			default:
			case 0: PreloadMetadataAsync("Pole/Carrotus"_s); break;
			case 1: PreloadMetadataAsync("Pole/Diamondus"_s); break;
			case 2: PreloadMetadataAsync("Pole/DiamondusTree"_s); break;
			case 3: PreloadMetadataAsync("Pole/Jungle"_s); break;
			case 4: PreloadMetadataAsync("Pole/Psych"_s); break;
		}
	}

	Task<bool> Pole::OnActivatedAsync(const ActorActivationDetails& details)
	{
		uint8_t theme = details.Params[0];
		int16_t x = *(int16_t*)&details.Params[2];
		int16_t y = *(int16_t*)&details.Params[4];

		_pos.X += x;
		_pos.Y += y;
		_renderer.setLayer(_renderer.layer() - 20);

		SetState(ActorFlags::CanBeFrozen, false);
		CollisionFlags &= ~CollisionFlags::ApplyGravitation;

		bool isSolid = true;
		switch (theme) {
			default:
			case 0: co_await RequestMetadataAsync("Pole/Carrotus"_s); break;
			case 1: co_await RequestMetadataAsync("Pole/Diamondus"_s); break;
			case 2: co_await RequestMetadataAsync("Pole/DiamondusTree"_s); isSolid = false; break;
			case 3: co_await RequestMetadataAsync("Pole/Jungle"_s); break;
			case 4: co_await RequestMetadataAsync("Pole/Psych"_s); break;
		}

		if (isSolid) {
			CollisionFlags |= CollisionFlags::IsSolidObject;
		}

		SetAnimation("Pole"_s);

		co_return true;
	}

	void Pole::OnUpdate(float timeMult)
	{
		constexpr float FallMultiplier = 0.0036f;
		constexpr float Bounce = -0.2f;

		ActorBase::OnUpdate(timeMult);

		if (_fall != FallDirection::Left && _fall != FallDirection::Right) {
			return;
		}

		_fallTime += timeMult;

		if (_fall == FallDirection::Right) {
			if (_angleVel > 0 && IsPositionBlocked()) {
				if (_bouncesLeft > 0) {
					if (_bouncesLeft == BouncesMax) {
						_angleVelLast = _angleVel;
					}

					_bouncesLeft--;
					_angleVel = Bounce * _bouncesLeft * _angleVelLast;
				} else {
					_fall = FallDirection::Fallen;
					if (_fallTime < 10) {
						CollisionFlags &= ~CollisionFlags::IsSolidObject;
					}
				}
			} else {
				_angleVel += FallMultiplier * timeMult;
				_renderer.setRotation(_renderer.rotation() + _angleVel * timeMult);
				CollisionFlags |= CollisionFlags::IsDirty;
			}
		} else if (_fall == FallDirection::Left) {
			if (_angleVel < 0 && IsPositionBlocked()) {
				if (_bouncesLeft > 0) {
					if (_bouncesLeft == BouncesMax) {
						_angleVelLast = _angleVel;
					}

					_bouncesLeft--;
					_angleVel = Bounce * _bouncesLeft * _angleVelLast;
				} else {
					_fall = FallDirection::Fallen;
					if (_fallTime < 10.0f) {
						CollisionFlags &= ~CollisionFlags::IsSolidObject;
					}
				}
			} else {
				_angleVel -= FallMultiplier * timeMult;
				_renderer.setRotation(_renderer.rotation() + _angleVel * timeMult);
				CollisionFlags |= CollisionFlags::IsDirty;
			}
		}
	}

	bool Pole::OnHandleCollision(ActorBase* other)
	{
		if (auto shotBase = dynamic_cast<Weapons::ShotBase*>(other)) {
			Fall(shotBase->GetSpeed().X < 0.0f ? FallDirection::Left : FallDirection::Right);
			shotBase->DecreaseHealth(1, this);
			return true;
		}
		// TODO: TNT

		return false;
	}

	void Pole::Fall(FallDirection dir)
	{
		if (_fall != FallDirection::None) {
			return;
		}

		_fall = dir;
		SetState(ActorFlags::IsInvulnerable, true);
		CollisionFlags |= CollisionFlags::IsSolidObject;
	}

	bool Pole::IsPositionBlocked()
	{
		constexpr float Ratio1 = 0.96f;
		constexpr float Ratio2 = 0.8f;
		constexpr float Ratio3 = 0.6f;
		constexpr float Ratio4 = 0.3f;

		float angle = _renderer.rotation() - fPiOver2;
		float rx = std::cosf(angle);
		float ry = std::sinf(angle);
		float radius = _currentAnimation->Base->FrameDimensions.Y;

		if (_fallTime > 20) {
			// Check radius 1
			{
				float x = _pos.X + (rx * Ratio1 * radius);
				float y = _pos.Y + (ry * Ratio1 * radius);
				AABBf aabb = AABBf(x - 3, y - 3, x + 7, y + 7);
				if (!_levelHandler->IsPositionEmpty(this, aabb, true)) {
					return true;
				}
			}
			// Check radius 2
			{
				float x = _pos.X + (rx * Ratio2 * radius);
				float y = _pos.Y + (ry * Ratio2 * radius);
				AABBf aabb = AABBf(x - 3, y - 3, x + 7, y + 7);
				if (!_levelHandler->IsPositionEmpty(this, aabb, true)) {
					return true;
				}
			}
		}
		// Check radius 3
		{
			float x = _pos.X + (rx * Ratio3 * radius);
			float y = _pos.Y + (ry * Ratio3 * radius);
			AABBf aabb = AABBf(x - 3, y - 3, x + 7, y + 7);
			if (!_levelHandler->IsPositionEmpty(this, aabb, true)) {
				return true;
			}
		}
		// Check radius 4
		{
			float x = _pos.X + (rx * Ratio4 * radius);
			float y = _pos.Y + (ry * Ratio4 * radius);
			AABBf aabb = AABBf(x - 3, y - 3, x + 7, y + 7);
			if (!_levelHandler->IsPositionEmpty(this, aabb, true)) {
				return true;
			}
		}

		return false;
	}
}
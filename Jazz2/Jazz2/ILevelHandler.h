﻿#pragma once

#include "IStateHandler.h"
#include "ActorBase.h"
#include "LevelInitialization.h"

namespace Jazz2
{
	enum class PlayerActions {
		Left,
		Right,
		Up,
		Down,
		Fire,
		Jump,
		Run,
		SwitchWeapon,
		Menu,

		Count,

		None = -1
	};

	namespace Events
	{
		class EventSpawner;
		class EventMap;
	}

	namespace Tiles
	{
		class TileMap;
	}

	namespace Actors
	{
		class Player;
	}

	class ILevelHandler : public IStateHandler
	{
	public:
		static constexpr int MainPlaneZ = 500;
		static constexpr int PlayerZ = MainPlaneZ + 10;

		static constexpr float DefaultGravity = 0.3f;

		ILevelHandler() : Gravity(DefaultGravity) { }

		virtual Events::EventSpawner* EventSpawner() = 0;
		virtual Events::EventMap* EventMap() = 0;
		virtual Tiles::TileMap* TileMap() = 0;

		virtual GameDifficulty Difficulty() const = 0;
		virtual bool ReduxMode() const = 0;
		virtual Recti LevelBounds() const = 0;
		virtual float WaterLevel() const = 0;

		virtual const Death::SmallVectorImpl<Actors::Player*>& GetPlayers() const = 0;

		virtual void AddActor(const std::shared_ptr<ActorBase>& actor) = 0;

		virtual void WarpCameraToTarget(const std::shared_ptr<ActorBase>& actor) = 0;
		virtual bool IsPositionEmpty(ActorBase* self, const AABBf& aabb, bool downwards, __out ActorBase** collider) = 0;

		bool IsPositionEmpty(ActorBase* self, const AABBf& aabb, bool downwards)
		{
			ActorBase* collider;
			return IsPositionEmpty(self, aabb, downwards, &collider);
		}

		virtual void GetCollidingPlayers(const AABBf& aabb, const std::function<bool(ActorBase*)> callback) = 0;

		virtual void HandleGameOver() = 0;
		virtual bool HandlePlayerDied(const std::shared_ptr<ActorBase>& player) = 0;

		virtual bool PlayerActionPressed(int index, PlayerActions action, bool includeGamepads = true) = 0;
		virtual bool PlayerActionPressed(int index, PlayerActions action, bool includeGamepads, __out bool& isGamepad) = 0;
		virtual bool PlayerActionHit(int index, PlayerActions action, bool includeGamepads = true) = 0;
		virtual bool PlayerActionHit(int index, PlayerActions action, bool includeGamepads, __out bool& isGamepad) = 0;
		virtual float PlayerHorizontalMovement(int index) = 0;
		virtual float PlayerVerticalMovement(int index) = 0;
		virtual void PlayerFreezeMovement(int index, bool enable) = 0;

		float Gravity;
	};
}
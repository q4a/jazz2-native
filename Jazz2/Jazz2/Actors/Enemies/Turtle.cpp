﻿#include "Turtle.h"
#include "../../LevelInitialization.h"
#include "../../ILevelHandler.h"
#include "../../Tiles/TileMap.h"

#include "../../../nCine/Base/Random.h"

namespace Jazz2::Actors::Enemies
{
	Turtle::Turtle()
		:
		_isTurning(false),
		_isWithdrawn(false),
		_isAttacking(false)
	{
	}

	void Turtle::Preload(const ActorActivationDetails& details)
	{
		uint8_t theme = details.Params[0];
		switch (theme) {
			case 0:
			default:
				PreloadMetadataAsync("Enemy/Turtle");
				PreloadMetadataAsync("Enemy/TurtleShell");
				break;

			case 1: // Xmas
				PreloadMetadataAsync("Enemy/TurtleXmas");
				PreloadMetadataAsync("Enemy/TurtleShellXmas");
				break;
		}
	}

	Task<bool> Turtle::OnActivatedAsync(const ActorActivationDetails& details)
	{
		SetHealthByDifficulty(1);
		_scoreValue = 100;

		_theme = details.Params[0];
		switch (_theme) {
			case 0:
			default:
				co_await RequestMetadataAsync("Enemy/Turtle");
				break;

			case 1: // Xmas
				co_await RequestMetadataAsync("Enemy/TurtleXmas");
				break;
		}

		SetAnimation(AnimState::Walk);

		SetFacingLeft(random().NextBool());
		_speed.X = (IsFacingLeft() ? -1 : 1) * DefaultSpeed;

		co_return true;
	}

	void Turtle::OnUpdate(float timeMult)
	{
		EnemyBase::OnUpdate(timeMult);

		if (_frozenTimeLeft > 0) {
			return;
		}

		if (GetState(ActorFlags::CanJump)) {
			if (std::abs(_speed.X) > std::numeric_limits<float>::epsilon() && !CanMoveToPosition(_speed.X * 4, 0)) {
				SetTransition(AnimState::TransitionWithdraw, false, [this]() {
					HandleTurn(true);
				});
				_isTurning = true;
				_canHurtPlayer = false;
				_speed.X = 0;
				//PlaySound("Withdraw", 0.4f);
			}
		}

		if (!_isTurning && !_isWithdrawn && !_isAttacking) {
			AABBf aabb = AABBInner + Vector2f(_speed.X * 32, 0);
			if (_levelHandler->TileMap()->IsTileEmpty(aabb, true)) {
				_levelHandler->GetCollidingPlayers(aabb + Vector2f(_speed.X * 32, 0), [this](ActorBase* player) -> bool {
					if (!player->IsInvulnerable()) {
						Attack();
						return false;
					}
					return true;
				});
			}
		}
	}

	void Turtle::OnUpdateHitbox()
	{
		UpdateHitbox(24, 24);
	}

	bool Turtle::OnPerish(ActorBase* collider)
	{
		/*if (renderer.AnimPaused) {
			// Animation should be paused only if enemy is frozen
			CreateDeathDebris(collider);
			levelHandler.PlayCommonSound("Splat", Transform.Pos);

			TryGenerateRandomDrop();
		} else {
			TurtleShell shell = new TurtleShell(speedX * 1.1f, 1.1f);
			shell.OnActivated(new ActorActivationDetails {
				LevelHandler = levelHandler,
				Pos = Transform.Pos,
				Params = new[] { theme }
			});
			levelHandler.AddActor(shell);

			Explosion.Create(levelHandler, Transform.Pos, Explosion.SmokeGray);
		}*/

		return EnemyBase::OnPerish(collider);
	}

	void Turtle::HandleTurn(bool isFirstPhase)
	{
		if (_isTurning) {
			if (isFirstPhase) {
				SetFacingLeft(!IsFacingLeft());
				SetTransition(AnimState::TransitionWithdrawEnd, false, [this]() {
				   HandleTurn(false);
				});
				//PlaySound("WithdrawEnd", 0.4f);
				_isWithdrawn = true;
			} else {
				_canHurtPlayer = true;
				_isWithdrawn = false;
				_isTurning = false;
				_speed.X = (IsFacingLeft() ? -1 : 1) * DefaultSpeed;
			}
		}
	}

	void Turtle::Attack()
	{
		_speed.X = 0;
		_isAttacking = true;
		//PlaySound("Attack");

		SetTransition(AnimState::TransitionAttack, false, [this]() {
			_speed.X = (IsFacingLeft() ? -1 : 1) * DefaultSpeed;
			_isAttacking = false;

			// ToDo: Bad timing
			//PlaySound("Attack2");
		});
	}
}
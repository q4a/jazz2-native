﻿#include "Checkpoint.h"
#include "../../ILevelHandler.h"
#include "../../Events/EventMap.h"
#include "../Player.h"

namespace Jazz2::Actors::Environment
{
	Checkpoint::Checkpoint()
		:
		_activated(false)
	{
	}

	void Checkpoint::Preload(const ActorActivationDetails& details)
	{
		uint8_t theme = details.Params[0];
		switch (theme) {
			case 0:
			default:
				PreloadMetadataAsync("Object/Checkpoint"_s);
				break;
			case 1: // Xmas
				PreloadMetadataAsync("Object/CheckpointXmas"_s);
				break;
		}
	}

	Task<bool> Checkpoint::OnActivatedAsync(const ActorActivationDetails& details)
	{
		SetState(ActorFlags::CanBeFrozen, false);

		// TODO: change these types to uint8_t
		_theme = details.Params[0];
		_activated = (details.Params[2] != 0);

		switch (_theme) {
			case 0:
			default:
				co_await RequestMetadataAsync("Object/Checkpoint"_s);
				break;

			case 1: // Xmas
				co_await RequestMetadataAsync("Object/CheckpointXmas"_s);
				break;
		}

		SetAnimation(_activated ? "Opened"_s : "Closed"_s);

		co_return true;
	}

	void Checkpoint::OnUpdateHitbox()
	{
		UpdateHitbox(20, 20);
	}

	bool Checkpoint::OnHandleCollision(ActorBase* other)
	{
		if (_activated) {
			return true;
		}

		if (auto player = dynamic_cast<Player*>(other)) {
			_activated = true;

			// Set this checkpoint for all players
			// TODO
			/*for (auto& p : _levelHandler.Players) {
				p.SetCheckpoint(_pos);
			}*/

			SetAnimation("Opened"_s);
			SetTransition(AnimState::TransitionActivate, false);

			PlaySfx("TransitionActivate"_s);

			// Deactivate event in map
			// TODO: change these types to uint8_t
			uint8_t playerParams[16] = { _theme, 0, 1 };
			_levelHandler->EventMap()->StoreTileEvent(_originTile.X, _originTile.Y, EventType::Checkpoint, ActorFlags::None, playerParams);

			// TODO
			/*if (_levelHandler->Difficulty() != GameDifficulty::Multiplayer) {
				_levelHandler->EventMap()->CreateCheckpointForRollback();
			}*/
			return true;
		}

		return false;
	}
}
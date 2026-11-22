// Expansion Quest tweaks: approved radio items open the quest menu based on freq and time

#ifdef EXPANSIONMODQUESTS

	// Helper class for item, frequency and time lookups + reward drops 
	class RadioQuestBoardHelper
	{
		static const int RADIO_WILDCARD_CONTEXT = -1000000;
		static const int REWARD_DROP_ATTEMPTS_PER_RADIUS = 10;
		static const int REWARD_DROP_RADIUS_STEPS = 3;
		static const float REWARD_DROP_RADIUS_STEP = 35.0;

		#ifdef SERVER
		// Server-side test if approved radio item is tuned to freq thats transmitting at in-game time
		static bool FindActiveChannel(string radioType, float tunedFreq, out GwExpansionQuestRadioChannel matchedChannel, out int matchedIndex)
		{
			array<ref GwExpansionQuestRadioChannel> channels;
			GwExpansionQuestRadioChannel channel;
			int channelIndex;
			int year, month, day, hour, minute;

			if (!GwExpansionTweaksUtils.IsStringInListSub(radioType, GetGwTweaksConfig().ExpansionQuestsRadio_AllowedRadioTypes))
				return false;

			channels = GetGwTweaksConfig().ExpansionQuestsRadio_Channels;
			if (!channels || !g_Game.GetWorld())
				return false;

			g_Game.GetWorld().GetDate(year, month, day, hour, minute);

			for (channelIndex = 0; channelIndex < channels.Count(); channelIndex++)
			{
				channel = channels[channelIndex];
				if (!channel || Math.AbsFloat(tunedFreq - channel.Frequency) > GwExpansionTweaksConfig.QUEST_RADIO_FREQUENCY_EPSILON || !channel.IsActiveAtHour(hour))
					continue;

				matchedChannel = channel;
				matchedIndex = channelIndex;
				return true;
			}

			return false;
		}

		// Asks the quest module to open the menu for player after checking FindActiveChannel above
		static void TryOpenQuestMenu(EntityAI radio, float tunedFreq, PlayerBase player)
		{
			string typeName;
			PlayerIdentity identity;
			ExpansionQuestModule questModule;
			GwExpansionQuestRadioChannel channel;
			int channelIndex;

			if (!radio || !player || !GetGwTweaksConfig().ExpansionQuestsRadio_Enabled)
				return;

			typeName = radio.GetType();

			if (!FindActiveChannel(typeName, tunedFreq, channel, channelIndex))
			{
				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("RadioQuests|TryOpenQuestMenu", "REJECTED " + typeName + " @ " + tunedFreq + " - not an allowed radio, or no channel live at the current in-game hour");
				return;
			}

			identity = player.GetIdentity();
			questModule = ExpansionQuestModule.GetModuleInstance();
			if (!identity || !questModule)
			{
				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("RadioQuests|TryOpenQuestMenu", "REJECTED " + typeName + " - player identity or ExpansionQuestModule missing");
				return;
			}

			if (!questModule.GwET_OpenRadioQuestMenu(radio, identity, channel))
			{
				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("RadioQuests|TryOpenQuestMenu", "REJECTED " + typeName + " - could not open quest channel " + channelIndex);
				return;
			}

			if (GwExpansionTweaksUtils.IsLogging())
				GwExpansionTweaksUtils.GwLog("RadioQuests|TryOpenQuestMenu", "OPENED channel " + channelIndex + " for " + identity.GetName() + " via " + typeName + " @ " + tunedFreq + " hours " + channel.HourActiveStart + "-" + channel.HourActiveEnd + " giver " + channel.QuestGiverID);
		}
		#endif

		// True when context ID is a configured radio channel giver (or wildcard)
		static bool IsRadioMenuContext(int questNPCID)
		{
			array<ref GwExpansionQuestRadioChannel> channels;
			GwExpansionQuestRadioChannel channel;
			int channelIndex;
			bool wantWildcard = questNPCID == RADIO_WILDCARD_CONTEXT;

			if (!GetGwTweaksConfig().ExpansionQuestsRadio_Enabled || (!wantWildcard && questNPCID < 1))
				return false;

			channels = GetGwTweaksConfig().ExpansionQuestsRadio_Channels;
			if (!channels)
				return false;

			for (channelIndex = 0; channelIndex < channels.Count(); channelIndex++)
			{
				channel = channels[channelIndex];
				if (!channel)
					continue;

				if (wantWildcard && channel.QuestGiverID < 1)
					return true;

				if (!wantWildcard && channel.QuestGiverID == questNPCID)
					return true;
			}

			return false;
		}

		// Server-side test if quest is handed out configured radio channel givers
		#ifdef SERVER
		static bool IsRadioGiverQuest(ExpansionQuestConfig config)
		{
			array<int> giverIDs;
			array<ref GwExpansionQuestRadioChannel> channels;
			GwExpansionQuestRadioChannel channel;
			int channelIndex;

			if (!config)
				return false;

			giverIDs = config.GetQuestGiverIDs();
			channels = GetGwTweaksConfig().ExpansionQuestsRadio_Channels;
			if (!giverIDs || !channels)
				return false;

			for (channelIndex = 0; channelIndex < channels.Count(); channelIndex++)
			{
				channel = channels[channelIndex];
				if (channel && channel.QuestGiverID > 0 && giverIDs.Find(channel.QuestGiverID) > -1)
					return true;
			}

			return false;
		}

		// Find random drop position on open dry land for reward drop on quest turn-in
		static bool FindRewardDropPosition(PlayerBase player, out vector dropPosition)
		{
			vector playerPosition;
			vector candidate;
			vector hitPos, hitNormal;
			float radius;
			float heading;
			int hitComponent;
			int radiusStep;
			int attempt;

			if (!player)
				return false;

			playerPosition = player.GetPosition();

			for (radiusStep = 1; radiusStep <= REWARD_DROP_RADIUS_STEPS; radiusStep++)
			{
				radius = REWARD_DROP_RADIUS_STEP * radiusStep;
				for (attempt = 0; attempt < REWARD_DROP_ATTEMPTS_PER_RADIUS; attempt++)
				{
					heading = Math.RandomFloat(0.0, Math.PI2);
					candidate = playerPosition + Vector(Math.Cos(heading), 0.0, Math.Sin(heading)) * radius;
					candidate[1] = g_Game.SurfaceY(candidate[0], candidate[2]);

					if (ExpansionStatic.SurfaceIsWater(candidate[0], candidate[2]))
						continue;

					if (DayZPhysics.RaycastRV(candidate + "0 0.5 0", candidate + "0 100 0", hitPos, hitNormal, hitComponent, null, null, null, false, false, ObjIntersectFire))
						continue;

					dropPosition = candidate;
					return true;
				}
			}

			return false;
		}
		#endif
	}

	// Handheld transmitters override (e.g., personal+base radios)
	#ifdef SERVER
	modded class TransmitterBase
	{
		override void SetNextFrequency(PlayerBase player = NULL)
		{
			super.SetNextFrequency(player);
			RadioQuestBoardHelper.TryOpenQuestMenu(this, GetTunedFrequency(), player);
		}

		override void OnSwitchOn()
		{
			super.OnSwitchOn();

			if (!GetCompEM().CanWork())
				return;

			PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
			if (player)
				RadioQuestBoardHelper.TryOpenQuestMenu(this, GetTunedFrequency(), player);
		}
	}
	#endif

	// Static radio panel override
	modded class Land_Radio_PanelBig
	{
		static const string SND_RADIO_TUNE = "tuneRadio_SoundSet";
		static const string SND_PANEL_STATIC = "baseradio_staticnoise_SoundSet";
		protected ref EffectSound m_PanelRadioSound;

		#ifdef SERVER
		override void SetNextFrequency(PlayerBase player = null)
		{
			super.SetNextFrequency(player);
			RadioQuestBoardHelper.TryOpenQuestMenu(this, GetTunedFrequency(), player);
		}
		#endif

		// Use baseradio sound because vanilla PAS sound sux ass
		override void OnSwitchOn()
		{
			super.OnSwitchOn();

			if (!GetGwTweaksConfig().ExpansionQuestsRadio_Enabled || !GetCompEM().CanWork())
				return;

			GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_TUNE, this);
			m_PanelRadioSound = GwExpansionTweaksUtils.PlayAutodestroySound(SND_PANEL_STATIC, this, 0.15, 0.15, true);
		}

		override void OnWorkStop()
		{
			super.OnWorkStop();

			if (!m_PanelRadioSound)
				return;

			m_PanelRadioSound.SoundStop();
			m_PanelRadioSound = null;
			GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_TUNE, this);
		}
	}

	#ifndef SERVER
	modded class ActionTuneFrequencyOnGround
	{
		// Play client-side click on land_radio panels because PAS sound sux
		override void OnExecuteClient(ActionData action_data)
		{
			super.OnExecuteClient(action_data);

			Land_Radio_PanelBig panel = Land_Radio_PanelBig.Cast(action_data.m_Target.GetObject());
			if (panel && GetGwTweaksConfig().ExpansionQuestsRadio_Enabled)
				GwExpansionTweaksUtils.PlayAutodestroySound(Land_Radio_PanelBig.SND_RADIO_TUNE, panel);
		}
	}
	#endif

	#ifdef SERVER
	modded class ActionTurnOnTransmitterOnGround
	{
		// Server-side check at power on for ground transmitters like land_radio panels
		override void OnExecuteServer(ActionData action_data)
		{
			super.OnExecuteServer(action_data);

			StaticTransmitter radio = StaticTransmitter.Cast(action_data.m_Target.GetObject());
			if (!radio || !action_data.m_Player || !radio.GetCompEM().CanWork())
				return;

			RadioQuestBoardHelper.TryOpenQuestMenu(radio, radio.GetTunedFrequency(), action_data.m_Player);
		}
	}
	#endif

	modded class ExpansionQuestModule
	{
		// Server-side call of the Expansion quest RPC for player if all conditions are met
		#ifdef SERVER
		bool GwET_OpenRadioQuestMenu(Object radio, PlayerIdentity identity, GwExpansionQuestRadioChannel channel)
		{
			ExpansionScriptRPC rpc;
			int contextID;

			if (!radio || !identity || !channel)
				return false;

			contextID = RadioQuestBoardHelper.RADIO_WILDCARD_CONTEXT;
			if (channel.QuestGiverID > 0)
				contextID = channel.QuestGiverID;

			rpc = Expansion_CreateRPC("RPC_RequestOpenQuestMenu");
			if (!rpc)
				return false;

			rpc.Write(contextID);
			rpc.Write(GetGwTweaksConfig().ExpansionQuestsRadio_NoQuestsText);
			rpc.Write(-1);
			rpc.Write(CF_Date.Now(true).GetTimestamp());
			rpc.Expansion_Send(radio, true, identity);
			return true;
		}
		#endif

		// Client-side contexts for configured radio questgiver ID
		override bool QuestDisplayConditions(ExpansionQuestConfig config, PlayerBase player, ExpansionQuestPersistentData playerQuestData = null, int questNPCID = -1, bool displayQuestsWithCooldown = false, bool skipPreQuestCheck = false)
		{
			array<int> giverIDs;
			array<int> turnInIDs;
			bool isGiver;
			bool isTurnIn;
			int expansionNPCID;

			if (!RadioQuestBoardHelper.IsRadioMenuContext(questNPCID))
				return super.QuestDisplayConditions(config, player, playerQuestData, questNPCID, displayQuestsWithCooldown, skipPreQuestCheck);

			if (!config)
				return false;

			expansionNPCID = -1;
			if (questNPCID > 0)
			{
				giverIDs = config.GetQuestGiverIDs();
				turnInIDs = config.GetQuestTurnInIDs();
				isGiver = giverIDs && giverIDs.Find(questNPCID) > -1;
				isTurnIn = turnInIDs && turnInIDs.Find(questNPCID) > -1;
				if (!isGiver && !isTurnIn)
					return false;

				if (!turnInIDs || turnInIDs.Count() == 0 || isTurnIn)
					expansionNPCID = questNPCID;
			}

			return super.QuestDisplayConditions(config, player, playerQuestData, expansionNPCID, displayQuestsWithCooldown, skipPreQuestCheck);
		}
	}

	// Server-side reward drop implementation
	#ifdef SERVER
	modded class ExpansionQuest
	{
		private string m_GwET_RewardDropPlayerUID;
		private vector m_GwET_RewardDropPosition;
		private bool m_GwET_RewardDropChecked;
		private bool m_GwET_RewardDropValid;
		private bool m_GwET_RewardSignalsSpawned;

		// Create flare and smoke over the reward site and notify the player
		private void GwET_SpawnRewardSignals(PlayerBase player)
		{
			if (m_GwET_RewardSignalsSpawned)
				return;

			m_GwET_RewardSignalsSpawned = true;

			int flags = ECE_CREATEPHYSICS | ECE_AIRBORNE | ECE_NOLIFETIME;
			Roadflare roadFlare = Roadflare.Cast(g_Game.CreateObjectEx("Roadflare", m_GwET_RewardDropPosition + "1 100 0", flags));
			if (roadFlare)
			{
				GwET_DropSignalFromSky(roadFlare);
				if (roadFlare.GetCompEM())
					roadFlare.GetCompEM().SwitchOn();
			}

			M18SmokeGrenade_Red smokeGrenade = M18SmokeGrenade_Red.Cast(g_Game.CreateObjectEx("M18SmokeGrenade_Red", m_GwET_RewardDropPosition + "-1 90 0", flags));
			if (smokeGrenade)
			{
				GwET_DropSignalFromSky(smokeGrenade);
				smokeGrenade.Unpin();
			}

			string rewardText = GetGwTweaksConfig().ExpansionQuestsRadio_RewardText;
			if (GetGwTweaksConfig().ExpansionRadioNotifications_Enabled && !GwExpansionTweaksUtils.Normalize(GetExpansionSettings().GetQuest().QuestTurnInTitle + rewardText).Contains("quest"))
				rewardText = "QUEST: " + rewardText;

			ExpansionNotification(new StringLocaliser(GetExpansionSettings().GetQuest().QuestTurnInTitle), new StringLocaliser(rewardText), ExpansionIcons.GetPath("Heart"), COLOR_EXPANSION_NOTIFICATION_SUCCESS, 7, ExpansionNotificationType.TOAST).Create(player.GetIdentity());

			if (GwExpansionTweaksUtils.IsLogging())
				GwExpansionTweaksUtils.GwLog("RadioQuests|SpawnRewardSignals", "Flare " + Object.GetDebugName(roadFlare) + " and smoke " + Object.GetDebugName(smokeGrenade) + " dropped at " + m_GwET_RewardDropPosition.ToString() + " for " + player.GetIdentity().GetName());
		}

		// Drop stuff from above - this was a pain to do in-line, and physics capped to 13 secs so they dont roll away
		private void GwET_DropSignalFromSky(ItemBase item)
		{
			if (!item)
				return;

			if (dBodyIsSet(item))
				dBodyDestroy(item);

			item.CreateDynamicPhysics(PhxInteractionLayers.DYNAMICITEM);
			item.EnableDynamicCCD(true);
			dBodyEnableGravity(item, true);
			dBodySetDamping(item, 0.65, 0.8);
			SetVelocity(item, "0 -2 0");
			dBodyActive(item, ActiveState.ALWAYS_ACTIVE);
			item.SetDynamicPhysicsLifeTime(13);
			item.SetTakeable(false);
		}

		override void SendNotification(StringLocaliser title, StringLocaliser text, string icon, int color)
		{
			if (GetGwTweaksConfig().ExpansionRadioNotifications_Enabled && !GwExpansionTweaksUtils.Normalize(title.GetText() + text.GetText()).Contains("quest"))
				text.Set(-1, "QUEST: " + text.GetText());

			super.SendNotification(title, text, icon, color);
		}

		// Override Expansion rewards on quest turn-in if FlareRewards enabled in config
		protected override void SpawnReward(ExpansionQuestRewardConfig reward, PlayerBase player, EntityAI parent, vector pos, vector ori)
		{
			// Stick with vanilla rewards if config disabled, or player in a vehicle or swimming
			if (!reward || !player || !player.GetIdentity() || !m_Config || !GetGwTweaksConfig().ExpansionQuestsRadio_Enabled || !GetGwTweaksConfig().ExpansionQuestsRadio_FlareRewards || !RadioQuestBoardHelper.IsRadioGiverQuest(m_Config) || player.IsInVehicle() || player.IsSwimming())
			{
				super.SpawnReward(reward, player, parent, pos, ori);
				return;
			}

			// Find a spot to drop reward if we dont have one yet
			if (!m_GwET_RewardDropChecked || m_GwET_RewardDropPlayerUID != player.GetIdentity().GetId())
			{
				m_GwET_RewardDropPlayerUID = player.GetIdentity().GetId();
				m_GwET_RewardDropChecked = true;
				m_GwET_RewardDropValid = RadioQuestBoardHelper.FindRewardDropPosition(player, m_GwET_RewardDropPosition);
				m_GwET_RewardSignalsSpawned = false;

				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("RadioQuests|SpawnReward", "Drop site for " + player.GetIdentity().GetName() + " valid=" + m_GwET_RewardDropValid + " at " + m_GwET_RewardDropPosition.ToString() + " distance " + vector.Distance(player.GetPosition(), m_GwET_RewardDropPosition) + "m");
			}

			// Also stick with vanilla rewards if no valid drop site? Edge case probably..
			if (!m_GwET_RewardDropValid)
			{
				super.SpawnReward(reward, player, parent, pos, ori);
				return;
			}

			bool spawnedAny = false;
			int remainingAmount = reward.GetAmount();
			while (remainingAmount > 0)
			{
				int remainingAmountBefore = remainingAmount;
				Object rewardObject = Spawn(reward, player, parent, m_GwET_RewardDropPosition, ori, remainingAmount);
				if (!rewardObject)
				{
					Error("Error: Couldn't spawn " + reward.GetClassName());
					break;
				}

				spawnedAny = true;

				// Vehicles already spawn at the drop position; everything else has to be moved out of the player's hands
				ItemBase rewardItem = ItemBase.Cast(rewardObject);
				if (!reward.IsVehicle() && rewardItem)
				{
					EntityAI inventoryParent = rewardItem.GetHierarchyParent();
					if (inventoryParent && inventoryParent.GetInventory())
						inventoryParent.GetInventory().DropEntity(InventoryMode.SERVER, inventoryParent, rewardItem);

					rewardItem.SetPosition(m_GwET_RewardDropPosition);
					rewardItem.PlaceOnSurface();
					dBodyActive(rewardItem, ActiveState.ACTIVE);
				}

				if (remainingAmount == remainingAmountBefore)
				{
					Error("Error: Spawning " + reward.GetClassName() + " did not affect remaining amount!");
					break;
				}
			}

			if (spawnedAny)
				GwET_SpawnRewardSignals(player);
		}
	}
	#endif

#endif

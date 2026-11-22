// Expansion AI tweaks: random size, beard/bloody-hands, cowardice and MadBomber fix

#ifdef EXPANSIONMODAI

	modded class eAIBase
	{
		// Consts for cowardice and visuals
		static const float GWET_COWARDICE_FLEE_DISTANCE = 1000.0;
		static const float GWET_COWARDICE_ARRIVE_DIST_SQ = 2500.0;
		static const float GWET_COWARDICE_PROGRESS_DIST_SQ = 640000.0;
		static const float GWET_COWARDICE_RETALIATE_DISTANCE = 5.0;
		static const float GWET_COWARDICE_RETALIATE_REMAINING_MS = 120000.0;
		static const float GWET_COWARDICE_MIN_DURATION_MS = 180000.0;
		static const float GWET_COWARDICE_MAX_DURATION_MS = 360000.0;
		static const float GWET_COWARDICE_GREETING_CHANCE = 0.069;
		static const int GWET_COWARDICE_TICK_MS = 10000;
		static const int GWET_COWARDICE_UNCON_RETRY_MS = 15000;
		static const int GWET_COWARDICE_MAX_ACTIVE = 30;
		static const int GWET_VISUALS_DELAY_MS = 3000;
		static const float GWET_SCALE_PROXIMITY_RANGE = 500.0;
		static const int GWET_SCALE_PROXIMITY_MS = 10000;
 
		// Vars
		private static int s_GwET_ActiveCowards;
		private vector m_GwET_FleePoint;
		private bool m_GwET_CowardiceActive;
		private bool m_GwET_CowardiceRetryPending;
		private float m_GwET_CowardiceUntil;
		private float m_GwET_CowardiceLastEvaluated;
		private float m_GwET_Scale = 1.0;
		private bool m_GwET_PostFrameEnabled;
		private ref array<ref eAITargetInformation> m_GwET_TargetCleanup = new array<ref eAITargetInformation>;
		private ref array<EntityAI> m_GwET_NearbyThreats = new array<EntityAI>;
		#ifdef MadBomberAI
			private bool m_MadBomberAI_Locked;
		#endif

		// Get a bates-granger weighted rand for AI scale (thanks google)
		float GwET_GetWeightedScale(float min, float max)
		{
			float u1 = Math.RandomFloatInclusive(min, max);
			float u2 = Math.RandomFloatInclusive(min, max);

			return (u1 + u2) / 2.0;
		}

		// Only do post frame if players are near enough to notice (500m)
		void GwET_CheckPlayerProximity(float range)
		{
			if (m_GwET_Scale == 1.0)
			{
				if (m_GwET_PostFrameEnabled)
				{
					ClearEventMask(EntityEvent.POSTFRAME);
					m_GwET_PostFrameEnabled = false;
				}
				g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(GwET_CheckPlayerProximity);
				return;
			}

			bool playerNearby = GwExpansionTweaksUtils.IsPlayerWithinRange(GetPosition(), range);

			if (playerNearby && !m_GwET_PostFrameEnabled)
			{
				SetEventMask(GetEventMask() | EntityEvent.POSTFRAME);
				m_GwET_PostFrameEnabled = true;
				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("eAIBase|GwET_CheckPlayerProximity", GetType() + " postframe on at " + GetPosition().ToString());
			}
			else if (!playerNearby && m_GwET_PostFrameEnabled)
			{
				ClearEventMask(EntityEvent.POSTFRAME);
				m_GwET_PostFrameEnabled = false;
				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("eAIBase|GwET_CheckPlayerProximity", GetType() + " postframe off at " + GetPosition().ToString());
			}
		}

		// Register AI scaling stuff
		override void Init()
		{
			super.Init();

			RegisterNetSyncVariableFloat("m_GwET_Scale", 0.0, 2.0, 2);
		}

		protected override void EOnInit(IEntity other, int extra)
		{
			super.EOnInit(other, extra);

			#ifdef SERVER
				if (IsAlive() && GetGwTweaksConfig().ExpansionTweaksForAI_Enabled && !IsInherited(eAINPCBase))
					g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GwET_ApplyVisuals, GWET_VISUALS_DELAY_MS, false);

				#ifdef MadBomberAI
					if (IsAlive())
						g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MadBomberAI_CheckHands, GWET_VISUALS_DELAY_MS, false);
				#endif
			#endif

			if (!GetGwTweaksConfig().ExpansionTweaksForAI_Enabled || IsInherited(eAINPCBase))
				return;

			float scaleMin = GetGwTweaksConfig().ExpansionTweaksForAI_FemaleScaleMin;
			float scaleMax = GetGwTweaksConfig().ExpansionTweaksForAI_FemaleScaleMax;
			if (IsMale())
			{
				scaleMin = GetGwTweaksConfig().ExpansionTweaksForAI_MaleScaleMin;
				scaleMax = GetGwTweaksConfig().ExpansionTweaksForAI_MaleScaleMax;
			}

			if (scaleMin <= 0 || scaleMax <= 0)
				return;

			SetEventMask(GetEventMask() | EntityEvent.POSTFRAME);
			m_GwET_PostFrameEnabled = true;

			#ifdef SERVER
				float newScale = GwET_GetWeightedScale(scaleMin, scaleMax);

				// Keep values within 5% of 1.0 as just 1.0, so we can skip EOnPostFrame for those
				if (Math.AbsFloat(newScale - 1.0) > 0.05)
				{
					m_GwET_Scale = newScale;
					SetScale(m_GwET_Scale);
				}

				SetSynchDirty();

				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("eAIBase|EOnInit", GetType() + " scale " + m_GwET_Scale + " at " + GetPosition().ToString());

				if (m_GwET_Scale == 1.0)
				{
					ClearEventMask(EntityEvent.POSTFRAME);
					m_GwET_PostFrameEnabled = false;
					return;
				}
			#endif

			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GwET_CheckPlayerProximity, GWET_SCALE_PROXIMITY_MS, true, GWET_SCALE_PROXIMITY_RANGE);
		}

		// Apply the synced scale if its <> 1.0
		protected override void EOnPostFrame(IEntity other, int extra)
		{
			super.EOnPostFrame(other, extra);

			if (m_GwET_Scale == 1.0)
				return;

			SetScale(m_GwET_Scale);
		}

		// Make sure we clean up cowards on AI despawn/delete
		override void EEDelete(EntityAI parent)
		{
			if (g_Game)
			{
				ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
				queue.Remove(GwET_CheckPlayerProximity);
				#ifdef SERVER
					queue.Remove(GwET_ApplyVisuals);
					queue.Remove(GwET_RetryCowardice);
					queue.Remove(GwET_CowardiceTick);
					#ifdef MadBomberAI
						queue.Remove(MadBomberAI_CheckHands);
					#endif
				#endif
			}

			#ifdef SERVER
				if (m_GwET_CowardiceActive && s_GwET_ActiveCowards > 0)
					s_GwET_ActiveCowards--;
			#endif

			super.EEDelete(parent);
		}

	// All the new modded funcs to manage AI happen on the server
	#ifdef SERVER

		// Cowardice trigger - got hurt real bad
		override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
		{
			super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

			if (m_GwET_CowardiceActive || !m_InjuryHandler || m_InjuryHandler.GetInjuryAnimValue() < InjuryAnimValues.LVL2 || !eAI_IsLowVitals())
				return;

			GwET_EvaluateCowardice();
		}

		// Cowardice trigger - got knocked out
		override void OnUnconsciousStop(int pCurrentCommandID)
		{
			super.OnUnconsciousStop(pCurrentCommandID);

			GwET_EvaluateCowardice();
		}

		// Roll the dice to see if cowardice gets triggered. Single gate for every trigger path.
		void GwET_EvaluateCowardice()
		{
			if (m_GwET_CowardiceActive || !IsAlive() || IsInVehicle() || IsSwimming() || IsClimbing())
				return;

			if (!GetGwTweaksConfig().ExpansionTweaksForAI_Enabled)
				return;

			if (g_Game.GetTime() - m_GwET_CowardiceLastEvaluated < GWET_COWARDICE_TICK_MS)
				return;

			m_GwET_CowardiceLastEvaluated = g_Game.GetTime();

			if (!GwExpansionTweaksUtils.IsPlayerWithinRange(GetPosition(), GWET_SCALE_PROXIMITY_RANGE))
				return;

			float chance = GetGwTweaksConfig().ExpansionTweaksForAI_ChanceForCowardice;
			if (chance > 0 && Math.RandomFloatInclusive(0.0, 1.0) <= chance)
				GwET_StartCowardice();
		}

		// Make the AI a chicken!
		void GwET_StartCowardice()
		{
			// Early exit if cowardice cant start
			if (!IsAlive() || m_GwET_CowardiceActive || s_GwET_ActiveCowards >= GWET_COWARDICE_MAX_ACTIVE)
				return;

			// Bombers dont let cowardice block their path to paradise
			#ifdef MadBomberAI
				if (m_MadBomberAI_Locked)
					return;
			#endif

			// Retry cowardice later if uncon
			if (IsUnconscious())
			{
				if (!m_GwET_CowardiceRetryPending)
				{
					m_GwET_CowardiceRetryPending = true;
					g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GwET_RetryCowardice, GWET_COWARDICE_UNCON_RETRY_MS, false);
				}
				return;
			}

			if (m_GwET_CowardiceRetryPending)
			{
				g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(GwET_RetryCowardice);
				m_GwET_CowardiceRetryPending = false;
			}

			// Split off into own group but maintain faction, otherwise other AI in old group get extremely trigger happy
			eAIGroup group = GetGroup();
			if (!group || !group.GetFaction())
				return;

			if (group.Count() > 1 || group.GetLeader() != this)
			{
				group = eAIGroup.CreateGroup(group.GetFaction());
				if (!group)
					return;

				SetGroup(group);
			}

			// Pick a random place to flee to, and a random time to flee for - and increment coward counter
			m_GwET_FleePoint = GwET_PickFleePoint(GetPosition());
			m_GwET_CowardiceUntil = g_Game.GetTime() + Math.RandomFloatInclusive(GWET_COWARDICE_MIN_DURATION_MS, GWET_COWARDICE_MAX_DURATION_MS);
			m_GwET_CowardiceActive = true;
			s_GwET_ActiveCowards++;

			// Set the flee target as their only waypoint, no looting
			eAI_ResetPathfinding();
			eAI_SetLootingBehavior(eAILootingBehavior.NONE);
			group.SetIsInCombat(false);
			group.ClearWaypoints();
			group.AddWaypoint(m_GwET_FleePoint);

			GwET_SuppressThreatState();

			// Small chance to emote when chickening out
			if (Math.RandomFloatInclusive(0, 1) <= GWET_COWARDICE_GREETING_CHANCE)
				eAI_RandomGreeting(true);

			// Start the coward clock
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GwET_CowardiceTick, GWET_COWARDICE_TICK_MS, true);

			if (GwExpansionTweaksUtils.IsLogging())
				GwExpansionTweaksUtils.GwLog("eAIBase|GwET_StartCowardice", GetType() + " cowardice started - fleeing to " + m_GwET_FleePoint.ToString() + ", active cowards " + s_GwET_ActiveCowards);
		}

		// Just called while uncon
		private void GwET_RetryCowardice()
		{
			m_GwET_CowardiceRetryPending = false;
			GwET_StartCowardice();
		}

		// Try to pick a flee point behind the AI
		private vector GwET_PickFleePoint(vector ownPos)
		{
			vector fleeDir = GetDirection() * -1;
			fleeDir[1] = 0.0;

			vector firstPoint = ownPos + fleeDir.Normalized() * GWET_COWARDICE_FLEE_DISTANCE;
			if (!g_Game.SurfaceIsSea(firstPoint[0], firstPoint[2]))
				return firstPoint;

			// If flee point is in the drink, try a few more headings
			for (int attempt = 0; attempt < 3; attempt++)
			{
				float heading = Math.RandomFloat(0.0, Math.PI2);
				vector candidate = ownPos + Vector(Math.Cos(heading), 0.0, Math.Sin(heading)) * GWET_COWARDICE_FLEE_DISTANCE;
				if (!g_Game.SurfaceIsSea(candidate[0], candidate[2]))
					return candidate;
			}

			// If every candidate in the water, then HANG ON LADY we going for a swim
			return firstPoint;
		}

		// Drop every target and zero threat checks - this is pretty kitchen sinky because shit didnt work
		private void GwET_ClearThreatState()
		{
			eAITarget target;
			int targetIndex;

			for (targetIndex = TargetCount() - 1; targetIndex >= 0; targetIndex--)
			{
				target = GetTarget(targetIndex);
				if (target)
					target.Remove();
				else
					GetTargets().RemoveOrdered(targetIndex);
			}

			m_GwET_TargetCleanup.Clear();
			foreach (eAITargetInformation targetInfo, eAITarget targetState : m_eAI_TargetInformationStates)
			{
				if (targetInfo)
					m_GwET_TargetCleanup.Insert(targetInfo);
			}

			foreach (eAITargetInformation staleInfo : m_GwET_TargetCleanup)
				staleInfo.RemoveAI(this);

			// I know this sux
			m_GwET_TargetCleanup.Clear();
			m_eAI_CurrentThreatToSelf = 0.0;
			m_eAI_PreviousThreatToSelf = 0.0;
			m_eAI_CurrentThreatToSelfActive = 0.0;
			m_eAI_PreviousThreatToSelfActive = 0.0;
			m_ThreatClearedTimeout = 0.0;
			m_eAI_AcuteDangerTargetCount = 0;
			m_eAI_AcuteDangerPlayerTargetCount = 0;
			m_eAI_PotentialTargetEntities.Clear();
			m_eAI_TrackedBodies.Clear();
			m_eAI_CurrentPotentialTargetIndex = 0;
			m_eAI_UpdateTargetsTick = 0.25;
			m_eAI_TargetChanged = true;
			m_eAI_SyncCurrentTarget = true;
		}

		// Posture seemed to be a blocker 
		private void GwET_ResetCombatPosture()
		{
			GwET_ClearThreatState();
			Notify_Melee(false);
			eAI_SetIsFightingFSM(false);
			m_eAI_InteractingPlayers.Clear();
			eAI_LeaveCover();
			eAI_CancelSidestep();
			Expansion_GetUp(true);
			RaiseWeapon(false);
		}

		// Hold the AI passive and upright and moving
		private void GwET_SuppressThreatState()
		{
			if (!IsAlive() || !m_GwET_CowardiceActive || IsUnconscious())
				return;

			eAI_SetPassive(true);
			GwET_ResetCombatPosture();

			if (!GetCommand_Move())
				StartCommand_MoveAI();

			if (eAI_GetStance() != DayZPlayerConstants.STANCEIDX_ERECT)
				OverrideStance(DayZPlayerConstants.STANCEIDX_ERECT, true);

			EmoteManager emoteManager = GetEmoteManager();
			if (emoteManager && emoteManager.IsEmotePlaying())
				emoteManager.ServerRequestEmoteCancel();

			if (GetBrokenLegs())
				SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);

			eAI_SetThreatDistanceLimit(1.5);
			eAI_SetNoiseInvestigationDistanceLimit(0.1);
			SetMovementSpeedLimits(3, 3);
			SetSynchDirty();
		}

		// Extra stuff to test local threats, felt more natural than AI freight-training past me
		private bool GwET_HasNearbyThreat()
		{
			vector position = GetPosition();
			vector extent = Vector(GWET_COWARDICE_RETALIATE_DISTANCE, GWET_COWARDICE_RETALIATE_DISTANCE, GWET_COWARDICE_RETALIATE_DISTANCE);
			PlayerBase playerThreat;
			eAITargetInformation targetInfo;

			m_GwET_NearbyThreats.Clear();
			DayZPlayerUtils.SceneGetEntitiesInBox(position - extent, position + extent, m_GwET_NearbyThreats, QueryFlags.DYNAMIC);

			foreach (EntityAI entity : m_GwET_NearbyThreats)
			{
				if (!entity || entity == this || entity.IsDamageDestroyed() || entity.IsSetForDeletion())
					continue;

				if (Class.CastTo(playerThreat, entity))
				{
					if (playerThreat.IsAlive() && PlayerIsEnemy(playerThreat))
						return true;

					continue;
				}

				if (!entity.IsDayZCreature())
					continue;

				targetInfo = eAITargetInformation.GetTargetInformation(entity);
				if (targetInfo && targetInfo.CalculateThreat(this) > 0.0)
					return true;
			}

			return false;
		}

		// Maintain cowardice or end if a reason exists
		void GwET_CowardiceTick()
		{
			if (!IsAlive())
			{
				GwET_OnFleeComplete("AI dead");
				return;
			}

			float distanceToFleePointSq = vector.DistanceSq(GetPosition(), m_GwET_FleePoint);
			if (distanceToFleePointSq <= GWET_COWARDICE_ARRIVE_DIST_SQ)
			{
				GwET_OnFleeComplete("AI arrived");
				return;
			}

			float remainingMs = m_GwET_CowardiceUntil - g_Game.GetTime();
			if (remainingMs <= 0.0)
			{
				GwET_OnFleeComplete("AI got tired");
				return;
			}

			// Time has passed and we have a nearby threat - wakey wakey
			if (remainingMs < GWET_COWARDICE_RETALIATE_REMAINING_MS && distanceToFleePointSq < GWET_COWARDICE_PROGRESS_DIST_SQ)
			{
				if (GwET_HasNearbyThreat())
				{
					GwET_OnFleeComplete("close threat");
					return;
				}

				eAI_SetPassive(false);
				eAI_SetThreatDistanceLimit(5.0);
				eAI_SetNoiseInvestigationDistanceLimit(5.0);
			}
			else
			{
				GwET_SuppressThreatState();
			}

			eAIGroup group = GetGroup();
			if (group && group.GetWaypoints().Count() != 1)
			{
				group.ClearWaypoints();
				group.AddWaypoint(m_GwET_FleePoint);
			}
		}

		// Cowardice complete - reset and restore as a roaming looting raider
		private void GwET_OnFleeComplete(string reason)
		{
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(GwET_CowardiceTick);
			m_GwET_CowardiceActive = false;
			if (s_GwET_ActiveCowards > 0)
				s_GwET_ActiveCowards--;

			if (!IsAlive())
				return;

			GwET_ResetCombatPosture();
			eAI_SetPassive(false);
			OverrideTargetPosition(GetPosition());
			OverrideMovementDirection(false, 0.0);
			OverrideMovementSpeed(true, 0);
			SetMovementSpeedLimits(2, 3);
			eAI_SetLootingBehavior(eAILootingBehavior.ALL);
			eAI_ResetPathfinding();

			eAIGroup group = GetGroup();
			if (group)
			{
				group.SetIsInCombat(false);
				group.ClearWaypoints();
				group.SetWaypointBehaviour(eAIWaypointBehavior.ROAMING);
				group.SetFaction(new eAIFactionRaiders());
			}

			// Rebuild the FSM from scratch I think?
			m_FSM = null;
			LoadFSM();
			eAI_SetThreatDistanceLimit(-1);
			eAI_SetNoiseInvestigationDistanceLimit(-1);
			SetSynchDirty();

			if (GwExpansionTweaksUtils.IsLogging())
				GwExpansionTweaksUtils.GwLog("eAIBase|GwET_OnFleeComplete", GetType() + " cowardice complete - " + reason + ", active cowards " + s_GwET_ActiveCowards);
		}

		// Apply AI visual tweaks on AI spawn
		void GwET_ApplyVisuals()
		{
			if (!IsAlive())
				return;

			float beardChance = GetGwTweaksConfig().ExpansionTweaksForAI_ChanceSpawnWithBeard;
			float bloodyHandsChance = GetGwTweaksConfig().ExpansionTweaksForAI_ChanceSpawnWithBloodyHands;

			// Skip if both chances are off or the lifespan manager doesnt wanna play
			PluginLifespan lifespan = PluginLifespan.GetInstance();
			if (beardChance + bloodyHandsChance <= 0 || !lifespan)
				return;

			lifespan.GwET_RegisterAILifespan(GetType());

			// Pick a visible beard state (1-3) for men if we roll a chance
			bool changed = false;
			int beardState = 0;
			if (beardChance > 0 && IsMale() && Math.RandomFloatInclusive(0, 1) <= beardChance)
			{
				beardState = Math.RandomIntInclusive(1, 3);
				lifespan.SynchBeardVisual(this, beardState);
				changed = true;
			}

			// Set bloody hands for all if we roll a chance
			bool bloodyHands = false;
			if (bloodyHandsChance > 0 && !HasBloodyHands() && Math.RandomFloatInclusive(0, 1) <= bloodyHandsChance)
			{
				lifespan.SetBloodyHandsEx(this, eBloodyHandsTypes.JUST_BLOOD);
				bloodyHands = true;
				changed = true;
			}

			// only bother syncing if something got tweaked
			if (changed)
			{
				Expansion_SynchLifespanVisual();
				if (GwExpansionTweaksUtils.IsLogging())
					GwExpansionTweaksUtils.GwLog("eAIBase|GwET_ApplyVisuals", GetType() + " beard " + beardState + " bloodyHands " + bloodyHands);
			}
		}

		// Tiny override for Agent001's MadBomberAI - if an AI picks up the bomb, keep it equipped
		#ifdef MadBomberAI
		override bool eAI_TakeItemToHands(ItemBase item, bool useAction = true)
		{
			bool result = super.eAI_TakeItemToHands(item, useAction);
			MadBomberAI_CheckHands();
			return result;
		}

		// Check if we have a bomb in hands?
		private void MadBomberAI_CheckHands()
		{
			if (m_MadBomberAI_Locked)
				return;

			EntityAI hands = GetHumanInventory().GetEntityInHands();
			if (!hands || !GwExpansionTweaksUtils.Normalize(hands.GetType()).Contains("aysa_madbomb"))
				return;

			m_MadBomberAI_Locked = true;
			eAI_SetLootingBehavior(eAILootingBehavior.NONE);

			if (GwExpansionTweaksUtils.IsLogging())
				GwExpansionTweaksUtils.GwLog("eAIBase|MadBomberAI_CheckHands", GetType() + " locked onto " + hands.GetType() + " - looting disabled");
		}
		#endif

	#endif
	}

	// Cowardice trigger - finished bandaging (sorta)
	#ifdef SERVER
	modded class eAIState_Bandaging_Self
	{
		override void OnExit(string Event, bool Aborted, ExpansionState To)
		{
			super.OnExit(Event, Aborted, To);

			if (!unit || !GetGwTweaksConfig().ExpansionTweaksForAI_Enabled || GetGwTweaksConfig().ExpansionTweaksForAI_ChanceForCowardice <= 0)
				return;

			// Force bloody hands on bandaging if not already
			PluginLifespan lifespan = PluginLifespan.GetInstance();
			if (lifespan && !unit.HasBloodyHands())
			{
				lifespan.GwET_RegisterAILifespan(unit.GetType());
				lifespan.SetBloodyHandsEx(unit, eBloodyHandsTypes.JUST_BLOOD);
				unit.Expansion_SynchLifespanVisual();
			}

			// Check if AI's a chicken after done bandaging - either no bleeds or bandages left
			if (!unit.GetBandageToUse() || (!Aborted && unit.GetBleedingSourceCount() == 0))
				unit.GwET_EvaluateCowardice();
		}
	}
	#endif

	// Lifespan ref for AI hands + beard
	modded class PluginLifespan
	{
		void GwET_RegisterAILifespan(string aiType)
		{
			string parent;
			int depth;

			bool hasLifespan = m_LifespanLevels.Contains(aiType);
			bool hasBloodyHands = m_BloodyHands.Contains(aiType);
			if (hasBloodyHands)
				return;

			string baseType = aiType;
			for (depth = 0; depth < 10; depth++)
			{
				if (!g_Game.ConfigGetBaseName("CfgVehicles " + baseType, parent) || parent == "" || parent == baseType)
					return;

				baseType = parent;

				if (!hasLifespan && m_LifespanLevels.Contains(baseType))
				{
					m_LifespanLevels.Set(aiType, m_LifespanLevels.Get(baseType));
					hasLifespan = true;
				}

				if (!hasBloodyHands && m_BloodyHands.Contains(baseType))
				{
					m_BloodyHands.Set(aiType, m_BloodyHands.Get(baseType));
					hasBloodyHands = true;
				}

				if (hasLifespan && hasBloodyHands)
					return;
			}
		}

		override void SynchLifespanVisual(PlayerBase player, int beard_state_visible, bool bloody_hands_visible, bool blood_type_visible, int blood_type)
		{
			if (player && player.IsInherited(eAIBase))
				GwET_RegisterAILifespan(player.GetType());

			super.SynchLifespanVisual(player, beard_state_visible, bloody_hands_visible, blood_type_visible, blood_type);
		}
	}

#endif

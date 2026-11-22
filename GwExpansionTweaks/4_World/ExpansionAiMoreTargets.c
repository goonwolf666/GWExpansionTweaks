// Expansion AI tweaks: adds custom entities as AI targets

#ifdef EXPANSIONMODAI

	// Treat listed animal entities as if they were predators
	modded class AnimalBase
	{
		override bool Expansion_IsDanger()
		{
			if (super.Expansion_IsDanger())
				return true;

			return GwExpansionTweaksUtils.IsForcedAiTarget(this, m_Expansion_LobotomyInProgress || m_Expansion_IsLobotomized);
		}
	}

	// Treat listed zombie entities as if they were predators
	modded class ZombieBase
	{
		override bool Expansion_IsDanger()
		{
			if (super.Expansion_IsDanger())
				return true;

			return GwExpansionTweaksUtils.IsForcedAiTarget(this, m_Expansion_LobotomyInProgress || m_Expansion_IsLobotomized);
		}
	}

	// Treat other non-animal/zed entities as a threat, dimishing with distance
	class eAIForcedCreatureTargetInformation: eAICustomCreatureTargetInformation
	{
		// My own drop off distances after testing lots of gunplay with eAI + AI Bandits...
		static const float GWET_THREAT_DISTANCE_CLOSE = 20.0;
		static const float GWET_THREAT_DISTANCE_NEAR = 100.0;
		static const float GWET_THREAT_DISTANCE_MID = 200.0;
		static const float GWET_THREAT_DISTANCE_MAX = 500.0;

		override bool IsAcuteDanger(eAIBase ai = null)
		{
			return true;
		}

		// Threat level very high up close, decreasing with distance until zero at THREAT_DISTANCE_MAX
		override float CalculateThreat(eAIBase ai = null, eAITargetInformationState state = null)
		{
			if (!ai || !m_Target || m_Target.IsDamageDestroyed() || m_Target.IsSetForDeletion() || ai.eAI_GetThreatOverride(m_Target))
				return 0.0;

			float distance = GetDistance(ai, true);
			if (distance <= GWET_THREAT_DISTANCE_CLOSE)
				return 5.0;

			if (distance <= GWET_THREAT_DISTANCE_NEAR)
				return ExpansionMath.LinearConversion(GWET_THREAT_DISTANCE_CLOSE, GWET_THREAT_DISTANCE_NEAR, distance, 5.0, 1.0);

			if (distance <= GWET_THREAT_DISTANCE_MID)
				return ExpansionMath.LinearConversion(GWET_THREAT_DISTANCE_NEAR, GWET_THREAT_DISTANCE_MID, distance, 1.0, 0.5);

			if (distance <= GWET_THREAT_DISTANCE_MAX)
				return ExpansionMath.LinearConversion(GWET_THREAT_DISTANCE_MID, GWET_THREAT_DISTANCE_MAX, distance, 0.5, 0.001);

			return 0.0;
		}

		override bool ShouldRemove(eAIBase ai = null)
		{
			return GetThreat(ai) <= 0.0;
		}
	}

	// Expansion target catch-all for custom creatures, added based the name array in config
	modded class eAITargetInformation
	{
		override static eAITargetInformation GetCustomCreatureTargetInformation(EntityAI entity)
		{
			eAITargetInformation info;

			if (!s_CustomAI.Find(entity, info))
			{
				if (GetGwTweaksConfig().ExpansionTweaksForAI_Enabled && GwExpansionTweaksUtils.IsStringInListSub(entity.GetType(), GetGwTweaksConfig().ExpansionTweaksForAI_AdditionalTargetEntities))
				{
					info = new eAIForcedCreatureTargetInformation(entity);
					if (GwExpansionTweaksUtils.IsLogging())
						GwExpansionTweaksUtils.GwLog("eAITargetInformation|GetCustomCreatureTargetInformation", entity.GetType() + " forced target at " + entity.GetPosition().ToString());
				}
				else if (entity.IsInherited(DayZInfected))
					info = new eAICustomInfectedTargetInformation(entity);
				else
					info = new eAICustomCreatureTargetInformation(entity);

				s_CustomAI[entity] = info;
			}

			return info;
		}
	}

#endif

// Expansion Mission tweaks: slightly randomize airdrop start times and announce gaps
// Missions get progressively more staggered after this tweak has been running a while

#ifdef EXPANSIONMODMISSIONS
modded class ExpansionMissionModule
{
	override void RemoveMission(ExpansionMissionEventBase mission)
	{
		super.RemoveMission(mission);

		if (!GetGwTweaksConfig().RandomizeExpansionMissionTimes_Enabled)
			return;

		// Expansion queues StartNewMissions in super, this removes that and re-queues it with a staggered time
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(StartNewMissions);
		int gap = GwExpansionTweaksUtils.RandomizeDelayMs(m_MissionSettings.TimeBetweenMissions);
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartNewMissions, gap, false);
	}

	// No super here unfortunately due to the way Expansion's structured - hopefully not too brittle?!
	override void StartNewMissionsInternal()
	{
	#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MISSIONS, this);
	#endif

		if (m_Missions.Count() == 0)
			return;

		if (m_RunningMissions.Count() >= m_MissionSettings.MaxMissions)
			return;

		bool skip;
		if (m_RunningMissions.Count() > m_MissionSettings.MinMissions)
		{
			float chance = ExpansionMath.LinearConversion(m_MissionSettings.MaxMissions, m_MissionSettings.MinMissions, m_RunningMissions.Count()) + 0.125;
			if (chance < Math.RandomFloatInclusive(0.0, 1.0))
				skip = true;
		}

		if (!skip && (!FindNewMission() || m_RunningMissions.Count() >= m_MissionSettings.MaxMissions))
			return;

		// Vanilla Expansion re-checks every 10s; the tweak spreads announcements out around a minute instead
		int announceGap = 10000;
		if (GetGwTweaksConfig().RandomizeExpansionMissionTimes_Enabled)
			announceGap = GwExpansionTweaksUtils.RandomizeDelayMs(60000);

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartNewMissionsInternal, announceGap, false);
	}
}
#endif

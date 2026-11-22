// Expansion Tweaks: sync config to client on connect

modded class MissionBase
{
	void MissionBase()
	{
		#ifndef SERVER
		GetRPCManager().AddRPC("GwExpansionTweaks", "RPC_Receive_GwExpansionTweaksConfig", this, SingleplayerExecutionType.Client);
		#endif
	}

	void RPC_Receive_GwExpansionTweaksConfig(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;

		Param1<ref GwExpansionTweaksConfig> data;

		if (!ctx.Read(data))
		{
			Print("[GwExpansionTweaks] (MissionBase|RPC_Receive_GwExpansionTweaksConfig) Failed to read config RPC");
			return;
		}

		if (!data || !data.param1)
		{
			Print("[GwExpansionTweaks] (MissionBase|RPC_Receive_GwExpansionTweaksConfig) Received NULL config");
			return;
		}

		GwExpansionTweaksConfig.SetSyncedConfig(data.param1);
	}
}

// Expansion Tweaks: read config from server and send to client on connect

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		GetGwTweaksConfig();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!identity)
			return;

		GetRPCManager().SendRPC("GwExpansionTweaks", "RPC_Receive_GwExpansionTweaksConfig", new Param1<ref GwExpansionTweaksConfig>(GetGwTweaksConfig()), true, identity);

		if (GwExpansionTweaksUtils.IsLogging())
			GwExpansionTweaksUtils.GwLog("MissionServer|InvokeOnConnect", "Config sync sent to " + identity.GetId());
	}
}

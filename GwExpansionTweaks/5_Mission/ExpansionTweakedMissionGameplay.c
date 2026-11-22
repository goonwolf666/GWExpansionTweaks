// Expansion BOOK tweak: intercept book/chat key binds if approved items required
modded class MissionGameplay
{
	#ifdef EXPANSIONMODBOOK
	override void OnBookTogglePressed()
	{
		// Fall through to Expansion defaults if book items not configured
		if (!GetGwTweaksConfig().ExpansionBookItem_Enabled || GetGwTweaksConfig().ExpansionBookItem_AllowedItemTypes.Count() == 0)
		{
			super.OnBookTogglePressed();
			return;
		}

		if (GwExpansionTweaksUtils.FindLocalPlayerItem(GetGwTweaksConfig().ExpansionBookItem_AllowedItemTypes))
		{
			super.OnBookTogglePressed();
			return;
		}

		if (GwExpansionTweaksUtils.IsLogging())
			GwExpansionTweaksUtils.GwLog("MissionGameplay|OnBookTogglePressed", "DENIED - carrying none of " + GwExpansionTweaksUtils.JoinTypes(GetGwTweaksConfig().ExpansionBookItem_AllowedItemTypes));
	}
	#endif

	#ifdef EXPANSIONMODCHAT
	#ifndef SERVER
	override void ShowChat()
	{
		if (GwExpansionTweaksUtils.BlocksGlobalChat(g_Game.GetPlayer()) && (GetChatChannel() == ExpansionChatChannels.CCGlobal || GetChatChannel() == ExpansionChatChannels.CCTeam))
			SwitchChatChannelToDirect();

		super.ShowChat();
	}

	override void SwitchChatChannelToGlobal()
	{
		if (GwExpansionTweaksUtils.BlocksGlobalChat(g_Game.GetPlayer()))
		{
			SwitchChatChannelToDirect();
			return;
		}

		super.SwitchChatChannelToGlobal();
	}

	override void SwitchChatChannelToTeam()
	{
		if (GwExpansionTweaksUtils.BlocksGlobalChat(g_Game.GetPlayer()))
		{
			SwitchChatChannelToDirect();
			return;
		}

		super.SwitchChatChannelToTeam();
	}
	#endif
	#endif
}

// Force direct channel only if player doesnt have approved radio item
#ifdef EXPANSIONMODCHAT
#ifndef SERVER
modded class Chat
{
	override void OnChatInputShow()
	{
		if (GwExpansionTweaksUtils.BlocksGlobalChat(g_Game.GetPlayer()))
		{
			MissionGameplay gameplayMission = MissionGameplay.Cast(g_Game.GetMission());
			if (gameplayMission && (gameplayMission.GetChatChannel() == ExpansionChatChannels.CCGlobal || gameplayMission.GetChatChannel() == ExpansionChatChannels.CCTeam))
				gameplayMission.SwitchChatChannelToDirect();
		}

		super.OnChatInputShow();
	}
}
#endif
#endif

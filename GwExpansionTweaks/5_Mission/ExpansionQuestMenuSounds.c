// Expansion QUESTS tweak: play custom radio ambience in quest menu

#ifdef EXPANSIONMODQUESTS

	modded class ExpansionQuestMenu
	{
		// Use custom + vanilla soundsets
		static const string GWET_SND_QUESTMENU_STATIC = "QuestRadioStatic_SoundSet";
		static const string GWET_SND_QUESTMENU_EXIT = "turnOffRadioNoise_SoundSet";
		private static ref EffectSound s_GwET_QuestMenuStaticSound;

		override void SetQuests(string npcName = "", string defaultText = "", int questNPCID = -1, int questID = -1, int serverTime = 0)
		{
			if (RadioQuestBoardHelper.IsRadioMenuContext(questNPCID))
				GwET_PlayQuestMenuStaticSound();

			super.SetQuests(npcName, defaultText, questNPCID, questID, serverTime);
		}

		override void OnHide()
		{
			bool isRadioMenu = RadioQuestBoardHelper.IsRadioMenuContext(m_CurrentNPCID);

			super.OnHide();

			if (isRadioMenu)
				GwET_StopQuestMenuStaticSound();
		}

		private static void GwET_PlayQuestMenuStaticSound()
		{
			if (s_GwET_QuestMenuStaticSound)
				return;

			s_GwET_QuestMenuStaticSound = GwExpansionTweaksUtils.PlayAutodestroySound(GWET_SND_QUESTMENU_STATIC, g_Game.GetPlayer(), 0.15, 0.15, true);
		}

		private static void GwET_StopQuestMenuStaticSound()
		{
			if (!s_GwET_QuestMenuStaticSound)
				return;

			s_GwET_QuestMenuStaticSound.SoundStop();
			s_GwET_QuestMenuStaticSound = null;

			GwExpansionTweaksUtils.PlayAutodestroySound(GWET_SND_QUESTMENU_EXIT, g_Game.GetPlayer());
		}
	}

#endif

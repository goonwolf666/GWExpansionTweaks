// Expansion Notification & Chat tweaks: requires player to have an approved radio item

#ifdef EXPANSIONMODCHAT
	// Global/team chats require a radio; admin and proximity stay open
	modded class ExpansionGlobalChatModule
	{
		#ifdef SERVER
		override bool CanSendInChannel(ExpansionChatChannels channel, PlayerIdentity sender)
		{
			if (!super.CanSendInChannel(channel, sender))
				return false;

			if (channel != ExpansionChatChannels.CCGlobal && channel != ExpansionChatChannels.CCTeam)
				return true;

			PlayerBase player;
			if (sender)
				player = PlayerBase.Cast(sender.GetPlayer());

			return !GwExpansionTweaksUtils.BlocksGlobalChat(player);
		}
		#endif

		#ifndef SERVER
		override void AddChatMessage_Client(PlayerIdentity sender, Object target, ParamsReadContext ctx, ExpansionChatMessageEventParams data)
		{
			if (data && (data.param1 == ExpansionChatChannels.CCGlobal || data.param1 == ExpansionChatChannels.CCTeam) && GwExpansionTweaksUtils.BlocksGlobalChat(g_Game.GetPlayer()))
				return;

			super.AddChatMessage_Client(sender, target, ctx, data);
		}
		#endif
	}
#endif

#ifdef EXPANSIONUI
#ifndef SERVER
	// Client-side:Expansion notifications requires a radio, except configured 'bypass' notifications
	modded class ExpansionNotificationView
	{
		// Use vanilla DayZ soundsets
		static const string SND_RADIO_ON = "turnOnRadioNoise_SoundSet";
		static const string SND_RADIO_STATIC = "personalradio_staticnoise_SoundSet";
		static const string SND_RADIO_TUNE = "tuneRadio_SoundSet";
		static const string SND_RADIO_SPECIAL = "defibrillator_ready_SoundSet";
		static const string SND_RADIO_SPECIALSTATIC = "baseradio_staticnoise_SoundSet";

		// Radio static sounds run for half the total notification time, except specials
		static const float SND_STATIC_MIN_MS = 1000.0;
		static const float SND_STATIC_MAX_MS = 20000.0;
		static const int SND_SPECIAL_EXTRA_MS = 666;

		protected ref EffectSound m_RadioSound;

		// Play the radio static on player receiving notification and queue the tune-out
		void StartRadioSound(bool special)
		{
			PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
			if (!player)
				return;

			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(StopStaticSound);
			if (m_RadioSound)
			{
				m_RadioSound.SoundStop();
				m_RadioSound = null;
			}

			// Radio static to run for half of notification time
			int durationMs = (int)Math.Clamp(m_Data.GetTime() * 500.0, SND_STATIC_MIN_MS, SND_STATIC_MAX_MS);
			GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_ON, player);

			// Special notifications get base radio static sound and defib tone
			if (special)
			{
				m_RadioSound = GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_SPECIALSTATIC, player, 0.15, 0.15, true);
				GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_SPECIAL, player, 0.25, 0.15);
				durationMs += SND_SPECIAL_EXTRA_MS;
			}
			else
			{
				m_RadioSound = GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_STATIC, player, 0.15, 0.15, true);
			}

			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(StopStaticSound, durationMs, false);
		}

		void StopStaticSound()
		{
			if (!m_RadioSound)
				return;

			m_RadioSound.SoundStop();
			m_RadioSound = null;

			GwExpansionTweaksUtils.PlayAutodestroySound(SND_RADIO_TUNE, g_Game.GetPlayer());
		}

		// Suppress notification if player doesnt have radio
		private void DeclineNotification()
		{
			if (GetLayoutRoot())
				GetLayoutRoot().Show(false);

			if (m_NotificationModule)
				m_NotificationModule.RemoveNotificationView(this);

			RemoveNotificationElement();
		}

		// Show notification player has radio item or it's a bypassed message
		override void ShowNotification()
		{
			// pseudo enum: 0 = Expansion sound, 1 = our radio sound, 2 = our special sound
			int soundType = 0;

			string notificationText = GwExpansionTweaksUtils.Normalize(m_Data.GetTitleText() + m_Data.GetDetailText());

			if (GetGwTweaksConfig().ExpansionRadioNotifications_Enabled && GetGwTweaksConfig().ExpansionRadioNotifications_AllowedItemTypes.Count() > 0 && !notificationText.Contains("quest"))
			{
				// Bypass keywords keep the normal expansion look and feel
				if (!GwExpansionTweaksUtils.IsStringInListSub(notificationText, GetGwTweaksConfig().ExpansionRadioNotifications_BypassKeywords))
				{
					// No approved radio/device and not a bypass notification - suppress the box entirely
					if (!GwExpansionTweaksUtils.FindLocalPlayerItem(GetGwTweaksConfig().ExpansionRadioNotifications_AllowedItemTypes))
					{
						DeclineNotification();
						return;
					}

					soundType = 1;
					if (GwExpansionTweaksUtils.IsStringInListSub(notificationText, GetGwTweaksConfig().ExpansionRadioNotifications_SpecialKeywords))
						soundType = 2;
				}
			}

			// Only play a sound if Expansion notification settings say we should
			if (GetExpansionClientSettings().NotificationSound)
			{
				if (soundType == 0)
					PlaySound();
				else
					StartRadioSound(soundType == 2);
			}

			// Fall through to the standard Expansion notification
			GetLayoutRoot().Show(true);
			m_Showing = true;
			m_ShowUpdateTime = 0;
			SetView();
		}
	}

#endif
#endif

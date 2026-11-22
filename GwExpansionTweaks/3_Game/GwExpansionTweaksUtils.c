// Expansion Tweaks utilities: logging, string/entity matching, client sound manager and time jitterizer

class GwExpansionTweaksUtils
{
	private static bool s_LoggingEnabled;
	private static ref array<EntityAI> s_InventoryScan = new array<EntityAI>;
	private static ref array<Man> s_PlayerScratch;

	static void SetLoggingEnabled(bool enabled)
	{
		s_LoggingEnabled = enabled;
	}

	static bool IsLogging()
	{
		return s_LoggingEnabled;
	}

	static void GwLog(string context, string message)
	{
		if (!s_LoggingEnabled)
			return;

		string side = "client";
		if (g_Game.IsDedicatedServer())
			side = "server";

		Print(GetYmdhmsTimestamp() + " [GwExpansionTweaks " + side + "] (" + context + ") " + message);
	}

	static string GetYmdhmsTimestamp()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);

		int yy = year % 100;
		return yy.ToStringLen(2) + month.ToStringLen(2) + day.ToStringLen(2) + hour.ToStringLen(2) + minute.ToStringLen(2) + second.ToStringLen(2);
	}

	static string Normalize(string value)
	{
		string result = value + "";
		result.TrimInPlace();
		result.ToLower();
		return result;
	}

	static bool IsStringInListSub(string typeName, array<string> allowedTypes)
	{
		if (!allowedTypes)
			return false;

		string haystack = Normalize(typeName);
		foreach (string hay : allowedTypes)
		{
			string needle = Normalize(hay);
			if (needle != "" && haystack.Contains(needle))
				return true;
		}

		return false;
	}

	static string JoinTypes(array<string> values)
	{
		if (!values || values.Count() == 0)
			return "(empty)";

		return string.Join(", ", values);
	}

	static EntityAI FindLocalPlayerItem(array<string> allowedTypes)
	{
		return FindItemInInventory(g_Game.GetPlayer(), allowedTypes);
	}

	static EntityAI FindItemInInventory(Man player, array<string> allowedTypes)
	{
		if (!player || !player.GetInventory() || !allowedTypes || allowedTypes.Count() == 0)
			return null;

		EntityAI match;

		s_InventoryScan.Clear();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, s_InventoryScan);

		foreach (EntityAI item : s_InventoryScan)
		{
			if (!item || item.IsRuined() || !IsStringInListSub(item.GetType(), allowedTypes))
				continue;

			if (!HasBatteryIfRequired(item))
				continue;

			match = item;
			break;
		}

		s_InventoryScan.Clear();
		return match;
	}

	static bool HasBatteryIfRequired(EntityAI parent)
	{
		if (!parent || !parent.GetInventory())
			return true;

		GameInventory inv = parent.GetInventory();
		int slotCount = inv.GetAttachmentSlotsCount();
		if (slotCount == 0)
			return true;

		bool needsBattery = false;
		int slotId;
		string slotName;

		for (int i = 0; i < slotCount; i++)
		{
			slotId = inv.GetAttachmentSlotId(i);
			slotName = Normalize(InventorySlots.GetSlotName(slotId));
			if (slotName.IndexOf("battery") == -1)
				continue;

			needsBattery = true;
			if (inv.FindAttachment(slotId))
				return true;
		}

		return !needsBattery;
	}

	static bool BlocksGlobalChat(Man player)
	{
		if (!GetGwTweaksConfig().ExpansionRadioNotifications_Enabled || !GetGwTweaksConfig().ExpansionRadioNotifications_RequiredForGlobalChat || GetGwTweaksConfig().ExpansionRadioNotifications_AllowedItemTypes.Count() == 0)
			return false;

		return FindItemInInventory(player, GetGwTweaksConfig().ExpansionRadioNotifications_AllowedItemTypes) == null;
	}

	static bool IsForcedAiTarget(DayZCreatureAI creature, bool lobotomized)
	{
		if (!GetGwTweaksConfig().ExpansionTweaksForAI_Enabled)
			return false;

		if (!creature || !creature.IsAlive() || !creature.GetAIAgent() || lobotomized)
			return false;

		return IsStringInListSub(creature.GetType(), GetGwTweaksConfig().ExpansionTweaksForAI_AdditionalTargetEntities);
	}

	static EffectSound PlayAutodestroySound(string soundSet, Object parent, float fadeIn = 0.15, float fadeOut = 0.15, bool loop = false)
	{
		if (!g_Game.IsClient() || !parent)
			return null;

		EffectSound sound = SEffectManager.PlaySoundOnObject(soundSet, parent, fadeIn, fadeOut, loop);
		if (sound)
			sound.SetAutodestroy(true);

		return sound;
	}

	static int RandomizeDelayMs(float baseMs)
	{
		if (!GetGwTweaksConfig().RandomizeExpansionMissionTimes_Enabled)
			return baseMs;

		return baseMs * Math.RandomFloatInclusive(GetGwTweaksConfig().RandomizeExpansionMissionTimes_MinMultiplier, GetGwTweaksConfig().RandomizeExpansionMissionTimes_MaxMultiplier);
	}

	// honestly think this is faster than SceneGetEntitiesInBox, especially for low pop servers
	static bool IsPlayerWithinRange(vector source, float range)
	{
		if (range <= 0)
			return false;

		if (!s_PlayerScratch)
			s_PlayerScratch = new array<Man>;

		s_PlayerScratch.Clear();
		g_Game.GetPlayers(s_PlayerScratch);

		float rangeSq = range * range;
		foreach (Man man : s_PlayerScratch)
		{
			vector pos = man.GetPosition();
			float dx = source[0] - pos[0];
			float dz = source[2] - pos[2];
			if ((dx * dx) + (dz * dz) > rangeSq)
				continue;

			if (man.IsAlive() && man.GetIdentity())
				return true;
		}

		return false;
	}
}

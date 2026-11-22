// Expansion Tweaks: JSON config stuff
class GwExpansionTweaksConfig
{
	private static ref GwExpansionTweaksConfig m_Instance;

	static const string MOD_AUTHOR = "Goonwolf";
	static const int MAX_QUEST_RADIO_CHANNELS = 100;
	static const int DEFAULT_RADIO_QUEST_GIVER_ID = 9001;
	static const float QUEST_RADIO_FREQUENCY_EPSILON = 0.01;

	private static const string CONFIG_FOLDER = "$profile:\\" + MOD_AUTHOR + "\\";
	private static const string CONFIG_NAME = "GwExpansionTweaks.json";
	private static const string CONFIG_FULLPATH = CONFIG_FOLDER + CONFIG_NAME;
	private static const string CURRENT_VERSION = "1";

	// Frequencies the vanilla radios can tune to. Hard to find in the source so thanks dayz.fandom.com 
	private static ref array<float> TUNABLE_FREQUENCIES = {87.8, 89.5, 91.3, 94.6, 96.6, 99.7, 102.5};

	string CONFIG_VERSION;

	bool ExpansionTweaksSessionLogging;

	bool RandomizeExpansionMissionTimes_Enabled;
	float RandomizeExpansionMissionTimes_MinMultiplier;
	float RandomizeExpansionMissionTimes_MaxMultiplier;

	bool ExpansionBookItem_Enabled;
	ref array<string> ExpansionBookItem_AllowedItemTypes;
	string ExpansionBookItem_ItemNameText;
	string ExpansionBookItem_ItemDescriptionText;
	string ExpansionBookItem_KeyBindingText;
	string ExpansionBookItem_NotReadableText;
	string ExpansionBookItem_TooDamagedText;

	bool ExpansionRadioNotifications_Enabled;
	ref array<string> ExpansionRadioNotifications_AllowedItemTypes;
	ref array<string> ExpansionRadioNotifications_SpecialKeywords;
	ref array<string> ExpansionRadioNotifications_BypassKeywords;
	bool ExpansionRadioNotifications_RequiredForGlobalChat;

	bool ExpansionLocationNotifications_Enabled;
	ref array<string> ExpansionLocationNotifications_GpsRequiredItems;

	bool ExpansionQuestsRadio_Enabled;
	ref array<string> ExpansionQuestsRadio_AllowedRadioTypes;
	ref array<ref GwExpansionQuestRadioChannel> ExpansionQuestsRadio_Channels;
	bool ExpansionQuestsRadio_FlareRewards;
	string ExpansionQuestsRadio_RewardText;
	string ExpansionQuestsRadio_NoQuestsText;

	bool ExpansionHouseInteriors_Enabled;

	bool ExpansionTweaksForAI_Enabled;
	float ExpansionTweaksForAI_MaleScaleMin;
	float ExpansionTweaksForAI_MaleScaleMax;
	float ExpansionTweaksForAI_FemaleScaleMin;
	float ExpansionTweaksForAI_FemaleScaleMax;
	ref array<string> ExpansionTweaksForAI_AdditionalTargetEntities;
	float ExpansionTweaksForAI_ChanceSpawnWithBeard;
	float ExpansionTweaksForAI_ChanceSpawnWithBloodyHands;
	float ExpansionTweaksForAI_ChanceForCowardice;

	static GwExpansionTweaksConfig GetInstance()
	{
		if (!m_Instance)
		{
			m_Instance = new GwExpansionTweaksConfig();
			m_Instance.Load();
		}

		return m_Instance;
	}

	static void SetSyncedConfig(GwExpansionTweaksConfig config)
	{
		if (!config)
		{
			Print("[GwExpansionTweaks] (GwExpansionTweaksConfig|SetSyncedConfig) REJECTED: received config is NULL");
			return;
		}

		m_Instance = config;
		m_Instance.Validate();
		GwExpansionTweaksUtils.SetLoggingEnabled(m_Instance.ExpansionTweaksSessionLogging);
		GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|SetSyncedConfig", "Config accepted and applied - version=" + config.CONFIG_VERSION);
	}

	void Load()
	{
		SetDefaultValues();

		// Clients just get what server sends em
		if (!g_Game.IsDedicatedServer() && g_Game.IsMultiplayer())
			return;

		if (!FileExist(CONFIG_FULLPATH))
		{
			GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Load", "No config found, writing defaults to " + CONFIG_FULLPATH);
			Save();
			return;
		}

		string error;

		if (!JsonFileLoader<GwExpansionTweaksConfig>.LoadFile(CONFIG_FULLPATH, this, error))
		{
			Print("[GwExpansionTweaks] (GwExpansionTweaksConfig|Load) Config load FAILED, running on defaults - " + error);
			SetDefaultValues();
			return;
		}

		GwExpansionTweaksUtils.SetLoggingEnabled(ExpansionTweaksSessionLogging);
		Validate();

		if (CONFIG_VERSION == CURRENT_VERSION)
		{
			GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Load", "Config loaded from " + CONFIG_FULLPATH);
			return;
		}

		string backupPath = CONFIG_FULLPATH + "." + GwExpansionTweaksUtils.GetYmdhmsTimestamp() + ".old";
		if (!CopyFile(CONFIG_FULLPATH, backupPath))
		{
			GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Load", "Migration skipped - could not archive old config to " + backupPath);
			return;
		}

		GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Load", "Archived old config to " + backupPath);
		CONFIG_VERSION = CURRENT_VERSION;
		Save();
	}

	void SetDefaultValues()
	{
		GwExpansionQuestRadioChannel questRadioChannel;

		CONFIG_VERSION = CURRENT_VERSION;
		ExpansionTweaksSessionLogging = false;

		RandomizeExpansionMissionTimes_Enabled = true;
		RandomizeExpansionMissionTimes_MinMultiplier = 0.7;
		RandomizeExpansionMissionTimes_MaxMultiplier = 1.3;

		ExpansionBookItem_Enabled = true;
		ExpansionBookItem_AllowedItemTypes = new array<string>;
		ExpansionBookItem_AllowedItemTypes.Insert("ExpBookItem");
		ExpansionBookItem_ItemNameText = "";
		ExpansionBookItem_ItemDescriptionText = "";
		ExpansionBookItem_KeyBindingText = "";
		ExpansionBookItem_NotReadableText = "";
		ExpansionBookItem_TooDamagedText = "";

		ExpansionRadioNotifications_Enabled = true;
		ExpansionRadioNotifications_AllowedItemTypes = new array<string>;
		ExpansionRadioNotifications_AllowedItemTypes.Insert("personalradio");
		ExpansionRadioNotifications_AllowedItemTypes.Insert("zenwatchgps");
		ExpansionRadioNotifications_AllowedItemTypes.Insert("nokiaz");
		ExpansionRadioNotifications_SpecialKeywords = new array<string>;
		ExpansionRadioNotifications_SpecialKeywords.Insert("killed");
		ExpansionRadioNotifications_SpecialKeywords.Insert("death");
		ExpansionRadioNotifications_SpecialKeywords.Insert("died");
		ExpansionRadioNotifications_BypassKeywords = new array<string>;
		ExpansionRadioNotifications_BypassKeywords.Insert("community online tools");
		ExpansionRadioNotifications_BypassKeywords.Insert("your skill category");
		ExpansionRadioNotifications_BypassKeywords.Insert("open your inventory");
		ExpansionRadioNotifications_BypassKeywords.Insert("abled the chat");
		ExpansionRadioNotifications_RequiredForGlobalChat = true;

		ExpansionLocationNotifications_Enabled = true;
		ExpansionLocationNotifications_GpsRequiredItems = new array<string>;
		ExpansionLocationNotifications_GpsRequiredItems.Insert("GPSReceiver");
		ExpansionLocationNotifications_GpsRequiredItems.Insert("ExpansionGPS");
		ExpansionLocationNotifications_GpsRequiredItems.Insert("ZenWatchGPS");

		ExpansionQuestsRadio_Enabled = true;
		ExpansionQuestsRadio_AllowedRadioTypes = new array<string>;
		ExpansionQuestsRadio_AllowedRadioTypes.Insert("BaseRadio");
		ExpansionQuestsRadio_AllowedRadioTypes.Insert("Land_Radio_PanelBig");
		ExpansionQuestsRadio_Channels = new array<ref GwExpansionQuestRadioChannel>;
		questRadioChannel = new GwExpansionQuestRadioChannel();
		questRadioChannel.Frequency = 102.5;
		questRadioChannel.HourActiveStart = 0;
		questRadioChannel.HourActiveEnd = 0;
		questRadioChannel.QuestGiverID = DEFAULT_RADIO_QUEST_GIVER_ID;
		ExpansionQuestsRadio_Channels.Insert(questRadioChannel);
		ExpansionQuestsRadio_FlareRewards = true;
		ExpansionQuestsRadio_RewardText = "";
		ExpansionQuestsRadio_NoQuestsText = "";

		ExpansionHouseInteriors_Enabled = true;

		ExpansionTweaksForAI_Enabled = true;
		ExpansionTweaksForAI_MaleScaleMin = 0.93;
		ExpansionTweaksForAI_MaleScaleMax = 1.13;
		ExpansionTweaksForAI_FemaleScaleMin = 0.91;
		ExpansionTweaksForAI_FemaleScaleMax = 1.11;
		ExpansionTweaksForAI_AdditionalTargetEntities = new array<string>;
		ExpansionTweaksForAI_AdditionalTargetEntities.Insert("doggo_patrol");
		ExpansionTweaksForAI_AdditionalTargetEntities.Insert("doggo_follow");
		ExpansionTweaksForAI_AdditionalTargetEntities.Insert("animal_horse");
		ExpansionTweaksForAI_AdditionalTargetEntities.Insert("banditai_");
		ExpansionTweaksForAI_ChanceSpawnWithBeard = 0.69;
		ExpansionTweaksForAI_ChanceSpawnWithBloodyHands = 0.5;
		ExpansionTweaksForAI_ChanceForCowardice = 0.13;

		Validate();
		GwExpansionTweaksUtils.SetLoggingEnabled(ExpansionTweaksSessionLogging);
	}

	private void Validate()
	{
		int channelIndex;
		int otherIndex;
		int hour;
		GwExpansionQuestRadioChannel channel;
		GwExpansionQuestRadioChannel otherChannel;

		ExpansionBookItem_AllowedItemTypes = EnsureStringArray(ExpansionBookItem_AllowedItemTypes);
		ExpansionRadioNotifications_AllowedItemTypes = EnsureStringArray(ExpansionRadioNotifications_AllowedItemTypes);
		ExpansionRadioNotifications_SpecialKeywords = EnsureStringArray(ExpansionRadioNotifications_SpecialKeywords);
		ExpansionRadioNotifications_BypassKeywords = EnsureStringArray(ExpansionRadioNotifications_BypassKeywords);
		ExpansionLocationNotifications_GpsRequiredItems = EnsureStringArray(ExpansionLocationNotifications_GpsRequiredItems);
		ExpansionQuestsRadio_AllowedRadioTypes = EnsureStringArray(ExpansionQuestsRadio_AllowedRadioTypes);
		ExpansionTweaksForAI_AdditionalTargetEntities = EnsureStringArray(ExpansionTweaksForAI_AdditionalTargetEntities);

		// Player-facing strings live here so no consumer needs its own fallback copy
		ExpansionBookItem_ItemNameText = EnsureText(ExpansionBookItem_ItemNameText, "Field Manual");
		ExpansionBookItem_ItemDescriptionText = EnsureText(ExpansionBookItem_ItemDescriptionText, "Army Survival Field Manual. This is the official guide to surviving extreme and life-threatening situations.");
		ExpansionBookItem_KeyBindingText = EnsureText(ExpansionBookItem_KeyBindingText, "Read by pressing [<BOOK_KEY_BINDING>]");
		ExpansionBookItem_NotReadableText = EnsureText(ExpansionBookItem_NotReadableText, "I'm not sure I can read this...");
		ExpansionBookItem_TooDamagedText = EnsureText(ExpansionBookItem_TooDamagedText, "This copy is too damaged to read...");
		ExpansionQuestsRadio_RewardText = EnsureText(ExpansionQuestsRadio_RewardText, "Reward has dropped near your location!");
		ExpansionQuestsRadio_NoQuestsText = EnsureText(ExpansionQuestsRadio_NoQuestsText, "No quests today... Check back later.");

		if (!ExpansionQuestsRadio_Channels)
			ExpansionQuestsRadio_Channels = new array<ref GwExpansionQuestRadioChannel>;

		if (ExpansionQuestsRadio_Channels.Count() > MAX_QUEST_RADIO_CHANNELS)
		{
			GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Validate", "Quest radio channel count exceeds " + MAX_QUEST_RADIO_CHANNELS + " - extra channels rejected");
			while (ExpansionQuestsRadio_Channels.Count() > MAX_QUEST_RADIO_CHANNELS)
				ExpansionQuestsRadio_Channels.Remove(ExpansionQuestsRadio_Channels.Count() - 1);
		}

		for (channelIndex = ExpansionQuestsRadio_Channels.Count() - 1; channelIndex >= 0; channelIndex--)
		{
			channel = ExpansionQuestsRadio_Channels[channelIndex];
			if (!channel)
			{
				ExpansionQuestsRadio_Channels.Remove(channelIndex);
				continue;
			}

			if (!IsTunableFrequency(channel.Frequency))
			{
				GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Validate", "REJECTED quest radio channel " + channelIndex + " - unsupported frequency " + channel.Frequency);
				ExpansionQuestsRadio_Channels.Remove(channelIndex);
				continue;
			}

			channel.HourActiveStart = Math.Clamp(channel.HourActiveStart, 0, 23);
			channel.HourActiveEnd = Math.Clamp(channel.HourActiveEnd, 0, 23);

			if (channel.QuestGiverID < 1)
			{
				channel.QuestGiverID = -1;
				GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Validate", "Quest radio channel " + channelIndex + " has a blank QuestGiverID - channel will list all Expansion-eligible quests");
			}
		}

		// Warn about channels that shadow each other - the first match at lookup time wins
		for (channelIndex = 0; channelIndex < ExpansionQuestsRadio_Channels.Count(); channelIndex++)
		{
			channel = ExpansionQuestsRadio_Channels[channelIndex];
			for (otherIndex = channelIndex + 1; otherIndex < ExpansionQuestsRadio_Channels.Count(); otherIndex++)
			{
				otherChannel = ExpansionQuestsRadio_Channels[otherIndex];
				if (Math.AbsFloat(channel.Frequency - otherChannel.Frequency) > QUEST_RADIO_FREQUENCY_EPSILON)
					continue;

				for (hour = 0; hour < 24; hour++)
				{
					if (!channel.IsActiveAtHour(hour) || !otherChannel.IsActiveAtHour(hour))
						continue;

					GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Validate", "Quest radio channels " + channelIndex + " and " + otherIndex + " overlap on " + channel.Frequency + " - first matching channel wins");
					break;
				}
			}
		}

		// Clamp multipliers to sane values: min beween 0-1, max between min-(min*2)
		if (RandomizeExpansionMissionTimes_MinMultiplier <= 0 || RandomizeExpansionMissionTimes_MaxMultiplier <= 0)
		{
			RandomizeExpansionMissionTimes_Enabled = false;
		}
		else
		{
			if (RandomizeExpansionMissionTimes_MinMultiplier > 1)
				RandomizeExpansionMissionTimes_MinMultiplier = 0.7;
			if (RandomizeExpansionMissionTimes_MaxMultiplier < RandomizeExpansionMissionTimes_MinMultiplier || RandomizeExpansionMissionTimes_MaxMultiplier > 2)
				RandomizeExpansionMissionTimes_MaxMultiplier = RandomizeExpansionMissionTimes_MinMultiplier * 1.8;
		}

		NormalizeScaleRange(ExpansionTweaksForAI_MaleScaleMin, ExpansionTweaksForAI_MaleScaleMax);
		NormalizeScaleRange(ExpansionTweaksForAI_FemaleScaleMin, ExpansionTweaksForAI_FemaleScaleMax);

		ExpansionTweaksForAI_ChanceSpawnWithBeard = Math.Clamp(ExpansionTweaksForAI_ChanceSpawnWithBeard, 0.0, 1.0);
		ExpansionTweaksForAI_ChanceSpawnWithBloodyHands = Math.Clamp(ExpansionTweaksForAI_ChanceSpawnWithBloodyHands, 0.0, 1.0);
		ExpansionTweaksForAI_ChanceForCowardice = Math.Clamp(ExpansionTweaksForAI_ChanceForCowardice, 0.0, 1.0);
	}

	private void NormalizeScaleRange(out float rangeMin, out float rangeMax)
	{
		if (rangeMin <= 0 || rangeMax <= 0)
			return;

		if (rangeMin > rangeMax)
		{
			float swap = rangeMin;
			rangeMin = rangeMax;
			rangeMax = swap;
		}

		rangeMin = Math.Clamp(rangeMin, 0.01, 2.0);
		rangeMax = Math.Clamp(rangeMax, 0.01, 2.0);
	}

	private array<string> EnsureStringArray(array<string> values)
	{
		if (!values)
			return new array<string>;

		return values;
	}

	private string EnsureText(string value, string fallback)
	{
		if (value == "")
			return fallback;

		return value;
	}

	private bool IsTunableFrequency(float frequency)
	{
		foreach (float tunable : TUNABLE_FREQUENCIES)
		{
			if (Math.AbsFloat(frequency - tunable) <= QUEST_RADIO_FREQUENCY_EPSILON)
				return true;
		}

		return false;
	}

	void Save()
	{
		// Clients never save their own config; the server is the source of truth.
		if (!g_Game.IsDedicatedServer() && g_Game.IsMultiplayer())
			return;

		if (!FileExist(CONFIG_FOLDER))
			MakeDirectory(CONFIG_FOLDER);

		string error;

		if (!JsonFileLoader<GwExpansionTweaksConfig>.SaveFile(CONFIG_FULLPATH, this, error))
		{
			GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Save", "Config save FAILED - " + error);
			return;
		}

		GwExpansionTweaksUtils.GwLog("GwExpansionTweaksConfig|Save", "Config saved to " + CONFIG_FULLPATH);
	}
}

static GwExpansionTweaksConfig GetGwTweaksConfig()
{
	return GwExpansionTweaksConfig.GetInstance();
}

// Quest radio channel class with frequency, active hours and quest giver ID 
class GwExpansionQuestRadioChannel
{
	float Frequency;
	int HourActiveStart;
	int HourActiveEnd;
	int QuestGiverID;

	void GwExpansionQuestRadioChannel()
	{
		QuestGiverID = -1;
	}

	bool IsActiveAtHour(int hour)
	{
		if (HourActiveStart + HourActiveEnd == 0)
			return true;

		if (HourActiveStart > HourActiveEnd)
			return hour >= HourActiveStart || hour <= HourActiveEnd;

		return hour >= HourActiveStart && hour <= HourActiveEnd;
	}
}
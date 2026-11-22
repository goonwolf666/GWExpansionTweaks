class CfgPatches
{
    class GwExpansionTweaks
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Sounds_Effects",
			"DF_Scripts",
			"DayZExpansion_Core_Scripts",
			"DayZExpansion_Book_Scripts"/*,
			"DayZExpansion_Quests_Scripts",
			"DayZExpansion_AI_Scripts",
			"DayZExpansion_Chat_Scripts",
			"DayZExpansion_Missions_Scripts",
			"DayZExpansion_Navigation_Scripts"*/
		};
	};
};

// Basic script stuff
class CfgMods
{
    class GwExpansionTweaks
    {
        name = "Goonwolf-Expansion-Tweaks";
		dir = "GwExpansionTweaks";
		author = "Goonwolf";
		type = "mod";
		version = "1.0";
		dependencies[] = {
			"Game", 
			"World", 
			"Mission"
		};
        class defs
        {
			class gameScriptModule
            {
                value = "";
                files[] = { "GwExpansionTweaks/3_Game"};
			};
			class worldScriptModule
            {
                value = "";
                files[] = { "GwExpansionTweaks/4_World"};
			};
			class missionScriptModule
            {
                value = "";
                files[] = { "GwExpansionTweaks/5_Mission"};
			};
		};
	};
};

// Custom Expansion Book object with default values
class cfgVehicles
{
	class Inventory_Base;
	class ExpBookItem: Inventory_Base
	{
		quantityShow = 0;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLevels[] = {{1.0,{"DZ\gear\books\Data\book.rvmat"}},{0.7,{"DZ\gear\books\Data\book.rvmat"}},{0.5,{"DZ\gear\navigation\data\Map_chernarus_damage.rvmat"}},{0.3,{"DZ\gear\books\Data\book_damage.rvmat"}},{0.0,{"DZ\gear\books\Data\book_destruct.rvmat"}}};
				};
			};
		};
		
		scope=2;
		title="Field Manual";
		author = "U.S. Army";
		displayName="Field Manual";
		descriptionShort="Army Survival Field Manual. This is the official guide to surviving extreme and life-threatening situations.";
		model="\DZ\gear\books\book_kniga.p3d";
		rotationFlags=1;		
		itemSize[]={1,1};
		weight=1;
		
		hiddenSelections[]=
		{
			"camoGround"
		};
		hiddenSelectionsTextures[]=
		{
			"GwExpansionTweaks\Data\BookItem.paa"
		};
	};
};

// Quest radio sounds customization
class CfgSoundShaders
{
	// Override vanilla base radio static to be quieter
    class baseCharacter_SoundShader;
    class baseradio_staticnoise_SoundShader: baseCharacter_SoundShader
    {
        volume = 0.10251;
    };
	// Override vanilla personal radio static to be louder :D
	class personalradio_staticnoise_SoundShader: baseCharacter_SoundShader
    {
        volume = 0.15050;
    };
	// Override vanilla personal radio startsound to be a bit louder too
	class turnOnRadioNoise_Soundshader: baseCharacter_SoundShader
	{
		volume = 0.25177;
	};
	
	// Add new quest radio chatter sounds
    class base_SoundShader
    {
        frequency=1;
        range=8;
        volume=1.11069;
        rangeCurve[] = {{0,1},{0.5,0.6},{0.75,0.1},{1,0}};
    };
    class QuestRadioStatic_SoundShader: base_SoundShader
    {
        samples[]=
        {
            { "GwExpansionTweaks\Data\RadioSounds\radio_czech1.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_czech2.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_czech3.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_german1.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_german2.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_german3.ogg", 1 },
			{ "GwExpansionTweaks\Data\RadioSounds\radio_english1.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_english2.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_english3.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_greek1.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_greek2.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_greek3.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_japanese1.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_japanese2.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_japanese3.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_russian1.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_russian2.ogg", 1 },
            { "GwExpansionTweaks\Data\RadioSounds\radio_russian3.ogg", 1 }
        };
	};
};
class CfgSoundSets
{
    class base_SoundSet
	{};

    class QuestRadioStatic_SoundSet: base_SoundSet
    {
        soundShaders[]= { "QuestRadioStatic_SoundShader" }; 
		loop=1;
		volumeFactor = 1.5;
		frequencyFactor = 1.0;
        frequencyRandomizer = 0.13;
        spatial=1;
    };
};
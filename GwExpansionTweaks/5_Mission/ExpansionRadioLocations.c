// Expansion NAVIGATION tweak: city/location text only shows with approved GPS item

#ifdef EXPANSIONMODNAVIGATION
modded class ExpansionLocatorUI
{
	override void OnShowCityClient( string title = "", string description = "", int duration = 5000, float fadein = 0.024, float fadeout = 0.024 )
	{
		if (GetGwTweaksConfig().ExpansionLocationNotifications_Enabled)
		{
			array<string> required = GetGwTweaksConfig().ExpansionLocationNotifications_GpsRequiredItems;
			if (required && required.Count() > 0 && !GwExpansionTweaksUtils.FindLocalPlayerItem(required))
				return;
		}

		super.OnShowCityClient( title, description, duration, fadein, fadeout );
	}
}
#endif

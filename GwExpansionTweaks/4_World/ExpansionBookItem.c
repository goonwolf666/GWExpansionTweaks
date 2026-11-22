// Expansion Book tweaks: adds ExpBookItem and custom description strings

class ExpBookItem extends ItemBase
{
	override bool NameOverride(out string output)
	{
		if (!GetGwTweaksConfig().ExpansionBookItem_Enabled)
			return false;

		output = GetGwTweaksConfig().ExpansionBookItem_ItemNameText;
		return true;
	}

	override bool DescriptionOverride(out string output)
	{
		if (!GetGwTweaksConfig().ExpansionBookItem_Enabled)
			return false;

		string additionalText = GetGwTweaksConfig().ExpansionBookItem_NotReadableText;
		string binding;

		#ifdef EXPANSIONMODBOOK
			ExpansionBookSettings bookSettings = GetExpansionSettings().GetBook(false);
			if (bookSettings && bookSettings.IsLoaded() && bookSettings.EnableBookMenu)
			{
				if (IsRuined())
				{
					additionalText = GetGwTweaksConfig().ExpansionBookItem_TooDamagedText;
				}
				else
				{
					// Swap the placeholder for whatever the player has the Expansion book toggle bound to
					additionalText = GetGwTweaksConfig().ExpansionBookItem_KeyBindingText;
					binding = GetExpansionBookKeyBind();
					if (additionalText.Contains("<BOOK_KEY_BINDING>"))
						additionalText.Replace("<BOOK_KEY_BINDING>", binding);
					else
						additionalText = binding;
				}
			}
		#endif

		output = GetGwTweaksConfig().ExpansionBookItem_ItemDescriptionText + " " + additionalText;
		return true;
	}

	// Human-readable key combos bound to the Expansion book toggle, e.g. "LCtrl + B; Numpad0"
	string GetExpansionBookKeyBind()
	{
		string ret = "";

		#ifdef EXPANSIONMODBOOK
			map<int, ref TStringArray> button_map = InputUtils.GetComboButtonNamesFromInput("UAExpansionBookToggle", EInputDeviceType.MOUSE_AND_KEYBOARD);
			if (!button_map)
				return ret;

			for (int j = 0; j < button_map.Count(); j++)
			{
				TStringArray buttons = button_map.GetElement(j);
				if (!buttons)
					continue;

				if (ret != "")
					ret += "; ";

				for (int i = 0; i < buttons.Count(); i++)
				{
					if (i > 0)
						ret += " + ";

					ret += buttons[i];
				}
			}
		#endif

		return ret;
	}
}

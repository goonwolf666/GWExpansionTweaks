// Expansion Interiors tweaks: randomizes the interior clutter (client-side) objects JUST a bit

modded class BuildingBase
{
    protected int m_GwET_ClutterSkipped;

    override void LoadInterior()
    {
        m_GwET_ClutterSkipped = 0;
        super.LoadInterior();

        if (m_GwET_ClutterSkipped > 0 && GwExpansionTweaksUtils.IsLogging())
            GwExpansionTweaksUtils.GwLog("ExpansionHouses|BuildingBase", GetType() + " at " + GetPosition().ToString() + " skipped " + m_GwET_ClutterSkipped + " clutter objects");
    }

    override protected Object SpawnInteriorObject(string type, vector position, vector orientation, float random = 1.0)
    {
        if (!GetGwTweaksConfig().ExpansionHouseInteriors_Enabled)
            return super.SpawnInteriorObject(type, position, orientation, random);

        bool hasCollision;
        if (s_InteriorModule.m_CachedCollision.Find(type, hasCollision) && !hasCollision)
        {
            // 33.4% chance to not spawn decorative/clutter objects
            if (Math.RandomFloatInclusive(0, 1) >= 0.666)
            {
                m_GwET_ClutterSkipped++;
                return NULL;
            }
        }

        // Interior furniture/colliding shit remains untouched
        return super.SpawnInteriorObject(type, position, orientation, random);
    }
}
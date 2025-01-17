#pragma once

#include <vector>
#include <cstdint>
#include <cstdio>
#include <string>
#include "weapon_patterns.hh"

int *GetWeaponInfo (int weaponId, char skill);

class WeaponRandomizer
{
    static WeaponRandomizer *                mInstance;
    static inline std::vector<WeaponPattern> mWeaponPatterns;

    WeaponRandomizer (){};
    static void DestroyInstance ();

public:
    /// Returns the static instance for WeaponRandomizer.
    static WeaponRandomizer *GetInstance ();

    static inline struct Config
    {
        bool RandomizePlayerWeapons;
        bool SkipChecks;
    } m_Config;

    int  GetRandomWeapon (CPed *ped, int weapon, bool isPickup);
    void CachePatterns ();

    struct WeaponInfo
    {
        int weaponID = 0;
        int ammo;
        int modelID;

        WeaponInfo () {}
    };

    static inline int  playerWeaponID         = -1;
    static inline int  dealerWeaponID         = random (22, 38);
    static inline bool forceWeapon            = false;
    static inline bool firstPartFinaleCActive = false;

    static inline WeaponInfo shPistol;
    static inline WeaponInfo shShotgun;
    static inline WeaponInfo shUzi;
    static inline WeaponInfo shAK;

    /// Initialises Hooks/etc.
    void Initialise ();
    void InitialiseCache ();
};

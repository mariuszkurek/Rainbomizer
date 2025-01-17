/*
    Rainbomizer - A (probably fun) Grand Theft Auto San Andreas Mod that
                  randomizes stuff
    Copyright (C) 2019 - Parik

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

 */

#include <cstdlib>
#include "colours.hh"
#include "base.hh"
#include "functions.hh"
#include <cmath>
#include "logger.hh"
#include "config.hh"
#include <algorithm>
#include "injector/calling.hpp"
#include <chrono>
#include <string>
#include "fades.hh"

ColourRandomizer *ColourRandomizer::mInstance = nullptr;

// Store RGB colours and light IDs to ensure each one stays consistent without
// flickering.
static std::vector<ColourRandomizer::Pattern> colourPatterns;
static ColourRandomizer::Pattern              weaponHUDSprite;

ColourRandomizer::Pattern
GetColour (ColourRandomizer::Pattern pattern)
{
    for (int i = 0; i < colourPatterns.size (); i++)
        {
            if (colourPatterns[i].ID == pattern.ID)
                {
                    return colourPatterns[i];
                }
        }
    return {-1, {-1, -1, -1}};
}

/*******************************************************/
void
HSVtoRGB (int H, double S, double V, int output[3])
{
    double C = S * V;
    double X = C * (1 - std::abs (fmod (H / 60.0, 2) - 1));
    double m = V - C;
    double Rs, Gs, Bs;

    if (H >= 0 && H < 60)
        {
            Rs = C;
            Gs = X;
            Bs = 0;
        }
    else if (H >= 60 && H < 120)
        {
            Rs = X;
            Gs = C;
            Bs = 0;
        }
    else if (H >= 120 && H < 180)
        {
            Rs = 0;
            Gs = C;
            Bs = X;
        }
    else if (H >= 180 && H < 240)
        {
            Rs = 0;
            Gs = X;
            Bs = C;
        }
    else if (H >= 240 && H < 300)
        {
            Rs = X;
            Gs = 0;
            Bs = C;
        }
    else
        {
            Rs = C;
            Gs = 0;
            Bs = X;
        }

    output[0] = (Rs + m) * 255;
    output[1] = (Gs + m) * 255;
    output[2] = (Bs + m) * 255;
}

/*******************************************************/
void
ColourRandomizer::DestroyInstance ()
{
    if (ColourRandomizer::mInstance)
        delete ColourRandomizer::mInstance;
}

/*******************************************************/
ColourRandomizer *
ColourRandomizer::GetInstance ()
{
    if (!ColourRandomizer::mInstance)
        {
            ColourRandomizer::mInstance = new ColourRandomizer ();
            atexit (&ColourRandomizer::DestroyInstance);
        }
    return ColourRandomizer::mInstance;
}

/*******************************************************/
void
RandomizeFadeColour ()
{
    injector::WriteMemory<CRGBA> (0xC3EFA8, GetRainbowColour ());
}

/*******************************************************/
CRGBA
GetRainbowColour (int offset)
{
    static bool hueCycle = ColourRandomizer::m_Config.RainbowHueCycle;

    int        colour[3];
    static int offsetForStatic = random (0, 10000);
    double     time
        = (!hueCycle) ? offsetForStatic : 1000.0 * clock () / CLOCKS_PER_SEC;

    HSVtoRGB (int (time / 10 + offset) % 360, 0.7, 0.7, colour);

    return {colour[0], colour[1], colour[2]};
}

int
HashColour (const CRGBA &colour)
{
    return (((colour.r + ColourRandomizer::HudRandomizerSeed) % 255) * 255
            + ((colour.g + ColourRandomizer::HudRandomizerSeed) % 255))
               * 255
           + ((colour.b + ColourRandomizer::HudRandomizerSeed) % 255);
}

/*******************************************************/
CRGBA *__fastcall RandomizeColours (CRGBA *thisCol, void *edx, uint8_t r,
                                    uint8_t g, uint8_t b, uint8_t a)
{
    static bool hueCycle  = ColourRandomizer::m_Config.RainbowHueCycle;
    static bool crazyMode = ColourRandomizer::m_Config.CrazyMode;

    static int offset = random (0, 10000);
    float      time = (!hueCycle) ? offset : 1000.0 * clock () / CLOCKS_PER_SEC;

    int colour[3];
    int hash = HashColour ({r, g, b, a});
    HSVtoRGB ((int) (time / 10 + hash) % 360, 0.7, 0.7, colour);

    bool randomizeColour = false;

    if (ColourRandomizer::m_Config.OldColourRandomization && r != g && g != b)
        randomizeColour = true;
    else if (!ColourRandomizer::m_Config.OldColourRandomization
             && ((r != 255 && g != 255 && b != 255)
                 && (r != 0 && g != 0 && b != 0)))
        randomizeColour = true;

    if (randomizeColour || crazyMode)
        {
            thisCol->r = colour[0];
            thisCol->g = colour[1];
            thisCol->b = colour[2];
            thisCol->a = a;
        }
    else
        {
            thisCol->r = r;
            thisCol->g = g;
            thisCol->b = b;
            thisCol->a = a;
        }
    return thisCol;
}

/*******************************************************/
CRGBA *__fastcall SkipRandomizeColours (CRGBA *thisCol, void *edx, uint8_t r,
                                        uint8_t g, uint8_t b, uint8_t a)
{
    thisCol->r = r;
    thisCol->g = g;
    thisCol->b = b;
    thisCol->a = a;

    return thisCol;
}

/*******************************************************/
void __fastcall RandomizeMarkers (C3dMarker *marker)
{
    auto colour      = marker->colour;
    marker->colour   = GetRainbowColour (HashColour (colour));
    marker->colour.a = colour.a;

    HookManager::CallOriginal<injector::thiscall<void (C3dMarker *)>,
                              0x7250B1> (marker);

    marker->colour = colour;
}

/*******************************************************/
void
RandomizeCoronaColours (int ID, CEntity *attachTo, char red, char green,
                        char blue, char alpha, CVector const &posn,
                        float radius, float farClip, int coronaType,
                        int flaretype, bool enableReflection,
                        bool checkObstacles, int _param_not_used, float angle,
                        bool longDistance, float nearClip, char fadeState,
                        float fadeSpeed, bool onlyFromBelow,
                        bool reflectionDelay)
{
    CRGBA colour = {red, green, blue, alpha};
    if (!ColourRandomizer::m_Config.ConsistentLights)
        {
            ColourRandomizer::Pattern currentID
                = {ID, {colour.r, colour.g, colour.b}};
            if (GetColour (currentID).ID == -1)
                {
                    currentID.colours[0] = random (255);
                    currentID.colours[1] = random (255);
                    currentID.colours[2] = random (255);
                    colourPatterns.push_back (currentID);
                }
            else
                {
                    colour.r = GetColour (currentID).colours[0];
                    colour.g = GetColour (currentID).colours[1];
                    colour.b = GetColour (currentID).colours[2];
                }
        }
    colour = GetRainbowColour (HashColour (colour));
    CCoronas::RegisterCorona (ID, attachTo, colour.r, colour.g, colour.b, alpha,
                              posn, radius, farClip, coronaType, flaretype,
                              enableReflection, checkObstacles, _param_not_used,
                              angle, longDistance, nearClip, fadeState,
                              fadeSpeed, onlyFromBelow, reflectionDelay);
}

/*******************************************************/
void
RandomizeLightColours (int ID, CEntity *attachTo, char red, char green,
                       char blue, char alpha, CVector const &posn, float radius,
                       float farClip, RwTexture *texture, int flaretype,
                       bool enableReflection, bool checkObstacles,
                       int _param_not_used, float angle, bool longDistance,
                       float nearClip, char fadeState, float fadeSpeed,
                       bool onlyFromBelow, bool reflectionDelay)
{
    CRGBA colour = {red, green, blue, alpha};
    if (!ColourRandomizer::m_Config.ConsistentLights)
        {
            ColourRandomizer::Pattern currentID
                = {ID, {colour.r, colour.g, colour.b}};
            if (GetColour (currentID).ID == -1)
                {
                    currentID.colours[0] = random (255);
                    currentID.colours[1] = random (255);
                    currentID.colours[2] = random (255);
                    colourPatterns.push_back (currentID);
                }
            else
                {
                    colour.r = GetColour (currentID).colours[0];
                    colour.g = GetColour (currentID).colours[1];
                    colour.b = GetColour (currentID).colours[2];
                }
        }
    colour = GetRainbowColour (HashColour (colour));
    CCoronas::RegisterCorona (ID, attachTo, colour.r, colour.g, colour.b, alpha,
                              posn, radius, farClip, texture, flaretype,
                              enableReflection, checkObstacles, _param_not_used,
                              angle, longDistance, nearClip, fadeState,
                              fadeSpeed, onlyFromBelow, reflectionDelay);
}

/*******************************************************/
template <int address>
void
RandomizeWeaponSpriteColours (float x, float y, float z, float halfw,
                              float halfh, char r, char g, char b, char a,
                              float rhw, char intensity, char udir, char vdir)
{
    CRGBA colour = {r, g, b, a};
    if (address == 0x58D8FD)
        {
            colour.r = weaponHUDSprite.colours[0];
            colour.g = weaponHUDSprite.colours[1];
            colour.b = weaponHUDSprite.colours[2];
        }
    else
        {
            static ColourRandomizer::Pattern weaponSpriteColours
                = {1, {random (255), random (255), b}};
            colour = {weaponSpriteColours.colours[0],
                      weaponSpriteColours.colours[1],
                      weaponSpriteColours.colours[2], a};
        }
    colour = GetRainbowColour (HashColour (colour));
    CSprite::RenderOneXLUSprite (x, y, z, halfw, halfh, colour.r, colour.g,
                                 colour.b, a, rhw, intensity, udir, vdir);
}

/*******************************************************/
template <int address>
void
RandomizeWeatherEffectColours (float x, float y, float z, float w, float h,
                               char r, char g, char b, char a, float recipNearZ,
                               char arg11)
{
    CRGBA colour = {r, g, b, a};
    if (address == 0x715EDF)
        {
            static ColourRandomizer::Pattern cloudColourHigh
                = {1, {random (255), g, b}};
            colour = {cloudColourHigh.colours[0], cloudColourHigh.colours[1],
                      cloudColourHigh.colours[2], a};
        }
    colour = GetRainbowColour (HashColour (colour));
    CSprite::RenderBufferedOneXLUSprite (x, y, z, w, h, colour.r, colour.g,
                                         colour.b, a, recipNearZ, arg11);
}

/*******************************************************/
template <int address>
void
RandomizeFogColours (float x, float y, float z, float w, float h, char r,
                     char g, char b, char a, float recipNearZ, float angle,
                     char arg12)
{
    CRGBA        colour  = {r, g, b, a};
    static CRGBA fogType = {r, g, b, -1};
    if (fogType.a == -1)
        {
            fogType = {random (255), g, b, a};
        }
    colour = fogType;
    colour = GetRainbowColour (HashColour (colour));
    CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect (x, y, z, w, h, colour.r,
                                                       colour.g, colour.b, a,
                                                       recipNearZ, angle,
                                                       arg12);
}

/*******************************************************/
template <int address>
void
RandomizeCloudColours (float x, float y, float z, float w, float h, char r,
                       char g, char b, char a, float recipNearZ, float angle,
                       char arg12)
{
    CRGBA        colour             = {r, g, b, a};
    static CRGBA cloudColourDistant = {-1, -1, -1, -1};
    if (cloudColourDistant.a == -1)
        cloudColourDistant = {r, g, b, a};
    colour = cloudColourDistant;
    colour = GetRainbowColour (HashColour (colour));
    CSprite::RenderBufferedOneXLUSprite_Rotate_Dimension (x, y, z, w, h,
                                                          colour.r, colour.g,
                                                          colour.b, a,
                                                          recipNearZ, angle,
                                                          arg12);
}

/*******************************************************/
void
RandomizeRpLights (RpLight *light, RwRGBAReal *colour)
{
    CRGBA colourNew = {(int) (colour->r * 255.0f), (int) (colour->g * 255.0f),
                       (int) (colour->b * 255.0f), (int) (colour->a * 255.0f)};
    colourNew       = GetRainbowColour (HashColour (colourNew));
    colour->r       = colourNew.r / 255.0f;
    colour->g       = colourNew.g / 255.0f;
    colour->b       = colourNew.b / 255.0f;
    RpLightSetColor (light, colour);
}

/*******************************************************/
template <int address>
CRGBA *__fastcall AddColourExceptions (CRGBA *thisCol, void *edx, uint8_t r,
                                       uint8_t g, uint8_t b, uint8_t a)
{
    CRGBA colour = {r, g, b, a};
    if (address == 0x58D978)
        {
            colour.r = weaponHUDSprite.colours[0];
            colour.g = weaponHUDSprite.colours[1];
            colour.b = weaponHUDSprite.colours[2];
        }
    colour     = GetRainbowColour (HashColour (colour));
    thisCol->r = colour.r;
    thisCol->g = colour.g;
    thisCol->b = colour.b;
    thisCol->a = a;
    return thisCol;
}

/*******************************************************/
void
RandomizeBlipColourID (int blip, int colourID)
{
    if (!CRunningScripts::CheckForRunningScript ("steal1"))
        colourID = random (0, 8);
    Call<0x583AB0> (blip, colourID); // CRadar::ChangeBlipColour
}

/*******************************************************/
void
ColourRandomizer::Initialise ()
{
    if (!ConfigManager::ReadConfig (
            "ColourRandomizer",
            std::pair ("RandomizeCarCols", &m_Config.RandomizeCarCols),
            std::pair ("ChangeCarColsOnFade", &m_Config.ChangeCarColsFade),
            std::pair ("RandomizeMarkers", &m_Config.RandomizeMarkers),
            std::pair ("RandomizeText", &m_Config.RandomizeText),
            std::pair ("RandomizeWeaponSprites",
                       &m_Config.RandomizeWeaponSprites),
            std::pair ("UseMinimalistTextRandomization",
                       &m_Config.OldColourRandomization),

            std::pair ("RandomizeLights", &m_Config.RandomizeLights),
            std::pair ("ConsistentLights", &m_Config.ConsistentLights),
            std::pair ("RandomizeClouds", &m_Config.RandomizeClouds),
            std::pair ("RandomizeStars", &m_Config.RandomizeStars),
            std::pair ("RandomizeRainbows", &m_Config.RandomizeRainbows),
            std::pair ("RandomizeFireLighting", &m_Config.RandomizeFireLight),
            std::pair ("ChangeOnFade", &m_Config.ChangeOnFade),

            std::pair ("RainbowHueCycle", &m_Config.RainbowHueCycle),
            std::pair ("RandomizeFades", &m_Config.RandomizeFades),
            std::pair ("CrazyMode", &m_Config.CrazyMode)))
        return;

    ColourRandomizer::HudRandomizerSeed = random (INT_MAX);
    weaponHUDSprite = {1, {random (255), random (255), random (255)}};

    if (m_Config.RandomizeCarCols)
        {
            RegisterHooks (
                {{HOOK_CALL, 0x5B8F17, (void *) &RandomizeColourTables<true>},
                 {HOOK_JUMP, 0x4C8500, (void *) &RandomizeVehicleColour},
                 {HOOK_CALL, 0x47EA9B, (void *) &RandomizeScriptVehicleColours},
                 {HOOK_CALL, 0x47B8FE,
                  (void *) &RandomizeScriptVehicleColours}});

            if (m_Config.ChangeCarColsFade)
                {
                    FadesManager::AddFadeCallback (
                        [] { RandomizeColourTables<false> (); });
                }
        }

    if (m_Config.RandomizeText)
        {
            RegisterHooks (
                {{HOOK_JUMP, 0x7170C0, (void *) &RandomizeColours},
                 {HOOK_CALL, 0x728788, (void *) &SkipRandomizeColours},
                 {HOOK_CALL, 0x721CE0, (void *) &SkipRandomizeColours}});

            if (m_Config.RandomizeWeaponSprites
                && !m_Config.OldColourRandomization)
                {
                    injector::MakeCALL (0x58D8FD,
                                        (void *) &RandomizeWeaponSpriteColours<
                                            0x58D8FD>); // Weapon Icon
                    injector::MakeCALL (
                        0x58D978,
                        (void *) &AddColourExceptions<0x58D978>); // Fist Sprite
                }

            injector::MakeCALL (
                0x586850,
                (void *) &AddColourExceptions<0x586850>); // Gang Territories

            for (int address :
                 {0x444F5F, 0x44535A, 0x445840, 0x47C698, 0x60CB72, 0x73878A})
                {
                    injector::MakeCALL (address,
                                        (void *) &RandomizeBlipColourID);
                }

            injector::MakeCALL (0x58E95B,
                                (void *) &RandomizeWeaponSpriteColours<
                                    0x58E95B>); // FPS Crosshair 1
            injector::MakeCALL (0x58E9CA,
                                (void *) &RandomizeWeaponSpriteColours<
                                    0x58E9CA>); // FPS Crosshair 2
            injector::MakeCALL (0x58EA39,
                                (void *) &RandomizeWeaponSpriteColours<
                                    0x58EA39>); // FPS Crosshair 3
            injector::MakeCALL (0x58EAA8,
                                (void *) &RandomizeWeaponSpriteColours<
                                    0x58EAA8>); // FPS Crosshair 4
        }

    if (m_Config.RandomizeMarkers)
        {
            RegisterHooks (
                {{HOOK_CALL, 0x7250B1, (void *) &RandomizeMarkers},
                 {HOOK_CALL, 0x725D70,
                  (void *) &RandomizeWeaponSpriteColours<0x725D70>}}); // Checkpoint
                                                                       // Arrow
        }

    if (m_Config.RandomizeLights)
        {
            for (int address :
                 {0x45584E, 0x455F59, 0x456219, 0x47F18E, 0x48BFB4, 0x49DD4C,
                  0x6ABA60, 0x6ABB35, 0x6ABC69, 0x6BD4DD, 0x6BD531, 0x6C5B1F,
                  0x6DDF28, 0x6E0D01, 0x6E0DF7, 0x6E1A2D, 0x6E219F, 0x6E2297,
                  0x6E253E, 0x6E2633, 0x7175C3, 0x7176BA, 0x7177B1, 0x73AAA2})
                {
                    injector::MakeCALL (address,
                                        (void *) &RandomizeCoronaColours);
                }

            for (int address :
                 {0x5363A8, 0x536511, 0x6FC6B1, 0x6FC73D, 0x6FCA80, 0x6FD02E})
                {
                    injector::MakeCALL (address,
                                        (void *) &RandomizeLightColours);
                }
        }

    if (m_Config.RandomizeFireLight)
        {
            for (int address : {0x53B7E0, 0x53B893, 0x53B99C, 0x53BAA5})
                {
                    injector::MakeCALL (address,
                                        (void *) &RandomizeCoronaColours);
                }
        }

    if (m_Config.RandomizeStars)
        {
            // injector::MakeCALL (0x6FB9A6, (void *)
            // &RandomizeWeatherEffectColours<0x6FB9A6>); // Wet Roads
            injector::MakeCALL (
                0x713F0F,
                (void *) &RandomizeWeatherEffectColours<0x713F0F>); // Stars
        }

    if (m_Config.RandomizeRainbows)
        {
            injector::MakeCALL (
                0x714371,
                (void *) &RandomizeWeatherEffectColours<0x714371>); // Rainbows
        }

    if (m_Config.RandomizeClouds)
        {
            injector::MakeCALL (0x715EDF,
                                (void *) &RandomizeWeatherEffectColours<
                                    0x715EDF>); // High Clouds
            injector::MakeCALL (0x700817,
                                (void *) &RandomizeFogColours<0x700817>);
            injector::MakeCALL (0x700B6B,
                                (void *) &RandomizeFogColours<0x700B6B>);
            injector::MakeCALL (
                0x714216,
                (void *) &RandomizeCloudColours<0x714216>); // Distant Clouds
            injector::MakeCALL (0x714011,
                                (void *) &RandomizeWeaponSpriteColours<
                                    0x714011>); // Cloud-related
        }

    // Basically the equivalent of discount TimeCycle Randomizer, might add as
    // optional setting
    // for (int address : {0x5BA566, 0x5BA59E, 0x5BA64D, 0x7064A7, 0x7355D4,
    // 0x735613,
    //    0x735803, 0x73582B, 0x735971, 0x735A8B, 0x735A9C, 0x735B05, 0x735B16,
    //    0x735B95, 0x735BA6, 0x735C17, 0x735C28, 0x735C4B, 0x735C5C, 0x735CF8,
    //    0x735D1B, 0x735D3B, 0x735D5C, 0x735D7C, 0x735E21, 0x735E32, 0x735E4B,
    //    0x735E5C, 0x735ED6, 0x735EE6, 0x735EFB, 0x735F0C, 0x735F4F, 0x735F5F,
    //    0x735F9F, 0x735FAF})
    //{
    //    injector::MakeCALL (address, (void *) &RandomizeRpLights);
    //}

    if (m_Config.RandomizeFades)
        {
            RegisterHooks (
                {{HOOK_CALL, 0x50BF22, (void *) &RandomizeFadeColour}});
            injector::MakeRangedNOP (0x50BF22 + 5, 0x50BF33);
        }

    if (m_Config.ChangeOnFade)
        {
            FadesManager::AddFadeCallback (
                [] { ColourRandomizer::HudRandomizerSeed = random (INT_MAX); });
        }

    Logger::GetLogger ()->LogMessage ("Initialised ColourRandomizer");
}

///*******************************************************/
// template <bool callOriginal>
// static int
// RandomizeVehicleColours ()
//{
//    int ret = 0;
//    if constexpr (callOriginal)
//        ret = CModelInfo::LoadVehicleColours ();
//
//    const int COLOUR_TABLE_LENGTH = 128;
//    for (int i = 0; i < COLOUR_TABLE_LENGTH; i++)
//        {
//            auto &colour    = ms_vehicleColourTable[i];
//            auto  newColour = HSL (randomFloat (0.0f, 360.0f),
//                                               1.0f, randomFloat (0.0f, 1.0f))
//                                 .ToRGBA ();
//
//            colour = {newColour.r, newColour.g, newColour.b, colour.a};
//        }
//
//    return ret;
//}

void __fastcall RandomizeVehicleColour (void *info, void *edx, uint8_t *prim,
                                        uint8_t *secn, uint8_t *tert,
                                        uint8_t *quat, int variation)
{
    *prim = random (127);
    *secn = random (127);
    *tert = random (127);
    *quat = random (127);
}

/*******************************************************/
void __fastcall RandomizeScriptVehicleColours (CRunningScript *scr, void *edx,
                                               short count)
{
    scr->CollectParameters (count);
    if (!scr->CheckName ("scrash3"))
        {
            ScriptParams[1] = random (127);
            ScriptParams[2] = random (127);
        }
}

/*******************************************************/
template <bool callOriginal>
int
RandomizeColourTables ()
{
    int ret = 0;
    if constexpr (callOriginal)
        ret = CModelInfo::LoadVehicleColours ();

    for (int i = 0; i < 128; i++)
        {
            int colour[] = {0, 0, 0};
            HSVtoRGB ((int) (i * 2.8125), random (50, 100) / 100.0,
                      random (50, 100) / 100.0, colour);

            ms_vehicleColourTable[i].r = (uint8_t) colour[0];
            ms_vehicleColourTable[i].g = (uint8_t) colour[1];
            ms_vehicleColourTable[i].b = (uint8_t) colour[2];
        }
    return ret;
}

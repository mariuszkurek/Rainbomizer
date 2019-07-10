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

#pragma once

/* These contain game functions that might be called by other modules */

class cVehicleParams;
class CBaseModelInfo;
class CClumpModelInfo;
class CVector;
class CBox;
class CColModel;

enum eVehicleClass
{
    VEHICLE_AUTOMOBILE = 0,
    VEHICLE_MTRUCK,
    VEHICLE_QUAD,
    VEHICLE_HELI,
    VEHICLE_PLANE,
    VEHICLE_BOAT,
    VEHICLE_TRAIN,
    VEHICLE_FHELI,
    VEHICLE_FPLANE,
    VEHICLE_BIKE,
    VEHICLE_BMX,
    VEHICLE_TRAILER,
};

class cVehicleParams
{
public:
    unsigned int   m_nVehicleSubclass;
    unsigned int   m_nVehicleClass;
    char           field_8;
    char           pad1[3];
    int            field_C;
    void *         m_pVehicle;
    void *         m_pTransmission;
    unsigned int   m_nModelType;
    float          m_fVelocity;
    short          m_nGasState;
    short          m_nBreakState;
    float          m_fVelocityAbsolute;
    float          m_fZVelocity;
    float          m_fVelocityPercentage;
    int            field_30;
    float          field_34;
    char           m_nCurrentGear;
    bool           m_bHandbrakeOn;
    char           pad2[2];
    float          m_fVelocityChangingPercentage;
    float          m_fWheelSpinForAudio;
    unsigned short m_nNumberOfGears;
    unsigned char  m_nWheelsOnGround;
    char           field_47;
    int            field_48;
};

struct CStreaming
{
    static int  GetDefaultCopCarModel (int a1);
    static void RequestModel (int model, int flags);
    static void LoadAllRequestedModels (bool bOnlyPriority);
    static void RemoveModel (int model);
    static void SetMissionDoesntRequireModel (int index);
};

struct CCarGenerator
{
    void CheckForBlockage (int model_id);
};

struct CCarCtrl
{
    static void *CreateCarForScript (int modelId, float X, float Y, float Z,
                                     char a5);
};

struct CRunningScript
{
    void CollectParameters (short num);
};

struct CAudioEngine
{
	void PreloadMissionAudio(unsigned char slot, int id);
};

struct CText
{
	char* Get(char* key);
};

struct CPool
{
    void **      m_pObjects;
    static void *GetAtVehicle (signed int handle);
};

struct CPad
{
    static void DoCheats ();
};

struct CVehicleRecording
{
    static void SetPlaybackSpeed (void *a1, float a2);
};

struct CPopulation
{
    static void *AddPed (int type, int model, float *posn, bool unk);
};

struct CCarAI
{
    static void AddPoliceCarOccupants (void *vehicle, char a3);
};

struct CModelInfo
{
    static bool IsBoatModel (int modelId);
    static bool IsCarModel (int modelId);
    static bool IsTrainModel (int modelId);
    static bool IsHeliModel (int modelId);
    static bool IsPlaneModel (int modelId);
    static bool IsBikeModel (int modelId);
    static bool IsFakePlaneModel (int modelId);
    static bool IsMonsterTruckModel (int modelId);
    static bool IsQuadBikeModel (int modelId);
    static bool IsBmxModel (int modelId);
    static bool IsTrailerModel (int modelId);
    static bool IsPoliceModel (int modelId);
    static int  LoadVehicleColours ();
    static int  GetMaximumNumberOfPassengersFromNumberOfDoors (int modelIndex);
};

struct CAEVehicleAudioEntity
{
    void ProcessTrainTrackSound (void *vehicle_info);
};

struct CVector
{
    float x;
    float y;
    float z;
};

class CBox
{
public:
    CVector m_vecMin;
    CVector m_vecMax;
};

struct CColModel
{
    CBox m_boundBox;
};

struct CVehicle
{
    int GetVehicleAppearence ();
};

struct CBaseModelInfo
{
public:
    char             pad[0x14];
    CColModel *      m_pColModel;
    float            m_fDrawDistance;
    struct RwObject *m_pRwObject;
};

class CClumpModelInfo : public CBaseModelInfo
{
public:
    CBox *GetBoundingBox ();
};

struct CStreamingInfo
{
public:
    short         m_nNextIndex; // ms_pArrayBase array index
    short         m_nPrevIndex; // ms_pArrayBase array index
    short         m_nNextIndexOnCd;
    unsigned char m_nFlags; // see eStreamingFlags
    unsigned char m_nImgId;
    unsigned int  m_nCdPosn;
    unsigned int  m_nCdSize;
    unsigned char m_nLoadState; // see eStreamingLoadState
private:
    char __pad[3];
};

struct RwRGBA
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct CMatrix
{
    CVector      right;
    unsigned int flags;
    CVector      up;
    unsigned int pad1;
    CVector      at;
    unsigned int pad2;
    CVector      pos;
    unsigned int pad3;

    void Attach (CMatrix *attach, char link);
};

CMatrix *RwFrameGetLTM (void *frame);

int random (int max, int min = 0);

extern CStreamingInfo * ms_aInfoForModel;
extern CBaseModelInfo **ms_modelInfoPtrs;
extern RwRGBA *         ms_vehicleColourTable;

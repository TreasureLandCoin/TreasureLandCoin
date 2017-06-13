// Copyright (c) 2014-2017 Coin Sciences Ltd
// TreasureLandCoin code distributed under the GPLv3 license, see COPYING file.

#ifndef TREASURELANDCOINPARAMS_H
#define	TREASURELANDCOINPARAMS_H

#include "utils/declare.h"

#define MC_DEFAULT_NETWORK_PORT 8571
#define MC_DEFAULT_RPC_PORT 8570


#define MC_PRM_MAX_PARAM_NAME_SIZE       31
#define MC_PRM_MAX_ARG_NAME_SIZE         31
#define MC_PRM_PARAM_SIZE_BYTES           2
#define MC_PRM_MAX_DESCRIPTION_SIZE     255
#define MC_PRM_DECIMAL_GRANULARITY 1000000

#define MC_PRM_UNKNOWN          0x00000000
#define MC_PRM_BINARY           0x00000001
#define MC_PRM_STRING           0x00000002
#define MC_PRM_BOOLEAN          0x00000003
#define MC_PRM_INT32            0x00000004
#define MC_PRM_INT64            0x00000005
#define MC_PRM_DOUBLE           0x00000006
#define MC_PRM_UINT32           0x00000007
#define MC_PRM_DATA_TYPE_MASK   0x0000000F

#define MC_PRM_COMMENT          0x00000010
#define MC_PRM_USER             0x00000020
#define MC_PRM_GENERATED        0x00000030
#define MC_PRM_CALCULATED       0x00000040
#define MC_PRM_SOURCE_MASK      0x000000F0

#define MC_PRM_CLONE            0x00010000
#define MC_PRM_SPECIAL          0x00020000
#define MC_PRM_NOHASH           0x00040000
#define MC_PRM_MINIMAL          0x00080000
#define MC_PRM_DECIMAL          0x00100000


#define MC_PRM_STATUS_EMPTY              0
#define MC_PRM_STATUS_MINIMAL            1
#define MC_PRM_STATUS_ERROR              2
#define MC_PRM_STATUS_GENERATED          3
#define MC_PRM_STATUS_INVALID            4
#define MC_PRM_STATUS_VALID              5


typedef struct mc_OneTreasurelandcoinParam
{    
    char m_Name[MC_PRM_MAX_ARG_NAME_SIZE+1]; 
    char m_DisplayName[MC_PRM_MAX_ARG_NAME_SIZE+1]; 
    int m_Type;
    int m_MaxStringSize;
    int64_t m_DefaultIntegerValue;
    int64_t m_MinIntegerValue;
    int64_t m_MaxIntegerValue;
    double m_DefaultDoubleValue;
    int m_ProtocolVersion;
    int m_Removed;
    char m_ArgName[MC_PRM_MAX_PARAM_NAME_SIZE+1]; 
    char m_Next[MC_PRM_MAX_ARG_NAME_SIZE+1]; 
    char m_Group[MC_PRM_MAX_DESCRIPTION_SIZE+1]; 
    char m_Description[MC_PRM_MAX_DESCRIPTION_SIZE+1]; 
    
    int IsRelevant(int version);
} mc_OneTreasurelandcoinParam;

typedef struct mc_TreasurelandcoinParams
{    
    char *m_lpData;
    mc_MapStringIndex *m_lpIndex;
    mc_OneTreasurelandcoinParam *m_lpParams;
    int *m_lpCoord;
    int m_Status;
    int m_Size;
    int m_Count;
    int m_IsProtocolTreasureLandCoin;
    int m_ProtocolVersion;
    
    int m_AssetRefSize;
    
    mc_TreasurelandcoinParams()
    {
        Zero();
    }

    ~mc_TreasurelandcoinParams()
    {
        Destroy();
    }
    
    void Zero();                                                                // Initializes parameters set object
    void Init();                                                                // Initializes parameters set object
    void Destroy();                                                             // Destroys parameters set object
    
    int Create(const char *name,int version);
    int Read(const char *name);    
    int Read(const char* name,int argc, char* argv[],int create_version);
    int Clone(const char *name,mc_TreasurelandcoinParams *source);
    int Build(const unsigned char* pubkey,int pubkey_size);
    int Validate();
    int CalculateHash(unsigned char *hash);
    int Write(int overwrite);
    int Print(FILE *);
    int SetGlobals();
    int Import(const char *name,const char *source_address);
    int Set(const char *name,const char *source,int source_size);
    
    int FindParam(const char *param);
    void* GetParam(const char *param,int* size);
    int64_t GetInt64Param(const char *param);
    double GetDoubleParam(const char *param);
    
    int SetParam(const char *param,const char* value,int size);
    int SetParam(const char *param,int64_t value);
    
    
    const char* Name();
    const unsigned char* DefaultMessageStart();
    const unsigned char* MessageStart();
    const unsigned char* AddressVersion();
    const unsigned char* AddressCheckumValue();
    const unsigned char* AddressScriptVersion();
    int ProtocolVersion();
    int IsProtocolTreasurelandcoin();
    double ParamAccuracy();
    

    
} mc_TreasurelandcoinParams;


#endif	/* TREASURELANDCOINPARAMS_H */


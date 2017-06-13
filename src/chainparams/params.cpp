// Copyright (c) 2014-2017 Coin Sciences Ltd
// TreasureLandCoin code distributed under the GPLv3 license, see COPYING file.

#include "treasurelandcoin/treasurelandcoin.h"


#define MC_PRM_DAT_FILE_LINE_SIZE 39

const uint32_t FreePortRangesOver50[]={2644,2744,2870,4244,4324,4374,4754,5744,6264,6446,
                                       6716,6790,7172,7314,7404,7718,8338,9218,9538,9696};

const unsigned char c_DefaultMessageStart[4]={0xfb,0xb4,0xc7,0xde};

#include "chainparams/paramlist.h"


int mc_OneTreasurelandcoinParam::IsRelevant(int version)
{
    int ret=1;
    
    if(m_ProtocolVersion > version)
    {
        ret=0;
    }
    
    if(m_Removed > 0)
    {
        if(m_Removed <= version)
        {
            ret=0;
        }
    }
    
    return ret;
}

void mc_TreasurelandcoinParams::Zero()
{
    m_lpData = NULL;
    m_lpParams = NULL;        
    m_lpIndex=NULL;    
    m_lpCoord=NULL;
    m_Status=MC_PRM_STATUS_EMPTY;
    m_Size=0;
    m_IsProtocolTreasureLandCoin=1;
    m_ProtocolVersion=0;
    
    m_AssetRefSize=MC_AST_SHORT_TXID_SIZE;
}

void mc_TreasurelandcoinParams::Destroy()
{
    if(m_lpData)
    {
        mc_Delete(m_lpData);
        m_lpData = NULL;
    }
    if(m_lpParams)
    {
        mc_Delete(m_lpParams);
        m_lpParams = NULL;        
    }
    if(m_lpCoord)
    {
        mc_Delete(m_lpCoord);
        m_lpCoord = NULL;        
    }
    if(m_lpIndex)
    {
        delete m_lpIndex;
        m_lpIndex=NULL;
    }    
}


int64_t mc_TreasurelandcoinParams::GetInt64Param(const char *param)
{
    int size;
    void* ptr=GetParam(param,&size);
    if(ptr == NULL)
    {
        if(m_lpIndex == NULL)
        {
            return -1;
        }
        int index=m_lpIndex->Get(param);
        if(index<0)
        {
            printf("Parameter not found: %s\n",param);
            return -1;
        }   
        
        return (m_lpParams+index)->m_DefaultIntegerValue;
    }
   
    return mc_GetLE(ptr,size);
}

double mc_TreasurelandcoinParams::GetDoubleParam(const char *param)
{
    int n=(int)mc_gState->m_NetworkParams->GetInt64Param(param);
    if(n < 0)
    {
        return -((double)(-n) / MC_PRM_DECIMAL_GRANULARITY);
    }
    return (double)n / MC_PRM_DECIMAL_GRANULARITY;    
}


void* mc_TreasurelandcoinParams::GetParam(const char *param,int* size)
{
    if(m_lpIndex == NULL)
    {
        return NULL;
    }
    int index=m_lpIndex->Get(param);
    if(index<0)
    {
        return NULL;
    }   
    int offset=m_lpCoord[2 * index + 0];
    if(offset<0)
    {
        return NULL;
    }   
    if(size)
    {
        *size=m_lpCoord[2 * index + 1];
    }
    
    return m_lpData+offset;
}


int mc_TreasurelandcoinParams::SetParam(const char *param,const char* value,int size)
{
    int offset;
    if(m_lpIndex == NULL)
    {
        return MC_ERR_INTERNAL_ERROR;
    }
    int index=m_lpIndex->Get(param);
    if(index<0)
    {
        return MC_ERR_INTERNAL_ERROR;
    }   
    
    switch((m_lpParams+index)->m_Type & MC_PRM_SOURCE_MASK)
    {
        case MC_PRM_COMMENT:
        case MC_PRM_CALCULATED:
            break;
        default:
            return MC_ERR_INTERNAL_ERROR;
    }
    
    if(m_lpCoord[2 * index + 0] >= 0)
    {
        return MC_ERR_INTERNAL_ERROR;        
    }
    
    offset=m_Size;
    
    strcpy(m_lpData+offset,param);
    offset+=strlen(param)+1;

    if(size)
    {
        memcpy(m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES,value,size);
    }
    
    mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
    offset+=MC_PRM_PARAM_SIZE_BYTES;
    m_lpCoord[2 * index + 0]=offset;
    m_lpCoord[2 * index + 1]=size;
    offset+=size;                        
    
    m_Size=offset;
    
    return MC_ERR_NOERROR;    
}

int mc_TreasurelandcoinParams::SetParam(const char *param,int64_t value)
{
    int size;
    char buf[8];
    if(m_lpIndex == NULL)
    {
        return MC_ERR_INTERNAL_ERROR;
    }
    int index=m_lpIndex->Get(param);
    if(index<0)
    {
        return MC_ERR_INTERNAL_ERROR;
    }   
    
    size=0;
    switch((m_lpParams+index)->m_Type & MC_PRM_DATA_TYPE_MASK)
    {
        case MC_PRM_BOOLEAN:
            size=1;
            break;
        case MC_PRM_INT32:
        case MC_PRM_UINT32:            
            size=4;
            break;
        case MC_PRM_INT64:    
            size=8;
            break;
        default:
            return MC_ERR_INTERNAL_ERROR;
    }

    mc_PutLE(buf,&value,size);
    return SetParam(param,buf,size);
}


void mc_TreasurelandcoinParams::Init()
{
    int size,max_size,i;
    
    Destroy();
    
    m_lpIndex=new mc_MapStringIndex;            
    
    m_Count=sizeof(TreasurelandcoinParamArray)/sizeof(mc_OneTreasurelandcoinParam);
    max_size=0;
    
    for(i=0;i<m_Count;i++)
    {
        size=0;
        switch((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
        {
            case MC_PRM_BINARY  : size=(TreasurelandcoinParamArray+i)->m_MaxStringSize;   break;
            case MC_PRM_STRING  : size=(TreasurelandcoinParamArray+i)->m_MaxStringSize+1; break;
            case MC_PRM_BOOLEAN : size=1;                                           break;
            case MC_PRM_INT32   : size=4;                                           break;
            case MC_PRM_INT64   : size=8;                                           break;
            case MC_PRM_DOUBLE  : size=sizeof(double);                              break;
            case MC_PRM_UINT32  : size=4;                                           break;
        }
                
        max_size+=MC_PRM_MAX_PARAM_NAME_SIZE+1+MC_PRM_PARAM_SIZE_BYTES+size;
    }
    
    m_lpParams=(mc_OneTreasurelandcoinParam*)mc_New(sizeof(TreasurelandcoinParamArray));
    m_lpData=(char*)mc_New(max_size);
    m_lpCoord=(int*)mc_New(2*m_Count*sizeof(int));
    
    memcpy(m_lpParams,TreasurelandcoinParamArray,sizeof(TreasurelandcoinParamArray));
    for(i=0;i<m_Count;i++)
    {
        m_lpIndex->Add((m_lpParams+i)->m_Name,i);
        m_lpCoord[2 * i + 0]=-1;
    }    
    m_Size=0;
    
}

int mc_TreasurelandcoinParams::Create(const char* name,int version)
{
    int size,offset,i,set;
    mc_OneTreasurelandcoinParam *param;
    char *ptrData;
    int num_sets;
    uint32_t network_port=MC_DEFAULT_NETWORK_PORT;
    uint32_t rpc_port=MC_DEFAULT_RPC_PORT;
    
    int param_sets[]={MC_PRM_COMMENT, MC_PRM_USER, MC_PRM_GENERATED};
    num_sets=sizeof(param_sets)/sizeof(int);    
    
    Init();
 
    mc_RandomSeed(mc_TimeNowAsUInt());
    
    offset=0;

    for(set=0;set<num_sets;set++)
    {    
        param=m_lpParams;
        for(i=0;i<m_Count;i++)
        {
            if(((m_lpParams+i)->m_Type & MC_PRM_SOURCE_MASK) == param_sets[set])
            {                                                                
                strcpy(m_lpData+offset,param->m_Name);
                offset+=strlen(param->m_Name)+1;
                size=0;

                ptrData=m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES;

                switch(param->m_Type & MC_PRM_SOURCE_MASK)
                {
                    case MC_PRM_COMMENT:
                    case MC_PRM_USER:
                        switch((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
                        {
                            case MC_PRM_BINARY  : 
                                size=0;   
                                break;
                            case MC_PRM_STRING  : 
                                size=1;   
                                if((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SPECIAL)
                                {
                                    if(strcmp(param->m_Name,"chaindescription") == 0)
                                    {
                                        if(strlen(name)+19<=(size_t)(param->m_MaxStringSize))
                                        {
                                            sprintf(ptrData,"TreasureLandCoin %s",name);
                                        }
                                        size=strlen(ptrData)+1;
                                    }                                   
                                    if(strcmp(param->m_Name,"rootstreamname") == 0)
                                    {
                                        sprintf(ptrData,"root");
                                        size=strlen(ptrData)+1;
                                    }                                   
                                    if(strcmp(param->m_Name,"seednode") == 0)
                                    {
                                        size=1;
                                    }                                   
                                    if(strcmp(param->m_Name,"chainprotocol") == 0)
                                    {
                                        sprintf(ptrData,"treasurelandcoin");
                                        size=strlen(ptrData)+1;
                                    }                                   
                                }
                                break;
                            case MC_PRM_BOOLEAN:
                                size=1;
                                ptrData[0]=0;
                                if(param->m_DefaultIntegerValue)
                                {
                                    ptrData[0]=1;                            
                                }
                                break;
                            case MC_PRM_INT32:
                            case MC_PRM_UINT32:
                                size=4;
                                mc_PutLE(ptrData,&(param->m_DefaultIntegerValue),4);
                                break;
                            case MC_PRM_INT64:
                                size=8;
                                mc_PutLE(ptrData,&(param->m_DefaultIntegerValue),8);
                                break;
                            case MC_PRM_DOUBLE:
                                size=8;
                                *((double*)ptrData)=param->m_DefaultDoubleValue;
                                break;
                        }            
                        break;
                    case MC_PRM_GENERATED:
                        if(strcmp(param->m_Name,"defaultnetworkport") == 0)
                        {
                            size=4;
                            network_port=mc_RandomInRange(0,sizeof(FreePortRangesOver50)/sizeof(uint32_t)-1);
                            network_port=FreePortRangesOver50[network_port];
                            network_port+=1+2*mc_RandomInRange(0,24);
                            mc_PutLE(ptrData,&network_port,4);
                        }
                        if(strcmp(param->m_Name,"defaultrpcport") == 0)
                        {
                            size=4;
                            rpc_port=network_port-1;
                            mc_PutLE(ptrData,&rpc_port,4);
                        }
                        if(strcmp(param->m_Name,"protocolversion") == 0)
                        {
                            size=4;
                            mc_PutLE(ptrData,&version,4);
                        }
                        if(strcmp(param->m_Name,"chainname") == 0)
                        {
                            if(strlen(name)>(size_t)(param->m_MaxStringSize))
                            {
                                mc_print("Invalid network name - too long");
                                return MC_ERR_INVALID_PARAMETER_VALUE;
                            }
                            size=strlen(name)+1;
                            strcpy(ptrData,name);
                        }
                        if(strcmp(param->m_Name,"networkmessagestart") == 0)
                        {
                            size=4;
                            memcpy(ptrData,c_DefaultMessageStart,4);
                            while(memcmp(ptrData,c_DefaultMessageStart,4) == 0)
                            {
                                *((unsigned char*)ptrData+0)=mc_RandomInRange(0xf0,0xff);
                                *((unsigned char*)ptrData+1)=mc_RandomInRange(0xc0,0xff);
                                *((unsigned char*)ptrData+2)=mc_RandomInRange(0xc0,0xff);
                                *((unsigned char*)ptrData+3)=mc_RandomInRange(0xe0,0xff);
                            }
                        }
                        if(strcmp(param->m_Name,"addresspubkeyhashversion") == 0)
                        {
                            size=4;
                            *((unsigned char*)ptrData+0)=0x00;
                            *((unsigned char*)ptrData+1)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+2)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+3)=mc_RandomInRange(0x00,0xff);
                        }
                        if(strcmp(param->m_Name,"addressscripthashversion") == 0)
                        {
                            size=4;
                            *((unsigned char*)ptrData+0)=0x05;
                            *((unsigned char*)ptrData+1)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+2)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+3)=mc_RandomInRange(0x00,0xff);
                        }
                        if(strcmp(param->m_Name,"privatekeyversion") == 0)
                        {
                            size=4;
                            *((unsigned char*)ptrData+0)=0x80;
                            *((unsigned char*)ptrData+1)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+2)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+3)=mc_RandomInRange(0x00,0xff);
                        }
                        if(strcmp(param->m_Name,"addresschecksumvalue") == 0)
                        {
                            size=4;
                            *((unsigned char*)ptrData+0)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+1)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+2)=mc_RandomInRange(0x00,0xff);
                            *((unsigned char*)ptrData+3)=mc_RandomInRange(0x00,0xff);
                        }
                        break;
                }

                mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
                offset+=MC_PRM_PARAM_SIZE_BYTES;
                m_lpCoord[2 * i + 0]=offset;
                m_lpCoord[2 * i + 1]=size;
                offset+=size;            
            }
            param++;
        }
        
    }
        
    m_Size=offset;
    
    return MC_ERR_NOERROR;
}

double mc_TreasurelandcoinParams::ParamAccuracy()
{
    return 1./(double)MC_PRM_DECIMAL_GRANULARITY;
}


int mc_TreasurelandcoinParams::Read(const char* name)
{
    return Read(name,0,NULL,0);
}

int mc_TreasurelandcoinParams::Read(const char* name,int argc, char* argv[],int create_version)
{
    mc_MapStringString *mapConfig;
    int err;
    int size,offset,i,version,len0,len1,len2;
    char empty_string[1];
    mc_OneTreasurelandcoinParam *param;
    char *ptrData;
    const char *ptr;
    empty_string[0]=0x00;    
    
    if(name == NULL)
    {
        return MC_ERR_INTERNAL_ERROR;
    }
    
    err=MC_ERR_NOERROR;
    offset=0;
    
    mc_TreasurelandcoinParams *lpDefaultParams;
    
    lpDefaultParams=NULL;
    mapConfig=new mc_MapStringString;
    if(argc)
    {
        err=mc_ReadParamArgs(mapConfig,argc,argv,"");
    }
    else
    {
        err=mc_ReadGeneralConfigFile(mapConfig,name,"params",".dat");
    }

    if(err)
    {
        goto exitlbl;
    }    
    
    Init();

    version=0;
    if(mapConfig->Get("protocolversion") != NULL)
    {
        version=atoi(mapConfig->Get("protocolversion"));
    }
    if(create_version)
    {
        version=create_version;
    }
    
    if(version == 0)
    {
        version=mc_gState->GetProtocolVersion();
    }

    if(argc == 0)
    {
        if(mapConfig->Get("chainname") == NULL)
        {
            err=MC_ERR_NOERROR;
            goto exitlbl;
        }


        if(strcmp(name,mapConfig->Get("chainname")) != 0)
        {
            printf("chain-name parameter (%s) doesn't match <network-name> (%s)\n",mapConfig->Get("chainname"),name);
            err=MC_ERR_INVALID_PARAMETER_VALUE;                        
            goto exitlbl;
        }
    }

    lpDefaultParams = new mc_TreasurelandcoinParams;
    
    err=lpDefaultParams->Create(name,version);
 
    if(err)
    {
        goto exitlbl;
    }
    

    param=m_lpParams;
    for(i=0;i<m_Count;i++)
    {
        ptr=mapConfig->Get(param->m_Name);
        if(ptr)
        {
            if(strcmp(ptr,"[null]") == 0)
            {
                ptr=NULL;;
            }
        }        
/*
        if(argc == 0)
        {
            if((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SPECIAL)
            {
                if(strcmp((TreasurelandcoinParamArray+i)->m_Name,"rootstreamname") == 0)
                {
                    if(ptr == NULL)
                    {
                        ptr=(const char*)&empty_string;
                    }
                }                                               
            }
        }
*/
        if(ptr)
        {        
            strcpy(m_lpData+offset,param->m_Name);
            offset+=strlen(param->m_Name)+1;
            size=0;

            ptrData=m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES;
            
            switch((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
            {
                case MC_PRM_BINARY  : 
                    if(strlen(ptr) % 2)
                    {
                        printf("Invalid parameter value for %s - odd length: %d\n",param->m_DisplayName,(int)strlen(ptr));
                        return MC_ERR_INVALID_PARAMETER_VALUE;                        
                    }
                    size=strlen(ptr) / 2;   
                    if(size > param->m_MaxStringSize)
                    {
                        printf("Invalid parameter value for %s - too long: %d\n",param->m_DisplayName,(int)strlen(ptr));
                        return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                    }
                    if(mc_HexToBin(ptrData,ptr,size) != size)
                    {
                        printf("Invalid parameter value for %s - cannot parse hex string: %s\n",param->m_DisplayName,ptr);
                        return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                    }
                    break;
                case MC_PRM_STRING  : 
                    size=strlen(ptr);
                    if(size > param->m_MaxStringSize)
                    {
                        printf("Invalid parameter value for %s - too long: %d\n",param->m_DisplayName,(int)strlen(ptr));
                        return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                    }
                    strcpy(ptrData,ptr);
                    size++;
                    break;
                case MC_PRM_BOOLEAN:
                    ptrData[0]=0;
                    if(strcasecmp(ptr,"true") == 0)
                    {
                        ptrData[0]=1;    
                    }
                    else
                    {
                        ptrData[0]=atoi(ptr);
                    }
                    size=1;
                    break;
                case MC_PRM_INT32:
                    size=4;
                    if(((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SOURCE_MASK) == MC_PRM_USER)
                    {
                        if(atoll(ptr) > (TreasurelandcoinParamArray+i)->m_MaxIntegerValue)
                        {
                            printf("Invalid parameter value for %s - too high: %s\n",param->m_DisplayName,ptr);                        
                            return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                        }
                        if(atoll(ptr) < (TreasurelandcoinParamArray+i)->m_MinIntegerValue)
                        {
                            printf("Invalid parameter value for %s - too low: %s\n",param->m_DisplayName,ptr);                        
                            return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                        }
                    }
                    if((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_DECIMAL)
                    {
                        double d=atof(ptr);
                        if(d >= 0)
                        {
                            *(int32_t*)ptrData=(int32_t)(d*MC_PRM_DECIMAL_GRANULARITY+ParamAccuracy());                            
                        }
                        else
                        {
                            *(int32_t*)ptrData=-(int32_t)(-d*MC_PRM_DECIMAL_GRANULARITY+ParamAccuracy());                                                        
                        }
                    }
                    else
                    {
                        *(int32_t*)ptrData=(int32_t)atol(ptr);
                    }
                    break;
                case MC_PRM_UINT32:
                    size=4;
                    if(((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SOURCE_MASK) == MC_PRM_USER)
                    {
                        if(atoll(ptr) > (TreasurelandcoinParamArray+i)->m_MaxIntegerValue)
                        {
                            printf("Invalid parameter value for %s - too high: %s\n",param->m_DisplayName,ptr);                        
                            return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                        }
                        if(atoll(ptr) < (TreasurelandcoinParamArray+i)->m_MinIntegerValue)
                        {
                            printf("Invalid parameter value for %s - too low: %s\n",param->m_DisplayName,ptr);                        
                            return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                        }
                    }
                    if((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_DECIMAL)
                    {
                        *(int32_t*)ptrData=(int32_t)(atof(ptr)*MC_PRM_DECIMAL_GRANULARITY+ParamAccuracy());
                    }
                    else
                    {
                        *(int32_t*)ptrData=(int32_t)atol(ptr);
                    }
                    if(ptr[0]=='-')
                    {
                        printf("Invalid parameter value for %s - should be non-negative\n",param->m_DisplayName);
                        return MC_ERR_INVALID_PARAMETER_VALUE;                                                                        
                    }
                    break;
                case MC_PRM_INT64:
                    if(((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SOURCE_MASK) == MC_PRM_USER)
                    {
                        if(atoll(ptr) > (TreasurelandcoinParamArray+i)->m_MaxIntegerValue)
                        {
                            printf("Invalid parameter value for %s - too high: %s\n",param->m_DisplayName,ptr);                        
                            return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                        }
                        if(atoll(ptr) < (TreasurelandcoinParamArray+i)->m_MinIntegerValue)
                        {
                            printf("Invalid parameter value for %s - too low: %s\n",param->m_DisplayName,ptr);                        
                            return MC_ERR_INVALID_PARAMETER_VALUE;                                                
                        }
                    }
                    size=8;
                    *(int64_t*)ptrData=(int64_t)atoll(ptr);
                    break;
                case MC_PRM_DOUBLE:
                    size=8;
                    *((double*)ptrData)=atof(ptr);
                    break;
            }            
            
            mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
            offset+=MC_PRM_PARAM_SIZE_BYTES;
            m_lpCoord[2 * i + 0]=offset;
            m_lpCoord[2 * i + 1]=size;
            offset+=size;            
        }
        else
        {
            if( ((((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SOURCE_MASK) != MC_PRM_CALCULATED) && 
               (((TreasurelandcoinParamArray+i)->m_Type & MC_PRM_SOURCE_MASK) != MC_PRM_GENERATED)) ||
               (argc > 0) )     
            {
                strcpy(m_lpData+offset,param->m_Name);
                offset+=strlen(param->m_Name)+1;
                size=0;

                ptrData=m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES;

                ptr=(char*)lpDefaultParams->GetParam(param->m_Name,&size);
                if(size)
                {
                    memcpy(ptrData,ptr,size);
                }            
                mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
                offset+=MC_PRM_PARAM_SIZE_BYTES;
                m_lpCoord[2 * i + 0]=offset;
                m_lpCoord[2 * i + 1]=size;
                offset+=size;                        
            }
        }

        param++;
    }

exitlbl:
                    
    m_Size=offset;

    delete mapConfig;
    if(lpDefaultParams)
    {
        delete lpDefaultParams;
    }
    if(err == MC_ERR_NOERROR)
    {
        len0=0;
        GetParam("addresspubkeyhashversion",&len0);
        len1=0;
        GetParam("addressscripthashversion",&len1);
        len2=0;
        GetParam("privatekeyversion",&len2);
        if(len1)                                                                // If params.dat is complete
        {
            if( (len0 != len1) || (len0 != len2) )            
            {
                printf("address-pubkeyhash-version, address-scripthash-version and private-key-version should have identical length \n");
                return MC_ERR_INVALID_PARAMETER_VALUE;                                                                                    
            }
        }
    }
    
    return err;
}

int mc_TreasurelandcoinParams::Set(const char *name,const char *source,int source_size)
{
    int size,offset,i,j,n;
    char *ptrData;
        
    Init();
    offset=0;

    j=0;
    while((j < source_size) && (*(source+j)!=0x00))
    {
        n=j;
        i=m_lpIndex->Get(source+j);
        j+=strlen(source+j)+1;
        if(j+MC_PRM_PARAM_SIZE_BYTES <= source_size)
        {
            size=mc_GetLE((void*)(source+j),MC_PRM_PARAM_SIZE_BYTES);
        }
        j+=MC_PRM_PARAM_SIZE_BYTES;
        if(j+size <= source_size)
        {
            if(i >= 0)
            {
                if(m_lpCoord[2 * i + 0] < 0)
                {
                    strcpy(m_lpData+offset,source+n);
                    offset+=strlen(source+n)+1;

                    ptrData=m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES;
                    if(size>0)
                    {
                        memcpy(ptrData,source+j,size);
                    }
                    mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
                    offset+=MC_PRM_PARAM_SIZE_BYTES;
                    m_lpCoord[2 * i + 0]=offset;
                    m_lpCoord[2 * i + 1]=size;
                    offset+=size;                                                    
                }
            }
            j+=size;
        }
    }
    
    m_Size=offset;
    return MC_ERR_NOERROR;    
}

int mc_TreasurelandcoinParams::Clone(const char* name, mc_TreasurelandcoinParams* source)
{
    int err;
    int size,offset,i,version;
    mc_OneTreasurelandcoinParam *param;
    char *ptrData;
    void *ptr;
    
    version=source->ProtocolVersion();
    if(version == 0)
    {
        version=mc_gState->GetProtocolVersion();
    }
    
    mc_TreasurelandcoinParams *lpDefaultParams;
    lpDefaultParams = new mc_TreasurelandcoinParams;
    
    err=lpDefaultParams->Create(name,version);
 
    if(err)
    {
        delete lpDefaultParams;
        return  err;
    }
    
    Init();
    offset=0;

    param=m_lpParams;
    for(i=0;i<m_Count;i++)
    {
        ptr=NULL;
        if(param->m_Type & MC_PRM_CLONE)
        {
            ptr=source->GetParam(param->m_Name,&size);
        }
        if(ptr == NULL)
        {
            ptr=lpDefaultParams->GetParam(param->m_Name,&size);
        }
        if(ptr)
        {            
//            m_lpIndex->Add(param->m_Name,i);

            strcpy(m_lpData+offset,param->m_Name);
            offset+=strlen(param->m_Name)+1;

            ptrData=m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES;
            if(size)
            {
                memcpy(ptrData,ptr,size);
            }
            mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
            offset+=MC_PRM_PARAM_SIZE_BYTES;
            m_lpCoord[2 * i + 0]=offset;
            m_lpCoord[2 * i + 1]=size;
            offset+=size;                        
        }
        param++;            
    }
    
    m_Size=offset;
    delete lpDefaultParams;
    
    return MC_ERR_NOERROR;
}

int mc_TreasurelandcoinParams::CalculateHash(unsigned char *hash)
{
    int i;
    int take_it;
    mc_SHA256 *hasher;
    
    hasher=new mc_SHA256;
    for(i=0;i<m_Count;i++)
    {
        take_it=1;
        if(((m_lpParams+i)->m_Type & MC_PRM_SOURCE_MASK) == MC_PRM_COMMENT)
        {
            take_it=0;
        }
        if((m_lpParams+i)->m_Type & MC_PRM_NOHASH)
        {
            take_it=0;            
        }
        if((m_lpParams+i)->m_Removed > 0)
        {
            if((m_lpParams+i)->m_Removed <= (int)GetInt64Param("protocolversion"))
            {
                take_it=0;
            }
        }

        if((m_lpParams+i)->IsRelevant((int)GetInt64Param("protocolversion")) == 0)
        {
            take_it=0;            
            switch((m_lpParams+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
            {
                case MC_PRM_BOOLEAN:
                case MC_PRM_INT32:
                case MC_PRM_UINT32:
                case MC_PRM_INT64:
                    if(GetInt64Param((m_lpParams+i)->m_Name) != (m_lpParams+i)->m_DefaultIntegerValue)
                    {
                        take_it=1;
                    }
                    break;
                case MC_PRM_STRING:
                    take_it=1;
                    
                    if(strcmp((m_lpParams+i)->m_Name,"rootstreamname") == 0)
                    {
                        if(strcmp((char*)GetParam((m_lpParams+i)->m_Name,NULL),"root") == 0)
                        {
                            take_it=0;
                        }
                    }                                   
                    break;
                case MC_PRM_BINARY:
                    take_it=1;
                    
                    if(strcmp((m_lpParams+i)->m_Name,"genesisopreturnscript") == 0)
                    {
                        take_it=0;
                    }                  
                    break;
                default:
                    take_it=1;                                                  // Not supported
                    break;
            }
        }
        if(take_it)
        {
            if(m_lpCoord[2* i + 1] > 0)
            {
                hasher->Write(m_lpData+m_lpCoord[2* i + 0],m_lpCoord[2* i + 1]);
            }
        }
    }
    
    hasher->GetHash(hash);
    hasher->Reset();
    hasher->Write(hash,32);
    hasher->GetHash(hash);
    delete hasher;
    
    return MC_ERR_NOERROR;
}


int mc_TreasurelandcoinParams::Validate()
{
    int i,size,offset;
    int isGenerated;
    int isMinimal;
    int isValid;
    unsigned char hash[32];
    char *ptrData;
    void *stored_hash;
    
    m_Status=MC_PRM_STATUS_EMPTY;

    if((m_lpParams == NULL) || (m_Size == 0))
    {
        return MC_ERR_NOERROR;
    }
        
    isGenerated=1;
    isMinimal=1;
    isValid=1;
    
    offset=m_Size;
    
    for(i=0;i<m_Count;i++)
    {
        if(m_lpCoord[2* i + 0] < 0)
        {
            switch((m_lpParams+i)->m_Type & MC_PRM_SOURCE_MASK)
            {
                case MC_PRM_COMMENT:                                            break;
                case MC_PRM_USER: 
                    if((m_lpParams+i)->IsRelevant((int)GetInt64Param("protocolversion")) == 0)
                    {
                        switch((m_lpParams+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
                        {
                            case MC_PRM_BOOLEAN:
                            case MC_PRM_INT32:
                            case MC_PRM_UINT32:
                            case MC_PRM_INT64:
                            case MC_PRM_DOUBLE:
                            case MC_PRM_STRING:

                                strcpy(m_lpData+offset,(m_lpParams+i)->m_Name);
                                offset+=strlen((m_lpParams+i)->m_Name)+1;
                                size=0;

                                ptrData=m_lpData+offset+MC_PRM_PARAM_SIZE_BYTES;
                                
                                switch((m_lpParams+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
                                {
                                    case MC_PRM_STRING:   
                                        if(strcmp((m_lpParams+i)->m_Name,"rootstreamname") == 0)
                                        {
                                            sprintf(ptrData,"root");
                                            size=strlen(ptrData)+1;
                                        }                                   
                                        break;
                                    case MC_PRM_BOOLEAN:
                                        size=1;
                                        if((m_lpParams+i)->m_DefaultIntegerValue)
                                        {
                                            ptrData[0]=1;
                                        }
                                        else
                                        {
                                            ptrData[0]=0;                                            
                                        }
                                        break;
                                    case MC_PRM_INT64:
                                        size=8;
                                        mc_PutLE(ptrData,&((m_lpParams+i)->m_DefaultIntegerValue),8);
                                        break;
                                    case MC_PRM_DOUBLE:
                                        size=8;
                                        *((double*)ptrData)=(m_lpParams+i)->m_DefaultDoubleValue;
                                        break;
                                    default:
                                        size=4;
                                        mc_PutLE(ptrData,&((m_lpParams+i)->m_DefaultIntegerValue),4);
                                        break;
                                }
                                
                                mc_PutLE(m_lpData+offset,&size,MC_PRM_PARAM_SIZE_BYTES);
                                offset+=MC_PRM_PARAM_SIZE_BYTES;
                                m_lpCoord[2 * i + 0]=offset;
                                m_lpCoord[2 * i + 1]=size;
                                offset+=size;            

                                break;
                            default:                                            // Not supported
                                isGenerated=0; 
                                isValid=0;
                                break;
                        }
                    }
                    else
                    {
                        isGenerated=0; 
                        isValid=0;
                    }
                    break;
                case MC_PRM_GENERATED: 
                    isGenerated=0; 
                    isValid=0;         
                    break;
                case MC_PRM_CALCULATED: 
                    if((m_lpParams+i)->IsRelevant((int)GetInt64Param("protocolversion")))
                    {                    
                        isValid=0;                       
                    }
                    break;
            }
            if((m_lpParams+i)->m_Type & MC_PRM_MINIMAL)
            {
                isMinimal=0;
            }
        }
    }   

    m_Size=offset;
    
    if(isValid)
    {
        m_Status=MC_PRM_STATUS_VALID;

        CalculateHash(hash);
        
        stored_hash=GetParam("chainparamshash",&size);
        if((stored_hash == NULL) || (size != 32))
        {
            m_Status=MC_PRM_STATUS_INVALID;
        }
        else
        {
            if(IsProtocolTreasurelandcoin())
            {
                if(memcmp(hash,stored_hash,32))
                {
                    m_Status=MC_PRM_STATUS_INVALID;                
                }
            }
        }
    }
    else
    {
        if(isGenerated)
        {
            m_Status=MC_PRM_STATUS_GENERATED;
        }
        else
        {
            if(isMinimal)
            {
                m_Status=MC_PRM_STATUS_MINIMAL;                            
            }
            else
            {
                m_Status=MC_PRM_STATUS_ERROR;            
            }
        }        
    }
    
    return MC_ERR_NOERROR;
}

int mc_TreasurelandcoinParams::Print(FILE* fileHan)
{
    int i,c,size;
    int version;
    int header_printed;
    int set,chars_remaining;
    void *ptr;
    char line[MC_PRM_DAT_FILE_LINE_SIZE+1+100];
    char *cptr;
    int num_sets;
    double d,d1,d2;
    
    int param_sets[]={MC_PRM_COMMENT, MC_PRM_USER, MC_PRM_GENERATED, MC_PRM_CALCULATED};
    num_sets=sizeof(param_sets)/sizeof(int);    
    
    fprintf(fileHan,"# ==== TreasureLandCoin configuration file ====\n\n");
    fprintf(fileHan,"# Created by treasurelandcoin-util \n");
    
    version=ProtocolVersion();
    if(version)
    {
        fprintf(fileHan,"# Protocol version: %d \n\n",version);
    }
    else
    {
        version=mc_gState->m_Features->LastVersionNotSendingProtocolVersionInHandShake();
    }
    
    switch(m_Status)
    {
        case MC_PRM_STATUS_EMPTY:
            fprintf(fileHan,"# Parameter set is EMPTY \n");
            fprintf(fileHan,"# To join network please run \"treasurelandcoind %s@<seed-node-ip>[:<seed-node-port>]\".\n",Name());
            return MC_ERR_NOERROR;
        case MC_PRM_STATUS_ERROR:
            fprintf(fileHan,"# This parameter set cannot be used for generating network. \n");
            fprintf(fileHan,"# One of the parameters is invalid. \n");
            fprintf(fileHan,"# Please fix it and rerun treasurelandcoin-util. \n");
            break;
        case MC_PRM_STATUS_MINIMAL:
            fprintf(fileHan,"# This parameter set contains MINIMAL number of parameters required for connection to existing network. \n");
            fprintf(fileHan,"# To join network please run \"treasurelandcoind %s@<seed-node-ip>[:<seed-node-port>]\".\n",Name());
            break;
        case MC_PRM_STATUS_GENERATED:
            fprintf(fileHan,"# This parameter set is properly GENERATED. \n");
            fprintf(fileHan,"# To generate network please run \"treasurelandcoind %s\".\n",Name());
            break;
        case MC_PRM_STATUS_VALID:
            fprintf(fileHan,"# This parameter set is VALID. \n");
            fprintf(fileHan,"# To join network please run \"treasurelandcoind %s\".\n",Name());
            break;
    }
        
    for(set=0;set<num_sets;set++)
    {
        header_printed=0;
                
        i=0;
        while(i>=0)
        {
            if( (((m_lpParams+i)->m_Type & MC_PRM_SOURCE_MASK) == param_sets[set]) && 
                    ((m_lpParams+i)->IsRelevant(version) > 0))
            {
                if(header_printed == 0)
                {
                    fprintf(fileHan,"\n");
                    header_printed=1;
                    switch(param_sets[set])
                    {
                        case MC_PRM_COMMENT:
                            fprintf(fileHan,"# The following parameters don't influence treasurelandcoin network configuration. \n");
                            fprintf(fileHan,"# They may be edited at any moment. \n");                            
                            break;
                        case MC_PRM_USER:
                            if(m_Status == MC_PRM_STATUS_ERROR)
                            {
                                fprintf(fileHan,"# The following parameters can be edited to fix errors. \n");
                                if(Name())
                                {
                                    fprintf(fileHan,"# Please rerun \"treasurelandcoin-util clone %s <new-network-name>\". \n",Name());
                                }
                            }
                            else
                            {
                                if(m_Status == MC_PRM_STATUS_GENERATED)
                                {
                                    fprintf(fileHan,"# The following parameters can be edited before running treasurelandcoind for this chain. \n");                                    
                                }
                                else
                                {
                                    fprintf(fileHan,"# The following parameters can only be edited if this file is a prototype of another configuration file. \n");
                                    fprintf(fileHan,"# Please run \"treasurelandcoin-util clone %s <new-network-name>\" to generate new network. \n",Name());
                                }
                            }
                            break;
                        case MC_PRM_GENERATED:
                            fprintf(fileHan,"# The following parameters were generated by treasurelandcoin-util.\n");
                            fprintf(fileHan,"# They SHOULD ONLY BE EDITED IF YOU KNOW WHAT YOU ARE DOING. \n");
                            break;
                        case MC_PRM_CALCULATED:
                            fprintf(fileHan,"# The following parameters were generated by treasurelandcoind.\n");
                            fprintf(fileHan,"# They SHOULD NOT BE EDITED. \n");
                            break;
                    }
                    fprintf(fileHan,"\n");
                }

                if(strlen((m_lpParams+i)->m_Group))
                {
                    fprintf(fileHan,"\n# %s\n\n",(m_lpParams+i)->m_Group);
                }
                
                chars_remaining=0;
                
                sprintf(line,"%s = ",(m_lpParams+i)->m_DisplayName);
                ptr=GetParam((m_lpParams+i)->m_Name,&size);
                if(size == 0)
                {
                    ptr=NULL;
                }                
                else
                {
                    if(((m_lpParams+i)->m_Type & MC_PRM_DATA_TYPE_MASK) == MC_PRM_STRING)
                    {
                        if(size == 1)
                        {
                            if( (((m_lpParams+i)->m_Type & MC_PRM_SPECIAL) == 0) ||
                                (strcmp((m_lpParams+i)->m_Name,"rootstreamname") != 0) )   
                            {
                                ptr=NULL;                    
                            }
                        }
                    }
                }

                if(ptr)
                {
                    switch((m_lpParams+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
                    {
                        case MC_PRM_BINARY:
                            if(2*size+strlen(line)>MC_PRM_DAT_FILE_LINE_SIZE)
                            {
                                fprintf(fileHan,"%s",line);
                                for(c=0;c<size;c++)
                                {
                                    fprintf(fileHan,"%02x",*((unsigned char*)ptr+c));
                                }
                                chars_remaining=1;
                            }
                            else
                            {
                                for(c=0;c<size;c++)
                                {
                                    sprintf(line+strlen(line),"%02x",*((unsigned char*)ptr+c));
                                }                            
                            }
                            break;
                        case MC_PRM_STRING:
                            if(size+strlen(line)>MC_PRM_DAT_FILE_LINE_SIZE)
                            {
                                fprintf(fileHan,"%s",line);
                                fprintf(fileHan,"%s",(char*)ptr);
                                chars_remaining=1;                                
                            }
                            else
                            {
                                sprintf(line+strlen(line),"%s",(char*)ptr);
                            }
                            break;
                        case MC_PRM_BOOLEAN:
                            if(*(char*)ptr)
                            {
                                sprintf(line+strlen(line),"true");                                
                            }
                            else
                            {
                                sprintf(line+strlen(line),"false");                                                                
                            }
                            break;
                        case MC_PRM_INT32:
                            if((m_lpParams+i)->m_Type & MC_PRM_DECIMAL)
                            {
                                if(mc_GetLE(ptr,4))
                                {
                                    int n=(int)mc_GetLE(ptr,4);
                                    if(n >= 0)
                                    {
                                        d=((double)n+ParamAccuracy())/MC_PRM_DECIMAL_GRANULARITY;
                                    }
                                    else
                                    {
                                        d=-((double)(-n)+ParamAccuracy())/MC_PRM_DECIMAL_GRANULARITY;                                        
                                    }
                                    sprintf(line+strlen(line),"%0.6g",d);                                                                
                                }
                                else
                                {
                                    d=0;
                                    sprintf(line+strlen(line),"0.0");                                                                
                                }
                            }
                            else
                            {
                                sprintf(line+strlen(line),"%d",(int)mc_GetLE(ptr,4));                                                                
                            }
                            break;
                        case MC_PRM_UINT32:
                            if((m_lpParams+i)->m_Type & MC_PRM_DECIMAL)
                            {
                                if(mc_GetLE(ptr,4))
                                {
                                    d=((double)mc_GetLE(ptr,4)+ParamAccuracy())/MC_PRM_DECIMAL_GRANULARITY;
                                    sprintf(line+strlen(line),"%0.6g",d);                                                                
                                }
                                else
                                {
                                    d=0;
                                    sprintf(line+strlen(line),"0.0");                                                                
                                }
                            }
                            else
                            {
                                sprintf(line+strlen(line),"%ld",mc_GetLE(ptr,4));                                                                
                            }
                            break;
                        case MC_PRM_INT64:
                            sprintf(line+strlen(line),"%lld",(long long int)mc_GetLE(ptr,8));                                                                
                            break;
                        case MC_PRM_DOUBLE:
                            sprintf(line+strlen(line),"%f",*(double*)ptr);                                                                
                            break;
                    }
                }
                else
                {
                    sprintf(line+strlen(line),"[null]");                                                                                    
                }
                if(chars_remaining == 0)
                {
                    fprintf(fileHan,"%s",line);
                    chars_remaining=MC_PRM_DAT_FILE_LINE_SIZE-strlen(line)+1;
                }
                for(c=0;c<chars_remaining;c++)
                {
                    fprintf(fileHan," ");
                }
                cptr=(m_lpParams+i)->m_Description;
                while(*cptr)
                {
                    c=0;
                    
                    while((c<(int)strlen(cptr)) && (cptr[c]!='\n'))
                    {
                        c++;
                    }
                    
                    if(c<(int)strlen(cptr))
                    {
                        cptr[c]=0x00;
                        fprintf(fileHan,"# %s",cptr);
                        memset(line,0x20,MC_PRM_DAT_FILE_LINE_SIZE);
                        line[MC_PRM_DAT_FILE_LINE_SIZE]=0x00;
                        fprintf(fileHan,"\n%s ",line);
                        cptr+=c+1;
                    }
                    else
                    {
                        fprintf(fileHan,"# %s",cptr);
                        cptr+=c;
                    }
                }

                switch((m_lpParams+i)->m_Type & MC_PRM_DATA_TYPE_MASK)
                {
                    case MC_PRM_INT32:
                    case MC_PRM_INT64:
                    case MC_PRM_UINT32:
                        switch(param_sets[set])
                        {
                            case MC_PRM_COMMENT:
                            case MC_PRM_USER:
                                if((m_lpParams+i)->m_MinIntegerValue <= (m_lpParams+i)->m_MaxIntegerValue)
                                {
                                    if((m_lpParams+i)->m_Type & MC_PRM_DECIMAL)
                                    {
                                        d1=0;
                                        if((m_lpParams+i)->m_MinIntegerValue)
                                        {
                                            d1=((double)((m_lpParams+i)->m_MinIntegerValue)+ParamAccuracy())/MC_PRM_DECIMAL_GRANULARITY;
                                        }
                                        d2=0;
                                        if((m_lpParams+i)->m_MaxIntegerValue)
                                        {
                                            d2=((double)((m_lpParams+i)->m_MaxIntegerValue)+ParamAccuracy())/MC_PRM_DECIMAL_GRANULARITY;
                                        }
                                        fprintf(fileHan," (%0.6g - %0.6g)",d1,d2);                            
                                    }
                                    else
                                    {
                                        fprintf(fileHan," (%ld - %ld)",(m_lpParams+i)->m_MinIntegerValue,(m_lpParams+i)->m_MaxIntegerValue);                            
                                    }
                                }
                                break;
                        }
                        break;
                }
                fprintf(fileHan,"\n");                    
            }
            
            if(strlen((m_lpParams+i)->m_Next))
            {
                i=m_lpIndex->Get((m_lpParams+i)->m_Next);
            }
            else
            {
                i=-1;
            }
        }   
        
    }

    fprintf(fileHan,"\n");                    
    return MC_ERR_NOERROR;

}

int mc_TreasurelandcoinParams::Write(int overwrite)
{
    FILE *fileHan;
    int create;
    char fileName[MC_DCT_DB_MAX_PATH];
    
    if(Name() == NULL)
    {
        return MC_ERR_INTERNAL_ERROR;
    }
    
    fileHan=mc_OpenFile(Name(),"params",".dat","r",MC_FOM_RELATIVE_TO_DATADIR);
    if(fileHan)
    {
        mc_CloseFile(fileHan);
        if(overwrite)
        {
            mc_BackupFile(Name(),"params",".dat",MC_FOM_RELATIVE_TO_DATADIR);
        }
        else
        {
            mc_GetFullFileName(mc_gState->m_Params->m_Arguments[1],"params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
            printf("Cannot create chain parameter set, file %s already exists\n",fileName);
            return MC_ERR_INVALID_PARAMETER_VALUE;                
        }
    }

    create=MC_FOM_CREATE_DIR;
    if(overwrite)
    {
        create=0;
    }
    
    fileHan=mc_OpenFile(Name(),"params",".dat","w",MC_FOM_RELATIVE_TO_DATADIR | create);
    if(fileHan == NULL)
    {
        mc_GetFullFileName(mc_gState->m_Params->m_Arguments[1],"params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
        printf("Cannot create chain parameter set, cannot open file %s for writing\n",fileName);
        return MC_ERR_INVALID_PARAMETER_VALUE;                
    }

    if(Print(fileHan))
    {
        mc_GetFullFileName(mc_gState->m_Params->m_Arguments[1],"params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
        printf("Cannot create chain parameter set, write error to file %s\n",fileName);
        mc_CloseFile(fileHan);
        if(overwrite)
        {
            mc_RecoverFile(Name(),"params",".dat",MC_FOM_RELATIVE_TO_DATADIR);
        }
        return MC_ERR_INVALID_PARAMETER_VALUE;                        
    }
    
    mc_CloseFile(fileHan);
    
    
    return MC_ERR_NOERROR;
}




const char* mc_TreasurelandcoinParams::Name()
{
    return (char*)GetParam("chainname",NULL);
}

const unsigned char* mc_TreasurelandcoinParams::MessageStart()
{
    return (unsigned char*)GetParam("networkmessagestart",NULL);
}

const unsigned char* mc_TreasurelandcoinParams::DefaultMessageStart()
{
    return c_DefaultMessageStart;
}


const unsigned char* mc_TreasurelandcoinParams::AddressVersion()
{
    return (unsigned char*)GetParam("addresspubkeyhashversion",NULL);
}

const unsigned char* mc_TreasurelandcoinParams::AddressScriptVersion()
{
    return (unsigned char*)GetParam("addressscripthashversion",NULL);
}

const unsigned char* mc_TreasurelandcoinParams::AddressCheckumValue()
{
    return (unsigned char*)GetParam("addresschecksumvalue",NULL);
}


int mc_TreasurelandcoinParams::ProtocolVersion()
{
    if(m_ProtocolVersion)
    {
        return m_ProtocolVersion;
    }
    void *ptr=GetParam("protocolversion",NULL);
    if(ptr)
    {
        return mc_GetLE(ptr,4);
    }
    return 0;
}

int mc_TreasurelandcoinParams::IsProtocolTreasurelandcoin()
{
    return m_IsProtocolTreasureLandCoin;
}


int mc_Features::ActivatePermission()
{
    int ret=0;
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10003)
        {
            ret=1;
        }
    }
    
    return ret;
}

int mc_Features::LastVersionNotSendingProtocolVersionInHandShake()
{
    return 10002;
}

int mc_Features::VerifySizeOfOpDropElements()
{
    
    int ret=0;        
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 1;
    }
    
    if(protocol)
    {
        if(protocol >= 10003)
        {
            ret=1;
        }
    }
    
    return ret;
}

int mc_Features::PerEntityPermissions()
{
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int ret=0;
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10004)
        {
            ret=1;
        }
    }
    
    return ret;
}

int mc_Features::FollowOnIssues()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    if(protocol)
    {
        if(protocol >= 10004)
        {
            ret=1;
        }
    }
    
    return ret;
}

int mc_Features::SpecialParamsInDetailsScript()
{
    int ret=0;
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10004)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::FixedGrantsInTheSameTx()
{
    int ret=0;
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10004)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::UnconfirmedMinersCannotMine()
{
    int ret=0;
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10004)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::Streams()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10006)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::OpDropDetailsScripts()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10007)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::ShortTxIDAsAssetRef()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10007)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::CachedInputScript()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10007)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::AnyoneCanReceiveEmpty()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10007)
        {
            if(mc_gState->m_NetworkParams->GetInt64Param("anyonecanreceiveempty"))                                
            {
                ret=1;
            }
        }
    }
    
    return ret;    
}

int mc_Features::FixedIn10007()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10007)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::Upgrades()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10008)
        {
            ret=1;
        }
    }
    
    return ret;    
}

int mc_Features::FixedIn10008()
{
    int ret=0;
    if(mc_gState->m_NetworkParams->IsProtocolTreasurelandcoin() == 0)
    {
        return 0;
    }
    int protocol=mc_gState->m_NetworkParams->ProtocolVersion();
    
    if(protocol)
    {
        if(protocol >= 10008)
        {
            ret=1;
        }
    }
    
    return ret;    
}



// Copyright (c) 2014-2017 Coin Sciences Ltd
// TreasureLandCoin code distributed under the GPLv3 license, see COPYING file.

#include "treasurelandcoin/treasurelandcoin.h"
#include "chainparams/globals.h"

int main(int argc, char* argv[])
{
    int err;
    int version,v;
    char fileName[MC_DCT_DB_MAX_PATH];
    char DataDirArg[MC_DCT_DB_MAX_PATH];
    int isSetDataDirArg;
    FILE *fHan;
    
    mc_TreasurelandcoinParams* params;
    mc_TreasurelandcoinParams* paramsOld;
    mc_gState=new mc_State;
     
    mc_gState->m_Params->Parse(argc, argv);
    mc_CheckDataDirInConfFile();

    mc_gState->m_Params->ReadConfig(NULL);
    
    mc_ExpandDataDirParam();
    
    printf("TreasureLandCoin utilities %s\n\n",mc_gState->GetFullVersion());
             
    err=MC_ERR_OPERATION_NOT_SUPPORTED;
     
    if(mc_gState->m_Params->Command())
    {
        if(strcmp(mc_gState->m_Params->Command(),"create") == 0)
        {
            if(mc_gState->m_Params->m_NumArguments>1)
            {
                params=new mc_TreasurelandcoinParams;
                
                err=MC_ERR_NOERROR;
                
                version=mc_gState->GetProtocolVersion();
                if(mc_gState->m_Params->m_NumArguments>2)
                {                    
                    v=atoi(mc_gState->m_Params->m_Arguments[2]);
                    if( (v>=10002) && (v<=version) )
                    {
                        version=v;                        
                    }
                    else
                    {
                        fprintf(stderr,"ERROR: Invalid value for protocol version. Valid range: 10002 - %d\n",mc_gState->GetProtocolVersion());   
                        err=MC_ERR_INVALID_PARAMETER_VALUE;
                    }
                }
                
                if(err == MC_ERR_NOERROR)
                {
//                    err=params->Create(mc_gState->m_Params->m_Arguments[1],version);
                    err=params->Read(mc_gState->m_Params->m_Arguments[1],argc, argv,version);
                }
                if(err == MC_ERR_NOERROR)
                {
                    err=params->Validate();
                }
                if(err == MC_ERR_NOERROR)
                {
                    err=params->Write(0);
                }                

                if(err == MC_ERR_NOERROR)
                {
                    mc_GenerateConfFiles(mc_gState->m_Params->m_Arguments[1]);                
                }
                if(err == MC_ERR_NOERROR)
                { 
                    printf("Blockchain parameter set was successfully generated.\n");
                    mc_GetFullFileName(mc_gState->m_Params->m_Arguments[1],"params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
                    printf("You can edit it in %s before running treasurelandcoind for the first time.\n\n",fileName);
                    printf("To generate blockchain please run \"treasurelandcoind %s -daemon\".\n",params->Name());
                }                
                else
                {
                    fprintf(stderr,"ERROR: Blockchain parameter set was not generated.\n");
                }
                delete params;
            }    
        }
        if(strcmp(mc_gState->m_Params->Command(),"clone") == 0)
        {
            if(mc_gState->m_Params->m_NumArguments>2)
            {
                params=new mc_TreasurelandcoinParams;
                paramsOld=new mc_TreasurelandcoinParams;

                
                isSetDataDirArg=mc_GetDataDirArg(DataDirArg);
                if(isSetDataDirArg)
                {
                    mc_UnsetDataDirArg();
                }                
                err=MC_ERR_NOERROR;
                mc_GetFullFileName(mc_gState->m_Params->m_Arguments[1],"params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
                if ((fHan = fopen(fileName, "r")))
                {
                    fclose(fHan);
                }
                else
                {
                    fprintf(stderr,"Cannot create chain parameter set, file %s does not exist\n",fileName);                    
                    err=MC_ERR_FILE_READ_ERROR;
                }

                if(err == MC_ERR_NOERROR)
                {
                    err=paramsOld->Read(mc_gState->m_Params->m_Arguments[1]);
                }                
                if(isSetDataDirArg)
                {
                    mc_SetDataDirArg(DataDirArg);
                }                

                mc_gState->m_Params->m_Arguments[1]=mc_gState->m_Params->m_Arguments[2];
                if(err == MC_ERR_NOERROR)
                {
                    err=params->Clone(mc_gState->m_Params->m_Arguments[2],paramsOld);
                }                
                if(err == MC_ERR_NOERROR)
                {
                    err=params->Validate();
                }
                if(err == MC_ERR_NOERROR)
                {
                    err=params->Write(0);
                }
                if(err == MC_ERR_NOERROR)
                {
                    
                    printf("Blockchain parameter set was successfully cloned.\n");
                    mc_GetFullFileName(mc_gState->m_Params->m_Arguments[2],"params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
                    printf("You can edit it in %s before running treasurelandcoind for the first time.\n\n",fileName);
                    printf("To generate blockchain please run \"treasurelandcoind %s -daemon\".\n",params->Name());
                }                
                else
                {
                    fprintf(stderr,"ERROR: Blockchain parameter set was not generated.\n");
                }
                delete paramsOld;
                delete params;
            }
            err=MC_ERR_NOERROR;
        }
/*        
        if(strcmp(mc_gState->m_Params->Command(),"test") == 0)
        {
            if(mc_gState->m_Params->m_NumArguments>1)
            {
                printf("\n>>>>> Test %s started\n\n",mc_gState->m_Params->m_Arguments[1]);
                if(strcmp(mc_gState->m_Params->m_Arguments[1],"scenario") == 0)
                {
                    if(mc_gState->m_Params->m_NumArguments>2)
                    {
                        err=mc_TestScenario(mc_gState->m_Params->m_Arguments[2]);                        
                    }                    
                }        
            }           
            
            if(err == MC_ERR_NOERROR)
            {
                printf("\n>>>>> Test completed\n\n");
            }            
            else
            {
                if(err == MC_ERR_OPERATION_NOT_SUPPORTED)                
                {
                    printf("\n>>>>> ERROR: Test not found\n\n");                    
                }
                else
                {
                    printf("\n>>>>> ERROR: Test exited with error code %d\n\n",err);                    
                }
            }
            
        }
*/        
        
    }
    
    if(err == MC_ERR_OPERATION_NOT_SUPPORTED)
    {
        mc_GetFullFileName("<blockchain-name>","params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
        printf("Usage:\n");
        printf("  treasurelandcoin-util create <blockchain-name>  ( <protocol-version> = %d ) [options]        Creates new treasurelandcoin configuration file %s with default parameters\n",
                mc_gState->GetProtocolVersion(),fileName);
        mc_GetFullFileName("<new-blockchain-name>","params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
        printf("  treasurelandcoin-util clone <old-blockchain-name> <new-blockchain-name> [options]               Creates new treasurelandcoin configuration file %s copying parameters\n",fileName);
        
        isSetDataDirArg=mc_GetDataDirArg(DataDirArg);
        if(isSetDataDirArg)
        {
            mc_UnsetDataDirArg();
        }                
        mc_GetFullFileName("<old-blockchain-name>","params", ".dat",MC_FOM_RELATIVE_TO_DATADIR,fileName);
        if(isSetDataDirArg)
        {
            mc_SetDataDirArg(DataDirArg);
        }                
        printf("                                                                                            from %s\n",fileName);
        printf("\n");
        printf("Options:\n");
        printf("  -datadir=<dir>                              Specify data directory\n");
        printf("  -<parameter-name>=<parameter-value>         Specify blockchain parameter value, e.g. -anyone-can-connect=true\n\n");
    }
            
    delete mc_gState;

    if(err)
    {
        return 1;
    }
    
    return 0;
}

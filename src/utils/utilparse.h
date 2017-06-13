// Copyright (c) 2014-2017 Coin Sciences Ltd
// TreasureLandCoin code distributed under the GPLv3 license, see COPYING file.

#ifndef TREASURELANDCOINUTILS_H
#define	TREASURELANDCOINUTILS_H

#include "structs/base58.h"
#include "treasurelandcoin/treasurelandcoin.h"
#include "primitives/transaction.h"
#include "keys/key.h"
#include "core/main.h"

bool ExtractDestinationScriptValid(const CScript& scriptPubKey, CTxDestination& addressRet);
const unsigned char* GetAddressIDPtr(const CTxDestination& address);
bool ParseTreasurelandcoinTxOutToBuffer(uint256 hash,const CTxOut& txout,mc_Buffer *amounts,mc_Script *lpScript,int *allowed,int *required,std::map<uint32_t, uint256>* mapSpecialEntity,std::string& strFailReason);
bool ParseTreasurelandcoinTxOutToBuffer(uint256 hash,const CTxOut& txout,mc_Buffer *amounts,mc_Script *lpScript,int *allowed,int *required,std::string& strFailReason);
bool CreateAssetBalanceList(const CTxOut& txout,mc_Buffer *amounts,mc_Script *lpScript,int *required);
bool CreateAssetBalanceList(const CTxOut& txout,mc_Buffer *amounts,mc_Script *lpScript);
void LogAssetTxOut(std::string message,uint256 hash,int index,unsigned char* assetrefbin,int64_t quantity);
bool AddressCanReceive(CTxDestination address);
bool FindFollowOnsInScript(const CScript& script1,mc_Buffer *amounts,mc_Script *lpScript);






#endif	/* TREASURELANDCOINUTILS_H */


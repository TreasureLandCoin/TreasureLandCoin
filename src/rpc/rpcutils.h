// Copyright (c) 2014-2016 The Bitcoin Core developers
// Original code was distributed under the MIT software license.
// Copyright (c) 2014-2017 Coin Sciences Ltd
// TreasureLandCoin code distributed under the GPLv3 license, see COPYING file.

#ifndef RPCTREASURELANDCOINUTILS_H
#define	RPCTREASURELANDCOINUTILS_H

#include "primitives/transaction.h"
#include "core/init.h"
#include "core/main.h"
#include "wallet/keystore.h"
#include "rpc/rpcserver.h"
//#include "script/script.h"
//#include "script/standard.h"
#include "structs/uint256.h"

#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "treasurelandcoin/treasurelandcoin.h"
#include "utils/utilparse.h"


using namespace std;
using namespace json_spirit;
#include <map>
#include <string>

//string HelpRequiringPassphrase();
string AllowedPermissions();
string AllowedPausedServices();
uint32_t GetPausedServices(const char *str);



CScript RemoveOpDropsIfNeeded(const CScript& scriptInput);
bool CoinSparkAssetRefDecode(unsigned char *bin, const char* string, const size_t stringLen);
int ParseAssetKey(const char* asset_key,unsigned char *txid,unsigned char *asset_ref,char *name,int *multiple,int *type,int entity_type);
int ParseAssetKeyToFullAssetRef(const char* asset_key,unsigned char *full_asset_ref,int *multiple,int *type,int entity_type);
Array AddressEntries(const CTxIn& txin,txnouttype& typeRet,mc_Script *lpScript);
Array AddressEntries(const CTxOut& txout,txnouttype& typeRet);
Value PermissionForFieldEntry(mc_EntityDetails *lpEntity);
Array PermissionEntries(const CTxOut& txout,mc_Script *lpScript,bool fLong);
Object StreamEntry(const unsigned char *txid,uint32_t output_level);
Object UpgradeEntry(const unsigned char *txid);
Value OpReturnEntry(const unsigned char *elem,size_t elem_size,uint256 txid, int vout);
Value DataItemEntry(const CTransaction& tx,int n,set <uint256>& already_seen,uint32_t stream_output_level);
Object AssetEntry(const unsigned char *txid,int64_t quantity,int output_level);
string ParseRawOutputObject(Value param,CAmount& nAmount,mc_Script *lpScript,int *eErrorCode);
bool FindPreparedTxOut(CTxOut& txout,COutPoint outpoint,string& reason);
bool GetTxInputsAsTxOuts(const CTransaction& tx, vector <CTxOut>& inputs, vector <string>& errors,string& reason);
CScript GetScriptForString(string source);
vector <pair<CScript, CAmount> > ParseRawOutputMultiObject(Object sendTo,int *required);
CScript ParseRawMetadata(Value param,uint32_t allowed_objects,mc_EntityDetails *given_entity,mc_EntityDetails *found_entity);
vector<string> ParseStringList(Value param);
void ParseEntityIdentifier(Value entity_identifier,mc_EntityDetails *entity,uint32_t entity_type);
bool AssetCompareByRef(Value a,Value b);
Array AssetArrayFromAmounts(mc_Buffer *asset_amounts,int issue_asset_id,uint256 hash,int show_type);
void ParseRawAction(string action,bool& lock_it, bool& sign_it,bool& send_it);
bool paramtobool(Value param);

#endif	/* RPCTREASURELANDCOINUTILS_H */


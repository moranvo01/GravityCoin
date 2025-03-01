// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.



#include "base58.h"
#include "consensus/validation.h"
#include "main.h" // For CheckTransaction
#include "protocol.h"
#include "serialize.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "wallet/wallet.h"

#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

using namespace std;

static uint64_t nAccountingEntryNumber = 0;

//
// CWalletDB
//

bool CWalletDB::WriteName(const string &strAddress, const string &strName) {
    nWalletDBUpdated++;
    return Write(make_pair(string("name"), strAddress), strName);
}

bool CWalletDB::EraseName(const string &strAddress) {
    // This should only be used for sending addresses, never for receiving addresses,
    // receiving addresses must always have an address book entry if they're not change return.
    nWalletDBUpdated++;
    return Erase(make_pair(string("name"), strAddress));
}

bool CWalletDB::WritePurpose(const string &strAddress, const string &strPurpose) {
    nWalletDBUpdated++;
    return Write(make_pair(string("purpose"), strAddress), strPurpose);
}

bool CWalletDB::ErasePurpose(const string &strPurpose) {
    nWalletDBUpdated++;
    return Erase(make_pair(string("purpose"), strPurpose));
}

bool CWalletDB::WriteTx(const CWalletTx &wtx) {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("tx"), wtx.GetHash()), wtx);
}

bool CWalletDB::EraseTx(uint256 hash) {
    nWalletDBUpdated++;
    return Erase(std::make_pair(std::string("tx"), hash));
}

bool CWalletDB::WriteKey(const CPubKey &vchPubKey, const CPrivKey &vchPrivKey, const CKeyMetadata &keyMeta) {
    nWalletDBUpdated++;

    if (!Write(std::make_pair(std::string("keymeta"), vchPubKey),
               keyMeta, false))
        return false;

    // hash pubkey/privkey to accelerate wallet load
    std::vector<unsigned char> vchKey;
    vchKey.reserve(vchPubKey.size() + vchPrivKey.size());
    vchKey.insert(vchKey.end(), vchPubKey.begin(), vchPubKey.end());
    vchKey.insert(vchKey.end(), vchPrivKey.begin(), vchPrivKey.end());

    return Write(std::make_pair(std::string("key"), vchPubKey),
                 std::make_pair(vchPrivKey, Hash(vchKey.begin(), vchKey.end())), false);
}

bool CWalletDB::WriteCryptedKey(const CPubKey &vchPubKey,
                                const std::vector<unsigned char> &vchCryptedSecret,
                                const CKeyMetadata &keyMeta) {
    const bool fEraseUnencryptedKey = true;
    nWalletDBUpdated++;

    if (!Write(std::make_pair(std::string("keymeta"), vchPubKey),
               keyMeta))
        return false;

    if (!Write(std::make_pair(std::string("ckey"), vchPubKey), vchCryptedSecret, false))
        return false;
    if (fEraseUnencryptedKey) {
        Erase(std::make_pair(std::string("key"), vchPubKey));
        Erase(std::make_pair(std::string("wkey"), vchPubKey));
    }
    return true;
}

bool CWalletDB::WriteMasterKey(unsigned int nID, const CMasterKey &kMasterKey) {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("mkey"), nID), kMasterKey, true);
}

bool CWalletDB::WriteCScript(const uint160 &hash, const CScript &redeemScript) {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("cscript"), hash), *(const CScriptBase *) (&redeemScript), false);
}

bool CWalletDB::WriteWatchOnly(const CScript &dest) {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("watchs"), *(const CScriptBase *) (&dest)), '1');
}

bool CWalletDB::EraseWatchOnly(const CScript &dest) {
    nWalletDBUpdated++;
    return Erase(std::make_pair(std::string("watchs"), *(const CScriptBase *) (&dest)));
}

bool CWalletDB::WriteBestBlock(const CBlockLocator &locator) {
    nWalletDBUpdated++;
    Write(std::string("bestblock"),
          CBlockLocator()); // Write empty block locator so versions that require a merkle branch automatically rescan
    return Write(std::string("bestblock_nomerkle"), locator);
}

bool CWalletDB::ReadBestBlock(CBlockLocator &locator) {
    if (Read(std::string("bestblock"), locator) && !locator.vHave.empty()) return true;
    return Read(std::string("bestblock_nomerkle"), locator);
}

bool CWalletDB::WriteOrderPosNext(int64_t nOrderPosNext) {
    nWalletDBUpdated++;
    return Write(std::string("orderposnext"), nOrderPosNext);
}

bool CWalletDB::WriteDefaultKey(const CPubKey &vchPubKey) {
    nWalletDBUpdated++;
    return Write(std::string("defaultkey"), vchPubKey);
}

bool CWalletDB::ReadPool(int64_t nPool, CKeyPool &keypool) {
    return Read(std::make_pair(std::string("pool"), nPool), keypool);
}

bool CWalletDB::WritePool(int64_t nPool, const CKeyPool &keypool) {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("pool"), nPool), keypool);
}

bool CWalletDB::ErasePool(int64_t nPool) {
    nWalletDBUpdated++;
    return Erase(std::make_pair(std::string("pool"), nPool));
}

bool CWalletDB::WriteMinVersion(int nVersion) {
    return Write(std::string("minversion"), nVersion);
}

bool CWalletDB::ReadAccount(const string &strAccount, CAccount &account) {
    account.SetNull();
    return Read(make_pair(string("acc"), strAccount), account);
}

bool CWalletDB::WriteAccount(const string &strAccount, const CAccount &account) {
    return Write(make_pair(string("acc"), strAccount), account);
}

bool CWalletDB::WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry &acentry) {
    return Write(std::make_pair(std::string("acentry"), std::make_pair(acentry.strAccount, nAccEntryNum)), acentry);
}

bool CWalletDB::WriteAccountingEntry_Backend(const CAccountingEntry &acentry) {
    return WriteAccountingEntry(++nAccountingEntryNumber, acentry);
}

CAmount CWalletDB::GetAccountCreditDebit(const string &strAccount) {
    list <CAccountingEntry> entries;
    ListAccountCreditDebit(strAccount, entries);

    CAmount nCreditDebit = 0;
    BOOST_FOREACH(const CAccountingEntry &entry, entries)
    nCreditDebit += entry.nCreditDebit;

    return nCreditDebit;
}

void CWalletDB::ListAccountCreditDebit(const string &strAccount, list <CAccountingEntry> &entries) {
    bool fAllAccounts = (strAccount == "*");

    Dbc *pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error(std::string(__func__) + ": cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    while (true) {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << std::make_pair(std::string("acentry"), std::make_pair((fAllAccounts ? string("") : strAccount), uint64_t(0)));
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0) {
            pcursor->close();
            throw runtime_error(std::string(__func__) + ": error scanning DB");
        }

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "acentry")
            break;
        CAccountingEntry acentry;
        ssKey >> acentry.strAccount;
        if (!fAllAccounts && acentry.strAccount != strAccount)
            break;

        ssValue >> acentry;
        ssKey >> acentry.nEntryNo;
        entries.push_back(acentry);
    }

    pcursor->close();
}

bool CWalletDB::WriteCoinSpendSerialEntry(const CZerocoinSpendEntry &zerocoinSpend) {
    return Write(make_pair(string("zcserial"), zerocoinSpend.coinSerial), zerocoinSpend, true);
}

bool CWalletDB::WriteCoinSpendSerialEntry(const CSigmaSpendEntry &zerocoinSpend) {
    return Write(std::make_pair(std::string("sigma_spend"), zerocoinSpend.coinSerial), zerocoinSpend, true);
}

bool CWalletDB::HasCoinSpendSerialEntry(const Bignum& serial) {
    return Exists(std::make_pair(std::string("zcserial"), serial));
}

bool CWalletDB::HasCoinSpendSerialEntry(const secp_primitives::Scalar& serial) {
    return Exists(std::make_pair(std::string("sigma_spend"), serial));
}

bool CWalletDB::EraseCoinSpendSerialEntry(const CZerocoinSpendEntry &zerocoinSpend) {
    return Erase(make_pair(string("zcserial"), zerocoinSpend.coinSerial));
}

bool CWalletDB::EraseCoinSpendSerialEntry(const CSigmaSpendEntry &zerocoinSpend) {
    return Erase(std::make_pair(std::string("sigma_spend"), zerocoinSpend.coinSerial));
}

bool
CWalletDB::WriteZerocoinAccumulator(libzerocoin::Accumulator accumulator, libzerocoin::CoinDenomination denomination,
                                    int pubcoinid) {
    return Write(std::make_tuple(string("zcaccumulator"), (unsigned int) denomination, pubcoinid), accumulator);
}

bool
CWalletDB::ReadZerocoinAccumulator(libzerocoin::Accumulator &accumulator, libzerocoin::CoinDenomination denomination,
                                   int pubcoinid) {
    return Read(std::make_tuple(string("zcaccumulator"), (unsigned int) denomination, pubcoinid), accumulator);
}

//bool CWalletDB::EraseZerocoinAccumulator(libzerocoin::Accumulator& accumulator, libzerocoin::CoinDenomination denomination, int pubcoinid)
//{
//    return Erase(std::make_tuple(string("zcaccumulator"), (unsigned int) denomination, pubcoinid), accumulator);
//}

bool CWalletDB::WriteZerocoinEntry(const CZerocoinEntry &zerocoin) {
    return Write(make_pair(string("zerocoin"), zerocoin.value), zerocoin, true);
}

bool CWalletDB::WriteZerocoinEntry(const CSigmaEntry &zerocoin) {
    return Write(std::make_pair(std::string("sigma_mint"), zerocoin.value), zerocoin, true);
}

bool CWalletDB::ReadZerocoinEntry(const Bignum& pub, CZerocoinEntry& entry) {
    return Read(std::make_pair(std::string("zerocoin"), pub), entry);
}

bool CWalletDB::ReadZerocoinEntry(const secp_primitives::GroupElement& pub, CSigmaEntry& entry) {
    return Read(std::make_pair(std::string("sigma_mint"), pub), entry);
}

bool CWalletDB::HasZerocoinEntry(const Bignum& pub) {
    return Exists(std::make_pair(std::string("zerocoin"), pub));
}

bool CWalletDB::HasZerocoinEntry(const secp_primitives::GroupElement& pub) {
    return Exists(std::make_pair(std::string("sigma_mint"), pub));
}

bool CWalletDB::EraseZerocoinEntry(const CSigmaEntry &zerocoin) {
    return Erase(std::make_pair(std::string("sigma_mint"), zerocoin.value));
}

bool CWalletDB::EraseZerocoinEntry(const CZerocoinEntry &zerocoin) {
    return Erase(make_pair(string("zerocoin"), zerocoin.value));
}

// Check Calculated Blocked for Zerocoin
bool CWalletDB::ReadCalculatedZCBlock(int &height) {
    height = 0;
    return Read(std::string("calculatedzcblock"), height);
}

bool CWalletDB::WriteCalculatedZCBlock(int height) {
    return Write(std::string("calculatedzcblock"), height);
}

void CWalletDB::ListPubCoin(std::list <CZerocoinEntry> &listPubCoin) {
    Dbc *pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error("CWalletDB::ListPubCoin() : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    while (true) {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << make_pair(string("zerocoin"), CBigNum(0));
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0) {
            pcursor->close();
            throw runtime_error("CWalletDB::ListPubCoin() : error scanning DB");
        }
        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "zerocoin")
            break;
        CBigNum value;
        ssKey >> value;
        CZerocoinEntry zerocoinItem;
        ssValue >> zerocoinItem;
        listPubCoin.push_back(zerocoinItem);
    }
    pcursor->close();
}

void CWalletDB::ListSigmaPubCoin(std::list <CSigmaEntry> &listPubCoin) {
    Dbc *pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error("CWalletDB::ListSigmaPubCoin() : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    while (true) {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << std::make_pair(std::string("sigma_mint"), secp_primitives::GroupElement());
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0) {
            pcursor->close();
            throw runtime_error("CWalletDB::ListSigmaPubCoin() : error scanning DB");
        }
        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "sigma_mint")
            break;
        GroupElement value;
        ssKey >> value;
        CSigmaEntry zerocoinItem;
        ssValue >> zerocoinItem;
        listPubCoin.push_back(zerocoinItem);
    }
    pcursor->close();
}

void CWalletDB::ListCoinSpendSerial(std::list <CZerocoinSpendEntry> &listCoinSpendSerial) {
    Dbc *pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error("CWalletDB::ListCoinSpendSerial() : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    while (true) {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << make_pair(string("zcserial"), CBigNum(0));
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0) {
            pcursor->close();
            throw runtime_error("CWalletDB::ListCoinSpendSerial() : error scanning DB");
        }

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "zcserial")
            break;
        CBigNum value;
        ssKey >> value;
        CZerocoinSpendEntry zerocoinSpendItem;
        ssValue >> zerocoinSpendItem;
        listCoinSpendSerial.push_back(zerocoinSpendItem);
    }

    pcursor->close();
}

void CWalletDB::ListCoinSpendSerial(std::list <CSigmaSpendEntry> &listCoinSpendSerial) {
    Dbc *pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error("CWalletDB::ListCoinSpendSerial() : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    while (true) {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << std::make_pair(std::string("sigma_spend"), secp_primitives::GroupElement());
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0) {
            pcursor->close();
            throw runtime_error("CWalletDB::ListCoinSpendSerial() : error scanning DB");
        }

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "sigma_spend")
            break;
        Scalar value;
        ssKey >> value;
        CSigmaSpendEntry zerocoinSpendItem;
        ssValue >> zerocoinSpendItem;
        listCoinSpendSerial.push_back(zerocoinSpendItem);
    }

    pcursor->close();
}

DBErrors CWalletDB::ReorderTransactions(CWallet *pwallet) {
    LOCK(pwallet->cs_wallet);
    // Old wallets didn't have any defined order for transactions
    // Probably a bad idea to change the output of this

    // First: get all CWalletTx and CAccountingEntry into a sorted-by-time multimap.
    typedef pair<CWalletTx *, CAccountingEntry *> TxPair;
    typedef multimap <int64_t, TxPair> TxItems;
    TxItems txByTime;

    for (map<uint256, CWalletTx>::iterator it = pwallet->mapWallet.begin(); it != pwallet->mapWallet.end(); ++it) {
        CWalletTx *wtx = &((*it).second);
        txByTime.insert(make_pair(wtx->nTimeReceived, TxPair(wtx, (CAccountingEntry *) 0)));
    }
    list <CAccountingEntry> acentries;
    ListAccountCreditDebit("", acentries);
    BOOST_FOREACH(CAccountingEntry & entry, acentries)
    {
        txByTime.insert(make_pair(entry.nTime, TxPair((CWalletTx *) 0, &entry)));
    }

    int64_t &nOrderPosNext = pwallet->nOrderPosNext;
    nOrderPosNext = 0;
    std::vector <int64_t> nOrderPosOffsets;
    for (TxItems::iterator it = txByTime.begin(); it != txByTime.end(); ++it) {
        CWalletTx *const pwtx = (*it).second.first;
        CAccountingEntry *const pacentry = (*it).second.second;
        int64_t &nOrderPos = (pwtx != 0) ? pwtx->nOrderPos : pacentry->nOrderPos;

        if (nOrderPos == -1) {
            nOrderPos = nOrderPosNext++;
            nOrderPosOffsets.push_back(nOrderPos);

            if (pwtx) {
                if (!WriteTx(*pwtx))
                    return DB_LOAD_FAIL;
            } else if (!WriteAccountingEntry(pacentry->nEntryNo, *pacentry))
                return DB_LOAD_FAIL;
        } else {
            int64_t nOrderPosOff = 0;
            BOOST_FOREACH(
            const int64_t &nOffsetStart, nOrderPosOffsets)
            {
                if (nOrderPos >= nOffsetStart)
                    ++nOrderPosOff;
            }
            nOrderPos += nOrderPosOff;
            nOrderPosNext = std::max(nOrderPosNext, nOrderPos + 1);

            if (!nOrderPosOff)
                continue;

            // Since we're changing the order, write it back
            if (pwtx) {
                if (!WriteTx(*pwtx))
                    return DB_LOAD_FAIL;
            } else if (!WriteAccountingEntry(pacentry->nEntryNo, *pacentry))
                return DB_LOAD_FAIL;
        }
    }
    WriteOrderPosNext(nOrderPosNext);

    return DB_LOAD_OK;
}

bool CWalletDB::WriteHDMint(const CHDMint& dMint)
{
    uint256 hash = dMint.GetPubCoinHash();
    return Write(make_pair(std::string("hdmint"), hash), dMint, true);
}

bool CWalletDB::ReadHDMint(const uint256& hashPubcoin, CHDMint& dMint)
{
    return Read(make_pair(std::string("hdmint"), hashPubcoin), dMint);
}

bool CWalletDB::EraseHDMint(const CHDMint& dMint) {
    nWalletDBUpdated++;
    uint256 hash = dMint.GetPubCoinHash();
    return Erase(std::make_pair(std::string("hdmint"), hash));
}

bool CWalletDB::HasHDMint(const secp_primitives::GroupElement& pub) {
    return Exists(std::make_pair(std::string("hdmint"), primitives::GetPubCoinValueHash(pub)));
}

class CWalletScanState {
public:
    unsigned int nKeys;
    unsigned int nCKeys;
    unsigned int nKeyMeta;
    bool fIsEncrypted;
    bool fAnyUnordered;
    int nFileVersion;
    vector <uint256> vWalletUpgrade;

    CWalletScanState() {
        nKeys = nCKeys = nKeyMeta = 0;
        fIsEncrypted = false;
        fAnyUnordered = false;
        nFileVersion = 0;
    }
};

bool ReadKeyValue(CWallet *pwallet, CDataStream &ssKey, CDataStream &ssValue,
                  CWalletScanState &wss, string &strType, string &strErr) {
    try {
        // Unserialize
        // Taking advantage of the fact that pair serialization
        // is just the two items serialized one after the other
        ssKey >> strType;
//        LogPrintf("ReadKeyValue(), strType=%s\n", strType);
        if (strType == "name") {
            string strAddress;
            ssKey >> strAddress;
            ssValue >> pwallet->mapAddressBook[CBitcoinAddress(strAddress).Get()].name;
        } else if (strType == "purpose") {
            string strAddress;
            ssKey >> strAddress;
            ssValue >> pwallet->mapAddressBook[CBitcoinAddress(strAddress).Get()].purpose;
        } else if (strType == "tx") {
            uint256 hash;
            ssKey >> hash;
            CWalletTx wtx;
            ssValue >> wtx;
            CValidationState state;
//            LogPrintf("CheckTransaction wtx.GetHash()=%s, hash=%s, state.IsValid()=%s\n", wtx.GetHash().ToString(),
//                      hash.ToString(), state.IsValid());
            if (!(CheckTransaction(wtx, state, wtx.GetHash(), true, INT_MAX, false, false) && (wtx.GetHash() == hash) &&
                  state.IsValid())) {
//                LogPrintf("ReadKeyValue|CheckTransaction(), wtx.GetHash() = &s\n", wtx.GetHash().ToString());
                return false;
            }
//            LogPrintf("done ->readkeyvalue()\n");
            // Undo serialize changes in 31600
            if (31404 <= wtx.fTimeReceivedIsTxTime && wtx.fTimeReceivedIsTxTime <= 31703) {
                if (!ssValue.empty()) {
                    char fTmp;
                    char fUnused;
                    ssValue >> fTmp >> fUnused >> wtx.strFromAccount;
                    strErr = strprintf("LoadWallet() upgrading tx ver=%d %d '%s' %s",
                                       wtx.fTimeReceivedIsTxTime, fTmp, wtx.strFromAccount, hash.ToString());
                    wtx.fTimeReceivedIsTxTime = fTmp;
                } else {
                    strErr = strprintf("LoadWallet() repairing tx ver=%d %s", wtx.fTimeReceivedIsTxTime,
                                       hash.ToString());
                    wtx.fTimeReceivedIsTxTime = 0;
                }
                wss.vWalletUpgrade.push_back(hash);
            }

            if (wtx.nOrderPos == -1)
                wss.fAnyUnordered = true;
            LogPrintf("readkeyvalue -> AddToWallet\n");
            pwallet->AddToWallet(wtx, true, NULL);
        } else if (strType == "acentry") {
            string strAccount;
            ssKey >> strAccount;
            uint64_t nNumber;
            ssKey >> nNumber;
            if (nNumber > nAccountingEntryNumber)
                nAccountingEntryNumber = nNumber;

            if (!wss.fAnyUnordered) {
                CAccountingEntry acentry;
                ssValue >> acentry;
                if (acentry.nOrderPos == -1)
                    wss.fAnyUnordered = true;
            }
        } else if (strType == "watchs") {
            CScript script;
            ssKey >> *(CScriptBase * )(&script);
            char fYes;
            ssValue >> fYes;
            if (fYes == '1')
                pwallet->LoadWatchOnly(script);

            // Watch-only addresses have no birthday information for now,
            // so set the wallet birthday to the beginning of time.
            pwallet->nTimeFirstKey = 1;
        } else if (strType == "key" || strType == "wkey") {
            CPubKey vchPubKey;
            ssKey >> vchPubKey;
            if (!vchPubKey.IsValid()) {
                strErr = "Error reading wallet database: CPubKey corrupt";
                return false;
            }
            CKey key;
            CPrivKey pkey;
            uint256 hash;

            if (strType == "key") {
                wss.nKeys++;
                ssValue >> pkey;
            } else {
                CWalletKey wkey;
                ssValue >> wkey;
                pkey = wkey.vchPrivKey;
            }

            // Old wallets store keys as "key" [pubkey] => [privkey]
            // ... which was slow for wallets with lots of keys, because the public key is re-derived from the private key
            // using EC operations as a checksum.
            // Newer wallets store keys as "key"[pubkey] => [privkey][hash(pubkey,privkey)], which is much faster while
            // remaining backwards-compatible.
            try {
                ssValue >> hash;
            }
            catch (...) {}

            bool fSkipCheck = false;

            if (!hash.IsNull()) {
                // hash pubkey/privkey to accelerate wallet load
                std::vector<unsigned char> vchKey;
                vchKey.reserve(vchPubKey.size() + pkey.size());
                vchKey.insert(vchKey.end(), vchPubKey.begin(), vchPubKey.end());
                vchKey.insert(vchKey.end(), pkey.begin(), pkey.end());

                if (Hash(vchKey.begin(), vchKey.end()) != hash) {
                    strErr = "Error reading wallet database: CPubKey/CPrivKey corrupt";
                    return false;
                }

                fSkipCheck = true;
            }

            if (!key.Load(pkey, vchPubKey, fSkipCheck)) {
                strErr = "Error reading wallet database: CPrivKey corrupt";
                return false;
            }
            if (!pwallet->LoadKey(key, vchPubKey)) {
                strErr = "Error reading wallet database: LoadKey failed";
                return false;
            }
        } else if (strType == "mkey") {
            unsigned int nID;
            ssKey >> nID;
            CMasterKey kMasterKey;
            ssValue >> kMasterKey;
            if (pwallet->mapMasterKeys.count(nID) != 0) {
                strErr = strprintf("Error reading wallet database: duplicate CMasterKey id %u", nID);
                return false;
            }
            pwallet->mapMasterKeys[nID] = kMasterKey;
            if (pwallet->nMasterKeyMaxID < nID)
                pwallet->nMasterKeyMaxID = nID;
        } else if (strType == "ckey") {
            CPubKey vchPubKey;
            ssKey >> vchPubKey;
            if (!vchPubKey.IsValid()) {
                strErr = "Error reading wallet database: CPubKey corrupt";
                return false;
            }
            vector<unsigned char> vchPrivKey;
            ssValue >> vchPrivKey;
            wss.nCKeys++;

            if (!pwallet->LoadCryptedKey(vchPubKey, vchPrivKey)) {
                strErr = "Error reading wallet database: LoadCryptedKey failed";
                return false;
            }
            wss.fIsEncrypted = true;
        } else if (strType == "keymeta") {
            CPubKey vchPubKey;
            ssKey >> vchPubKey;
            CKeyMetadata keyMeta;
            ssValue >> keyMeta;
            wss.nKeyMeta++;

            pwallet->LoadKeyMetadata(vchPubKey, keyMeta);

            // find earliest key creation time, as wallet birthday
            if (!pwallet->nTimeFirstKey ||
                (keyMeta.nCreateTime < pwallet->nTimeFirstKey))
                pwallet->nTimeFirstKey = keyMeta.nCreateTime;
        } else if (strType == "defaultkey") {
            ssValue >> pwallet->vchDefaultKey;
        } else if (strType == "pool") {
            int64_t nIndex;
            ssKey >> nIndex;
            CKeyPool keypool;
            ssValue >> keypool;
            pwallet->setKeyPool.insert(nIndex);

            // If no metadata exists yet, create a default with the pool key's
            // creation time. Note that this may be overwritten by actually
            // stored metadata for that key later, which is fine.
            CKeyID keyid = keypool.vchPubKey.GetID();
            if (pwallet->mapKeyMetadata.count(keyid) == 0)
                pwallet->mapKeyMetadata[keyid] = CKeyMetadata(keypool.nTime);
        } else if (strType == "version") {
            ssValue >> wss.nFileVersion;
            if (wss.nFileVersion == 10300)
                wss.nFileVersion = 300;
        } else if (strType == "cscript") {
            uint160 hash;
            ssKey >> hash;
            CScript script;
            ssValue >> *(CScriptBase * )(&script);
            if (!pwallet->LoadCScript(script)) {
                strErr = "Error reading wallet database: LoadCScript failed";
                return false;
            }
        } else if (strType == "orderposnext") {
            ssValue >> pwallet->nOrderPosNext;
        } else if (strType == "destdata") {
            std::string strAddress, strKey, strValue;
            ssKey >> strAddress;
            ssKey >> strKey;
            ssValue >> strValue;
            if (!pwallet->LoadDestData(CBitcoinAddress(strAddress).Get(), strKey, strValue)) {
                strErr = "Error reading wallet database: LoadDestData failed";
                return false;
            }
        } else if (strType == "hdchain") {
            CHDChain chain;
            ssValue >> chain;
            if (!pwallet->SetHDChain(chain, true)) {
                strErr = "Error reading wallet database: SetHDChain failed";
                return false;
            }
        }
    } catch (...) {
        return false;
    }
    return true;
}

static bool IsKeyType(string strType) {
    return (strType == "key" || strType == "wkey" ||
            strType == "mkey" || strType == "ckey");
}

DBErrors CWalletDB::LoadWallet(CWallet *pwallet) {
    LogPrintf("CWalletDB::LoadWallet()\n");
    pwallet->vchDefaultKey = CPubKey();
    CWalletScanState wss;
    bool fNoncriticalErrors = false;
    DBErrors result = DB_LOAD_OK;

    try {
        LOCK2(cs_main, pwallet->cs_wallet);
        int nMinVersion = 0;
        if (Read((string) "minversion", nMinVersion)) {
            if (nMinVersion > CLIENT_VERSION)
                return DB_TOO_NEW;
            pwallet->LoadMinVersion(nMinVersion);
        }

        // Get cursor
        Dbc *pcursor = GetCursor();
        if (!pcursor) {
            LogPrintf("Error getting wallet database cursor\n");
            return DB_CORRUPT;
        }

        while (true) {
            // Read next record
            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
            CDataStream ssValue(SER_DISK, CLIENT_VERSION);
            int ret = ReadAtCursor(pcursor, ssKey, ssValue);
            if (ret == DB_NOTFOUND)
                break;
            else if (ret != 0) {
                LogPrintf("Error reading next record from wallet database\n");
                return DB_CORRUPT;
            }

            // Try to be tolerant of single corrupt records:
            string strType, strErr;
            if (!ReadKeyValue(pwallet, ssKey, ssValue, wss, strType, strErr)) {
                // losing keys is considered a catastrophic error, anything else
                // we assume the user can live with:
                if (IsKeyType(strType))
                    result = DB_CORRUPT;
                else {
                    // Leave other errors alone, if we try to fix them we might make things worse.
                    LogPrintf("ReadKeyValue() failed, strType=%s\n", strType);
                    // GravityCoin - MTP
                    // Need Peter to take a look
                    //fNoncriticalErrors = true; // ... but do warn the user there is something wrong.
                    if (strType == "tx")
                        // Rescan if there is a bad transaction record:
                        SoftSetBoolArg("-rescan", true);
                }
            }
            if (!strErr.empty())
                LogPrintf("%s\n", strErr);
        }
        pcursor->close();
    }
    catch (const boost::thread_interrupted &) {
        throw;
    }
    catch (...) {
        result = DB_CORRUPT;
    }

    if (fNoncriticalErrors && result == DB_LOAD_OK)
        result = DB_NONCRITICAL_ERROR;

    // Any wallet corruption at all: skip any rewriting or
    // upgrading, we don't want to make it worse.
    if (result != DB_LOAD_OK)
        return result;

    LogPrintf("nFileVersion = %d\n", wss.nFileVersion);

    LogPrintf("Keys: %u plaintext, %u encrypted, %u w/ metadata, %u total\n",
              wss.nKeys, wss.nCKeys, wss.nKeyMeta, wss.nKeys + wss.nCKeys);

    // nTimeFirstKey is only reliable if all keys have metadata
    if ((wss.nKeys + wss.nCKeys) != wss.nKeyMeta)
        pwallet->nTimeFirstKey = 1; // 0 would be considered 'no value'

    BOOST_FOREACH(uint256
    hash, wss.vWalletUpgrade)
    WriteTx(pwallet->mapWallet[hash]);

    // Rewrite encrypted wallets of versions 0.4.0 and 0.5.0rc:
    if (wss.fIsEncrypted && (wss.nFileVersion == 40000 || wss.nFileVersion == 50000))
        return DB_NEED_REWRITE;

    if (wss.nFileVersion < CLIENT_VERSION) // Update
        WriteVersion(CLIENT_VERSION);

    if (wss.fAnyUnordered)
        result = ReorderTransactions(pwallet);

    pwallet->laccentries.clear();
    ListAccountCreditDebit("*", pwallet->laccentries);
    BOOST_FOREACH(CAccountingEntry & entry, pwallet->laccentries)
    {
        pwallet->wtxOrdered.insert(make_pair(entry.nOrderPos, CWallet::TxPair((CWalletTx *) 0, &entry)));
    }

    return result;
}

DBErrors CWalletDB::FindWalletTx(CWallet *pwallet, vector <uint256> &vTxHash, vector <CWalletTx> &vWtx) {
    pwallet->vchDefaultKey = CPubKey();
    bool fNoncriticalErrors = false;
    DBErrors result = DB_LOAD_OK;

    try {
        LOCK(pwallet->cs_wallet);
        int nMinVersion = 0;
        if (Read((string) "minversion", nMinVersion)) {
            if (nMinVersion > CLIENT_VERSION)
                return DB_TOO_NEW;
            pwallet->LoadMinVersion(nMinVersion);
        }

        // Get cursor
        Dbc *pcursor = GetCursor();
        if (!pcursor) {
            LogPrintf("Error getting wallet database cursor\n");
            return DB_CORRUPT;
        }

        while (true) {
            // Read next record
            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
            CDataStream ssValue(SER_DISK, CLIENT_VERSION);
            int ret = ReadAtCursor(pcursor, ssKey, ssValue);
            if (ret == DB_NOTFOUND)
                break;
            else if (ret != 0) {
                LogPrintf("Error reading next record from wallet database\n");
                return DB_CORRUPT;
            }

            string strType;
            ssKey >> strType;
            if (strType == "tx") {
                uint256 hash;
                ssKey >> hash;

                CWalletTx wtx;
                ssValue >> wtx;

                vTxHash.push_back(hash);
                vWtx.push_back(wtx);
            }
        }
        pcursor->close();
    }
    catch (const boost::thread_interrupted &) {
        throw;
    }
    catch (...) {
        result = DB_CORRUPT;
    }

    if (fNoncriticalErrors && result == DB_LOAD_OK)
        result = DB_NONCRITICAL_ERROR;

    return result;
}

DBErrors CWalletDB::ZapSelectTx(CWallet *pwallet, vector <uint256> &vTxHashIn, vector <uint256> &vTxHashOut) {
    // build list of wallet TXs and hashes
    vector <uint256> vTxHash;
    vector <CWalletTx> vWtx;
    DBErrors err = FindWalletTx(pwallet, vTxHash, vWtx);
    if (err != DB_LOAD_OK) {
        return err;
    }

    std::sort(vTxHash.begin(), vTxHash.end());
    std::sort(vTxHashIn.begin(), vTxHashIn.end());

    // erase each matching wallet TX
    bool delerror = false;
    vector<uint256>::iterator it = vTxHashIn.begin();
    BOOST_FOREACH(uint256
    hash, vTxHash) {
        while (it < vTxHashIn.end() && (*it) < hash) {
            it++;
        }
        if (it == vTxHashIn.end()) {
            break;
        } else if ((*it) == hash) {
            pwallet->mapWallet.erase(hash);
            if (!EraseTx(hash)) {
                LogPrint("db", "Transaction was found for deletion but returned database error: %s\n", hash.GetHex());
                delerror = true;
            }
            vTxHashOut.push_back(hash);
        }
    }

    if (delerror) {
        return DB_CORRUPT;
    }
    return DB_LOAD_OK;
}

DBErrors CWalletDB::ZapSigmaMints(CWallet *pwallet) {
    // get list of HD Mints
    std::list<CHDMint> vHDMints = ListHDMints();

    // get list of non HD Mints
    std::list <CSigmaEntry> sigmaEntries;
    ListSigmaPubCoin(sigmaEntries);

    // erase each HD Mint
    BOOST_FOREACH(CHDMint & hdMint, vHDMints)
    {
        if (!EraseHDMint(hdMint))
            return DB_CORRUPT;
    }

    // erase each non HD Mint
    BOOST_FOREACH(CSigmaEntry & sigmaEntry, sigmaEntries)
    {
        if (!EraseZerocoinEntry(sigmaEntry))
            return DB_CORRUPT;
    }

    return DB_LOAD_OK;
}

DBErrors CWalletDB::ZapWalletTx(CWallet *pwallet, vector <CWalletTx> &vWtx) {
    // build list of wallet TXs
    vector <uint256> vTxHash;
    DBErrors err = FindWalletTx(pwallet, vTxHash, vWtx);
    if (err != DB_LOAD_OK)
        return err;

    // erase each wallet TX
    BOOST_FOREACH(uint256 & hash, vTxHash)
    {
        if (!EraseTx(hash))
            return DB_CORRUPT;
    }

    return DB_LOAD_OK;
}

void ThreadFlushWalletDB(const string &strFile) {
    // Make this thread recognisable as the wallet flushing thread
    RenameThread("bitcoin-wallet");

    static bool fOneThread;
    if (fOneThread)
        return;
    fOneThread = true;
    if (!GetBoolArg("-flushwallet", DEFAULT_FLUSHWALLET))
        return;

    unsigned int nLastSeen = nWalletDBUpdated;
    unsigned int nLastFlushed = nWalletDBUpdated;
    int64_t nLastWalletUpdate = GetTime();
    while (true) {
        MilliSleep(500);

        if (nLastSeen != nWalletDBUpdated) {
            nLastSeen = nWalletDBUpdated;
            nLastWalletUpdate = GetTime();
        }

        if (nLastFlushed != nWalletDBUpdated && GetTime() - nLastWalletUpdate >= 2) {
            TRY_LOCK(bitdb.cs_db, lockDb);
            if (lockDb) {
                // Don't do this if any databases are in use
                int nRefCount = 0;
                map<string, int>::iterator mi = bitdb.mapFileUseCount.begin();
                while (mi != bitdb.mapFileUseCount.end()) {
                    nRefCount += (*mi).second;
                    mi++;
                }

                if (nRefCount == 0) {
                    boost::this_thread::interruption_point();
                    map<string, int>::iterator mi = bitdb.mapFileUseCount.find(strFile);
                    if (mi != bitdb.mapFileUseCount.end()) {
                        LogPrint("db", "Flushing %s\n", strFile);
                        nLastFlushed = nWalletDBUpdated;
                        int64_t nStart = GetTimeMillis();

                        // Flush wallet file so it's self contained
                        bitdb.CloseDb(strFile);
                        bitdb.CheckpointLSN(strFile);

                        bitdb.mapFileUseCount.erase(mi++);
                        LogPrint("db", "Flushed %s %dms\n", strFile, GetTimeMillis() - nStart);
                    }
                }
            }
        }
    }
}

// This should be called carefully:
// either supply "wallet" (if already loaded) or "strWalletFile" (if wallet wasn't loaded yet)
bool AutoBackupWallet (CWallet* wallet, std::string strWalletFile, std::string& strBackupWarning, std::string& strBackupError)
{
    namespace fs = boost::filesystem;

    strBackupWarning = strBackupError = "";

    if(nWalletBackups > 0)
    {
        fs::path backupsDir = GetBackupsDir();

        if (!fs::exists(backupsDir))
        {
            // Always create backup folder to not confuse the operating system's file browser
            LogPrintf("Creating backup folder %s\n", backupsDir.string());
            if(!fs::create_directories(backupsDir)) {
                // smth is wrong, we shouldn't continue until it's resolved
                strBackupError = strprintf(_("Wasn't able to create wallet backup folder %s!"), backupsDir.string());
                LogPrintf("%s\n", strBackupError);
                nWalletBackups = -1;
                return false;
            }
        }

        // Create backup of the ...
        std::string dateTimeStr = DateTimeStrFormat(".%Y-%m-%d-%H-%M", GetTime());
        if (wallet)
        {
            // ... opened wallet
            LOCK2(cs_main, wallet->cs_wallet);
            strWalletFile = wallet->strWalletFile;
            fs::path backupFile = backupsDir / (strWalletFile + dateTimeStr);
//            if(!BackupWallet(*wallet, backupFile.string())) {
//                strBackupWarning = strprintf(_("Failed to create backup %s!"), backupFile.string());
//                LogPrintf("%s\n", strBackupWarning);
//                nWalletBackups = -1;
//                return false;
//            }
            // Update nKeysLeftSinceAutoBackup using current pool size
            wallet->nKeysLeftSinceAutoBackup = wallet->GetKeyPoolSize();
            LogPrintf("nKeysLeftSinceAutoBackup: %d\n", wallet->nKeysLeftSinceAutoBackup);
            if(wallet->IsLocked(true)) {
                strBackupWarning = _("Wallet is locked, can't replenish keypool! Automatic backups and mixing are disabled, please unlock your wallet to replenish keypool.");
                LogPrintf("%s\n", strBackupWarning);
                nWalletBackups = -2;
                return false;
            }
        } else {
            // ... strWalletFile file
            fs::path sourceFile = GetDataDir() / strWalletFile;
            fs::path backupFile = backupsDir / (strWalletFile + dateTimeStr);
            sourceFile.make_preferred();
            backupFile.make_preferred();
            if (fs::exists(backupFile))
            {
                strBackupWarning = _("Failed to create backup, file already exists! This could happen if you restarted wallet in less than 60 seconds. You can continue if you are ok with this.");
                LogPrintf("%s\n", strBackupWarning);
                return false;
            }
            if(fs::exists(sourceFile)) {
                try {
                    fs::copy_file(sourceFile, backupFile);
                    LogPrintf("Creating backup of %s -> %s\n", sourceFile.string(), backupFile.string());
                } catch(fs::filesystem_error &error) {
                    strBackupWarning = strprintf(_("Failed to create backup, error: %s"), error.what());
                    LogPrintf("%s\n", strBackupWarning);
                    nWalletBackups = -1;
                    return false;
                }
            }
        }

        // Keep only the last 10 backups, including the new one of course
        typedef std::multimap<std::time_t, fs::path> folder_set_t;
        folder_set_t folder_set;
        fs::directory_iterator end_iter;
        backupsDir.make_preferred();
        // Build map of backup files for current(!) wallet sorted by last write time
        fs::path currentFile;
        for (fs::directory_iterator dir_iter(backupsDir); dir_iter != end_iter; ++dir_iter)
        {
            // Only check regular files
            if ( fs::is_regular_file(dir_iter->status()))
            {
                currentFile = dir_iter->path().filename();
                // Only add the backups for the current wallet, e.g. wallet.dat.*
                if(dir_iter->path().stem().string() == strWalletFile)
                {
                    folder_set.insert(folder_set_t::value_type(fs::last_write_time(dir_iter->path()), *dir_iter));
                }
            }
        }

        // Loop backward through backup files and keep the N newest ones (1 <= N <= 10)
        int counter = 0;
        BOOST_REVERSE_FOREACH(PAIRTYPE(const std::time_t, fs::path) file, folder_set)
        {
            counter++;
            if (counter > nWalletBackups)
            {
                // More than nWalletBackups backups: delete oldest one(s)
                try {
                    fs::remove(file.second);
                    LogPrintf("Old backup deleted: %s\n", file.second);
                } catch(fs::filesystem_error &error) {
                    strBackupWarning = strprintf(_("Failed to delete backup, error: %s"), error.what());
                    LogPrintf("%s\n", strBackupWarning);
                    return false;
                }
            }
        }
        return true;
    }

    LogPrintf("Automatic wallet backups are disabled!\n");
    return false;
}

//
// Try to (very carefully!) recover wallet file if there is a problem.
//
bool CWalletDB::Recover(CDBEnv &dbenv, const std::string &filename, bool fOnlyKeys) {
    // Recovery procedure:
    // move wallet file to wallet.timestamp.bak
    // Call Salvage with fAggressive=true to
    // get as much data as possible.
    // Rewrite salvaged data to fresh wallet file
    // Set -rescan so any missing transactions will be
    // found.
    LogPrintf("CWalletDB::Recover\n");
    int64_t now = GetTime();
    std::string newFilename = strprintf("wallet.%d.bak", now);

    int result = dbenv.dbenv->dbrename(NULL, filename.c_str(), NULL,
                                       newFilename.c_str(), DB_AUTO_COMMIT);
    if (result == 0)
        LogPrintf("Renamed %s to %s\n", filename, newFilename);
    else {
        LogPrintf("Failed to rename %s to %s\n", filename, newFilename);
        return false;
    }

    std::vector <CDBEnv::KeyValPair> salvagedData;
    bool fSuccess = dbenv.Salvage(newFilename, true, salvagedData);
    if (salvagedData.empty()) {
        LogPrintf("Salvage(aggressive) found no records in %s.\n", newFilename);
        return false;
    }
    LogPrintf("Salvage(aggressive) found %u records\n", salvagedData.size());

    boost::scoped_ptr <Db> pdbCopy(new Db(dbenv.dbenv, 0));
    int ret = pdbCopy->open(NULL,               // Txn pointer
                            filename.c_str(),   // Filename
                            "main",             // Logical db name
                            DB_BTREE,           // Database type
                            DB_CREATE,          // Flags
                            0);
    if (ret > 0) {
        LogPrintf("Cannot create database file %s\n", filename);
        return false;
    }
    CWallet dummyWallet;
    CWalletScanState wss;

    DbTxn *ptxn = dbenv.TxnBegin();
    BOOST_FOREACH(CDBEnv::KeyValPair & row, salvagedData)
    {
        if (fOnlyKeys) {
            CDataStream ssKey(row.first, SER_DISK, CLIENT_VERSION);
            CDataStream ssValue(row.second, SER_DISK, CLIENT_VERSION);
            string strType, strErr;
            bool fReadOK;
            {
                // Required in LoadKeyMetadata():
                LOCK(dummyWallet.cs_wallet);
                fReadOK = ReadKeyValue(&dummyWallet, ssKey, ssValue,
                                       wss, strType, strErr);
            }
            if (!IsKeyType(strType) && strType != "hdchain")
                continue;
            if (!fReadOK) {
                LogPrintf("WARNING: CWalletDB::Recover skipping %s: %s\n", strType, strErr);
                continue;
            }
        }
        Dbt datKey(&row.first[0], row.first.size());
        Dbt datValue(&row.second[0], row.second.size());
        int ret2 = pdbCopy->put(ptxn, &datKey, &datValue, DB_NOOVERWRITE);
        if (ret2 > 0)
            fSuccess = false;
    }
    ptxn->commit(0);
    pdbCopy->close(0);

    return fSuccess;
}

bool CWalletDB::Recover(CDBEnv &dbenv, const std::string &filename) {
    return CWalletDB::Recover(dbenv, filename, false);
}

bool CWalletDB::WriteDestData(const std::string &address, const std::string &key, const std::string &value) {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("destdata"), std::make_pair(address, key)), value);
}

bool CWalletDB::EraseDestData(const std::string &address, const std::string &key) {
    nWalletDBUpdated++;
    return Erase(std::make_pair(std::string("destdata"), std::make_pair(address, key)));
}


bool CWalletDB::WriteHDChain(const CHDChain &chain) {
    nWalletDBUpdated++;
    return Write(std::string("hdchain"), chain);
}

bool CWalletDB::ReadZerocoinCount(int32_t& nCount)
{
    return Read(string("dzc"), nCount);
}

bool CWalletDB::WriteZerocoinCount(const int32_t& nCount)
{
    return Write(string("dzc"), nCount);
}

bool CWalletDB::ReadZerocoinSeedCount(int32_t& nCount)
{
    return Read(string("dzsc"), nCount);
}

bool CWalletDB::WriteZerocoinSeedCount(const int32_t& nCount)
{
    return Write(string("dzsc"), nCount);
}

bool CWalletDB::WritePubcoin(const uint256& hashSerial, const GroupElement& pubcoin)
{
    return Write(make_pair(string("pubcoin"), hashSerial), pubcoin);
}

bool CWalletDB::ReadPubcoin(const uint256& hashSerial, GroupElement& pubcoin)
{
    return Read(make_pair(string("pubcoin"), hashSerial), pubcoin);
}

bool CWalletDB::ErasePubcoin(const uint256& hashSerial)
{
    return Erase(make_pair(string("pubcoin"), hashSerial));
}

std::vector<std::pair<uint256, GroupElement>> CWalletDB::ListSerialPubcoinPairs()
{
    std::vector<std::pair<uint256, GroupElement>> listSerialPubcoin;
    Dbc* pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error(std::string(__func__)+" : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    for (;;)
    {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << make_pair(string("pubcoin"), ArithToUint256(arith_uint256(0)));
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0)
        {
            pcursor->close();
            throw runtime_error(std::string(__func__)+" : error scanning DB");
        }

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "pubcoin")
            break;

        uint256 hashSerial;
        ssKey >> hashSerial;

        GroupElement pubcoin;
        ssValue >> pubcoin;

        listSerialPubcoin.push_back(make_pair(hashSerial, pubcoin));
    }

    pcursor->close();

    return listSerialPubcoin;

}

bool CWalletDB::EraseMintPoolPair(const uint256& hashPubcoin)
{
    return Erase(make_pair(string("mintpool"), hashPubcoin));
}

bool CWalletDB::WriteMintPoolPair(const uint256& hashPubcoin, const std::tuple<uint160, CKeyID, int32_t>& hashSeedMintPool)
{
    return Write(make_pair(string("mintpool"), hashPubcoin), hashSeedMintPool);
}

bool CWalletDB::ReadMintPoolPair(const uint256& hashPubcoin, uint160& hashSeedMaster, CKeyID& seedId, int32_t& nCount)
{
    std::tuple<uint160, CKeyID, int32_t> hashSeedMintPool;
    if(!Read(make_pair(string("mintpool"), hashPubcoin), hashSeedMintPool))
        return false;
    hashSeedMaster = get<0>(hashSeedMintPool);
    seedId = get<1>(hashSeedMintPool);
    nCount = get<2>(hashSeedMintPool);
    return true;
}

//! list of MintPoolEntry objects mapped with pubCoin hash, returned as pairs
std::vector<std::pair<uint256, MintPoolEntry>> CWalletDB::ListMintPool()
{
    std::vector<std::pair<uint256, MintPoolEntry>> listPool;
    Dbc* pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error(std::string(__func__)+" : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    for (;;)
    {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << make_pair(string("mintpool"), ArithToUint256(arith_uint256(0)));
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0)
        {
            pcursor->close();
            throw runtime_error(std::string(__func__)+" : error scanning DB");
        }

        // Unserialize

        try {
            string strType;
            ssKey >> strType;
            if (strType != "mintpool")
                break;

            uint256 hashPubcoin;
            ssKey >> hashPubcoin;

            uint160 hashSeedMaster;
            ssValue >> hashSeedMaster;

            CKeyID seedId;
            ssValue >> seedId;

            int32_t nCount;
            ssValue >> nCount;

            MintPoolEntry mintPoolEntry(hashSeedMaster, seedId, nCount);

            listPool.push_back(make_pair(hashPubcoin, mintPoolEntry));
        } catch (std::ios_base::failure const &) {
            // There maybe some old entries that don't conform to the latest version. Just skipping those.
        }
    }

    pcursor->close();

    return listPool;
}

std::list<CHDMint> CWalletDB::ListHDMints()
{
    std::list<CHDMint> listMints;
    Dbc* pcursor = GetCursor();
    if (!pcursor)
        throw runtime_error(std::string(__func__)+" : cannot create DB cursor");
    unsigned int fFlags = DB_SET_RANGE;
    for (;;)
    {
        // Read next record
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        if (fFlags == DB_SET_RANGE)
            ssKey << make_pair(string("hdmint"), ArithToUint256(arith_uint256(0)));
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
        fFlags = DB_NEXT;
        if (ret == DB_NOTFOUND)
            break;
        else if (ret != 0)
        {
            pcursor->close();
            throw runtime_error(std::string(__func__)+" : error scanning DB");
        }

        // Unserialize
        string strType;
        ssKey >> strType;
        if (strType != "hdmint")
            break;

        uint256 hashPubcoin;
        ssKey >> hashPubcoin;

        CHDMint mint;
        ssValue >> mint;

        listMints.emplace_back(mint);
    }

    pcursor->close();
    return listMints;
}

bool CWalletDB::ArchiveMintOrphan(const CZerocoinEntry& zerocoin)
{
    CDataStream ss(SER_GETHASH, 0);
    ss << zerocoin.value;
    uint256 hash = Hash(ss.begin(), ss.end());;

    if (!Write(make_pair(string("zco"), hash), zerocoin)) {
        LogPrintf("%s : failed to database orphaned zerocoin mint\n", __func__);
        return false;
    }

    if (!Erase(make_pair(string("zerocoin"), hash))) {
        LogPrintf("%s : failed to erase orphaned zerocoin mint\n", __func__);
        return false;
    }

    return true;
}

bool CWalletDB::ArchiveDeterministicOrphan(const CHDMint& dMint)
{
    if (!Write(make_pair(string("dzco"), dMint.GetPubCoinHash()), dMint))
        return error("%s: write failed", __func__);

    if (!Erase(make_pair(string("hdmint"), dMint.GetPubCoinHash())))
        return error("%s: failed to erase", __func__);

    return true;
}

bool CWalletDB::UnarchiveHDMint(const uint256& hashPubcoin, CHDMint& dMint)
{
    if (!Read(make_pair(string("dzco"), hashPubcoin), dMint))
        return error("%s: failed to retrieve deterministic mint from archive", __func__);

    if (!WriteHDMint(dMint))
        return error("%s: failed to write deterministic mint", __func__);

    if (!Erase(make_pair(string("dzco"), dMint.GetPubCoinHash())))
        return error("%s : failed to erase archived deterministic mint", __func__);

    return true;
}

bool CWalletDB::UnarchiveZerocoinMint(const uint256& hashPubcoin, CSigmaEntry& zerocoin)
{
    if (!Read(make_pair(string("zco"), hashPubcoin), zerocoin))
        return error("%s: failed to retrieve zerocoinmint from archive", __func__);

    if (!WriteZerocoinEntry(zerocoin))
        return error("%s: failed to write zerocoinmint", __func__);

    uint256 hash = primitives::GetPubCoinValueHash(zerocoin.value);
    if (!Erase(make_pair(string("zco"), hash)))
        return error("%s : failed to erase archived zerocoin mint", __func__);

    return true;
}

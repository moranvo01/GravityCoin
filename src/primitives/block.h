// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include <deque>
#include <type_traits>
#include <boost/foreach.hpp>
#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"
#include "definition.h"
#include "zerocoin_params.h"

// Can't include sigma.h
namespace sigma {
class CSigmaTxInfo;

} // namespace sigma.

inline int GetZerocoinChainID()
{
    return 0x0001; // We are the first :)
}

class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    static const int CURRENT_VERSION = 2;

    // uint32_t lastHeight;
    uint256 powHash;
    int32_t isComputed;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    class CSerializeBlockHeader {};
    class CReadBlockHeader : public CSerActionUnserialize, public CSerializeBlockHeader {};
    class CWriteBlockHeader : public CSerActionSerialize, public CSerializeBlockHeader {};

    template <typename Stream, typename Operation, typename = typename std::enable_if<!std::is_base_of<CSerializeBlockHeader,Operation>::value>::type>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    template <typename Stream>
    inline void SerializationOp(Stream &s, CReadBlockHeader ser_action, int nType, int) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    void SetNull()
    {
        nVersion = CBlockHeader::CURRENT_VERSION | (GetZerocoinChainID() * BLOCK_VERSION_CHAIN_START);
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        isComputed = -1;
        powHash.SetNull();
    }

    int GetChainID() const
    {
        return nVersion / BLOCK_VERSION_CHAIN_START;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    bool IsComputed() const
    {
        return (isComputed <= 0);
    }

    void SetPoWHash(uint256 hash) const
    {
//        isComputed = 1;
//        powHash = hash;
    }

    uint256 GetPoWHash(int nHeight, bool forceCalc = false) const;

    uint256 GetHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
    void InvalidateCachedPoWHash(int nHeight) const;

};

class CZerocoinTxInfo;

class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // memory only
    mutable CTxOut txoutXnode; // xnode payment
    mutable bool fChecked;

    // memory only, zerocoin tx info
    mutable std::shared_ptr<CZerocoinTxInfo> zerocoinTxInfo;

    // memory only, zerocoin tx info after V3-sigma.
    mutable std::shared_ptr<sigma::CSigmaTxInfo> sigmaTxInfo;

    CBlock()
    {
        zerocoinTxInfo = NULL;
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        zerocoinTxInfo = NULL;
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ~CBlock() {
        ZerocoinClean();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    template <typename Stream>
    inline void SerializationOp(Stream &s, CReadBlockHeader ser_action, int nType, int nVersion) {
        CBlockHeader::SerializationOp(s, ser_action, nType, nVersion);
    }

    void SetNull()
    {
        ZerocoinClean();
        CBlockHeader::SetNull();
        vtx.clear();
        txoutXnode = CTxOut();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;

        return block;
    }

    std::string ToString() const;

    void ZerocoinClean() const;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

/** Compute the consensus-critical block weight (see BIP 141). */
int64_t GetBlockWeight(const CBlock& tx);

#endif // BITCOIN_PRIMITIVES_BLOCK_H

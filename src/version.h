// Copyright (c) 2012-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_VERSION_H
#define BITCOIN_VERSION_H

/**
 * network protocol versioning
 */

static const int PROTOCOL_VERSION = 99029;

//! initial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 90013;

//! In this version, 'getheaders' was introduced.
static const int GETHEADERS_VERSION = 90020;

//! disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION = 99028;

//! disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION_SPORK = 99029;

//! nTime field added to CAddress, starting with this version;
//! if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSION = 90013;

//! BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 90013;

//! "mempool" command, enhanced "getdata" behavior starts with this version
static const int MEMPOOL_GD_VERSION = 90013;

//! "filter*" commands are disabled without NODE_BLOOM after and including this version
static const int NO_BLOOM_VERSION = 90013;

//! "sendheaders" command and announcing blocks with headers starts with this version
static const int SENDHEADERS_VERSION = 90013;

//! "feefilter" tells peers to filter invs to you by fee starts with this version
static const int FEEFILTER_VERSION = 90013;

//! shord-id-based block download starts with this version
static const int SHORT_IDS_BLOCKS_VERSION = 90013;

//! not banning for invalid compact blocks starts with this version
static const int INVALID_CB_NO_BAN_VERSION = 90013;

#endif // BITCOIN_VERSION_H

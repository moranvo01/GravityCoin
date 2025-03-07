# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue tor
SUBDIRS = tor

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

TOR_LIBS = tor/src/core/libtor-app.a \
    tor/src/lib/libtor-meminfo.a \
    tor/src/lib/libtor-term.a \
    tor/src/lib/libtor-osinfo.a \
    tor/src/lib/libtor-geoip.a \
    tor/src/lib/libtor-math.a \
    tor/src/lib/libtor-tls.a \
    tor/src/lib/libtor-process.a \
    tor/src/lib/libtor-evloop.a \
    tor/src/lib/libtor-thread.a \
    tor/src/lib/libtor-compress.a \
    tor/src/lib/libtor-net.a \
    tor/src/lib/libtor-buf.a \
    tor/src/lib/libtor-time.a \
    tor/src/lib/libtor-err.a \
    tor/src/lib/libtor-log.a \
    tor/src/lib/libtor-version.a \
    tor/src/lib/libtor-container.a \
    tor/src/lib/libtor-crypt-ops.a \
    tor/src/lib/libtor-fs.a \
    tor/src/lib/libtor-fdio.a \
    tor/src/lib/libtor-sandbox.a \
    tor/src/lib/libtor-memarea.a \
    tor/src/lib/libtor-intmath.a \
    tor/src/lib/libtor-encoding.a \
    tor/src/lib/libtor-smartlist-core.a \
    tor/src/lib/libtor-lock.a \
    tor/src/lib/libtor-wallclock.a \
    tor/src/lib/libtor-string.a \
    tor/src/lib/libtor-malloc.a \
	tor/src/lib/libtor-ctime.a \
	tor/src/trunnel/libor-trunnel.a \
	tor/src/lib/libcurve25519_donna.a \
	tor/src/ext/ed25519/donna/libed25519_donna.a \
	tor/src/ext/ed25519/ref10/libed25519_ref10.a \
	tor/src/ext/keccak-tiny/libkeccak-tiny.a

tor.timestamp: $(wildcard tor/src/core/or/*.c) $(wildcard tor/src/core/or/*.h) \
		$(wildcard tor/src/lib/*.c) $(wildcard tor/src/lib/*.h) \
		$(wildcard tor/src/trunnel/*.c) $(wildcard tor/src/trunnel/*.h) \
		$(wildcard tor/src/ext/*.c) $(wildcard tor/src/ext/*.h)
	@rm -f tor.timestamp.tmp
	@touch tor.timestamp.tmp
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C tor
	@mv -f tor.timestamp.tmp tor.timestamp

$(TOR_LIBS): tor.timestamp
	@if test -f $@; then :; else \
		rm -f tor.timestamp; \
		$(MAKE) $(AM_MAKEFLAGS) tor.timestamp; \
	fi

BITCOIN_CONFIG_INCLUDES=-I$(builddir)/config
BITCOIN_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

BITCOIN_INCLUDES += -I$(srcdir)/secp256k1/include
BITCOIN_INCLUDES += -I$(srcdir)/secp256k1/src
BITCOIN_INCLUDES += $(UNIVALUE_CFLAGS)

LIBBITCOIN_SERVER=libbitcoin_server.a
LIBBITCOIN_COMMON=libbitcoin_common.a
LIBBITCOIN_CONSENSUS=libbitcoin_consensus.a
LIBBITCOIN_CLI=libbitcoin_cli.a
LIBBITCOIN_UTIL=libbitcoin_util.a
LIBBITCOIN_CRYPTO=crypto/libbitcoin_crypto.a
LIBBITCOINQT=qt/libGravityCoinqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

LIBGRAVITYCOIN_SIGMA=libsigma.a

if ENABLE_ZMQ
LIBBITCOIN_ZMQ=libbitcoin_zmq.a
endif
if BUILD_BITCOIN_LIBS
LIBBITCOINCONSENSUS=libbitcoinconsensus.la
endif
if ENABLE_WALLET
LIBBITCOIN_WALLET=libbitcoin_wallet.a
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)



# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_CLI) \
  $(LIBBITCOIN_WALLET) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBGRAVITYCOIN_SIGMA)

lib_LTLIBRARIES = $(LIBBITCOINCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_BITCOIND
  bin_PROGRAMS += GravityCoind
endif

if BUILD_BITCOIN_UTILS
  bin_PROGRAMS += GravityCoin-cli GravityCoin-tx
endif

.PHONY: FORCE check-symbols check-security
# bitcoin core #
BITCOIN_CORE_H = \
  activexnode.h \
  addressindex.h \
  spentindex.h \
  addrman.h \
  base58.h \
  bloom.h \
  blockencodings.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  core_io.h \
  core_memusage.h \
  httprpc.h \
  httpserver.h \
  indirectmap.h \
  darksend.h \
  darksend-relay.h \
  init.h \
  instantx.h \
  key.h \
  keystore.h \
  dbwrapper.h \
  limitedmap.h \
  threadinterrupt.h \
  main.h \
  xnode.h \
  xnode-payments.h \
  xnode-sync.h \
  xnodeman.h \
  xnodeconfig.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  netbase.h \
  netfulfilledman.h \
  noui.h \
  policy/fees.h \
  policy/policy.h \
  policy/rbf.h \
  primitives/zerocoin.h \
  fixed.h \
  pow.h \
  hdmint/hdmint.h \
  protocol.h \
  random.h \
  reverselock.h \
  rpc/client.h \
  rpc/protocol.h \
  rpc/server.h \
  rpc/register.h \
  scheduler.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  script/ismine.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/pagelocker.h \
  sync.h \
  threadsafety.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  undo.h \
  util.h \
  utilmoneystr.h \
  utiltime.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/rpcwallet.h \
  wallet/sigmaspendbuilder.h \
  wallet/txbuilder.h \
  wallet/wallet.h \
  wallet/walletexcept.h \
  wallet/walletdb.h \
  wallet/authhelper.h \
  definition.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h \
  hdmint/mintpool.h \
  hdmint/tracker.h \
  hdmint/wallet.h \
  zerocoin.h \
  sigma.h \
  coin_containers.h \
  zerocoin_params.h \
  addresstype.h

obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libbitcoin_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between GravityCoind and GravityCoin-qt
libbitcoin_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libbitcoin_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_server_a_SOURCES = \
  addrman.cpp \
  bloom.cpp \
  blockencodings.cpp \
  chain.cpp \
  checkpoints.cpp \
  httprpc.cpp \
  httpserver.cpp \
  init.cpp \
  dbwrapper.cpp \
  threadinterrupt.cpp \
  main.cpp \
  merkleblock.cpp \
  miner.cpp \
  net.cpp \
  netfulfilledman.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  primitives/zerocoin.cpp \
  pow.cpp \
  rest.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/rpcxnode.cpp \
  rpc/server.cpp \
  script/sigcache.cpp \
  script/ismine.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  ui_interface.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  zerocoin.cpp \
  sigma.cpp \
  coin_containers.cpp \
  $(BITCOIN_CORE_H)

if ENABLE_ZMQ
libbitcoin_zmq_a_CPPFLAGS = $(BITCOIN_INCLUDES) $(ZMQ_CFLAGS)
libbitcoin_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif


# wallet: shared between GravityCoind and GravityCoin-qt, but only linked
# when wallet enabled
libbitcoin_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_wallet_a_SOURCES = \
  activexnode.cpp \
  darksend.cpp \
  hdmint/hdmint.cpp \
  xnode.cpp \
  instantx.cpp \
  xnode-payments.cpp \
  xnode-sync.cpp \
  xnodeconfig.cpp \
  xnodeman.cpp \
  hdmint/mintpool.cpp \
  hdmint/wallet.cpp \
  sigma.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/sigmaspendbuilder.cpp \
  wallet/txbuilder.cpp \
  wallet/walletexcept.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  wallet/authhelper.cpp \
  hdmint/tracker.cpp \
  policy/rbf.cpp \
  primitives/zerocoin.cpp \
  $(BITCOIN_CORE_H)

# crypto primitives library
crypto_libbitcoin_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_CONFIG_INCLUDES)
crypto_libbitcoin_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbitcoin_crypto_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/common.h \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h

# consensus: shared between all executables that validate any consensus rules.
libbitcoin_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(PIC_FLAGS)
libbitcoin_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) $(PIC_FLAGS)
libbitcoin_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/validation.h \
  hash.cpp \
  flat-database.h \
  hash.h \
  prevector.h \
  crypto/scrypt.h \
  primitives/block.h \
  primitives/precomputed_hash.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/bitcoinconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  spork.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  utilstrencodings.cpp \
  utilstrencodings.h \
  version.h \
  auxpow.h \
  crypto/Lyra2Z/Lyra2.h \
  crypto/Lyra2Z/Lyra2.c \
  crypto/Lyra2Z/Lyra2Z.h \
  crypto/Lyra2Z/Lyra2Z.c \
  crypto/Lyra2Z/blake.c \
  crypto/Lyra2Z/sph_blake.h \
  crypto/Lyra2Z/sph_types.h \
  crypto/Lyra2Z/Sponge.c \
  crypto/Lyra2Z/Sponge.h

# common: shared between GravityCoind, and GravityCoin-qt and non-server tools
libbitcoin_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_common_a_SOURCES = \
  amount.cpp \
  base58.cpp \
  chainparams.cpp \
  coins.cpp \
  auxpow.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  hdmint/hdmint.cpp \
  key.cpp \
  keystore.cpp \
  netbase.cpp \
  protocol.cpp \
  scheduler.cpp \
  script/sign.cpp \
  script/standard.cpp \
  spork.cpp \
  $(BITCOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libbitcoin_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_util_a_SOURCES = \
  support/pagelocker.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  random.cpp \
  rpc/protocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  util.cpp \
  utilmoneystr.cpp \
  utilstrencodings.cpp \
  utiltime.cpp \
  crypto/scrypt.cpp \
  primitives/block.cpp \
  libzerocoin/bitcoin_bignum/allocators.h \
  libzerocoin/bitcoin_bignum/bignum.h \
  libzerocoin/bitcoin_bignum/compat.h \
  libzerocoin/bitcoin_bignum/netbase.h \
  libzerocoin/Accumulator.h \
  libzerocoin/Accumulator.cpp \
  libzerocoin/AccumulatorProofOfKnowledge.h \
  libzerocoin/AccumulatorProofOfKnowledge.cpp \
  libzerocoin/Coin.h \
  libzerocoin/Coin.cpp \
  libzerocoin/CoinSpend.h \
  libzerocoin/CoinSpend.cpp \
  libzerocoin/Commitment.h \
  libzerocoin/Commitment.cpp \
  libzerocoin/ParallelTasks.h \
  libzerocoin/ParallelTasks.cpp \
  libzerocoin/ParamGeneration.h \
  libzerocoin/ParamGeneration.cpp \
  libzerocoin/Params.h \
  libzerocoin/Params.cpp \
  libzerocoin/SerialNumberSignatureOfKnowledge.h \
  libzerocoin/SerialNumberSignatureOfKnowledge.cpp \
  libzerocoin/AccumulatorProofOfKnowledge.cpp \
  libzerocoin/SpendMetaData.h \
  libzerocoin/SpendMetaData.cpp \
  libzerocoin/Zerocoin.h \
  $(BITCOIN_CORE_H)

libsigma_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) -Werror
libsigma_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) -Werror
libsigma_a_SOURCES = \
  sigma/r1_proof.h \
  sigma/r1_proof_generator.h \
  sigma/r1_proof_generator.hpp \
  sigma/r1_proof_verifier.h \
  sigma/r1_proof_verifier.hpp \
  sigma/sigmaplus_proof.h \
  sigma/sigmaplus_prover.h \
  sigma/sigmaplus_prover.hpp \
  sigma/sigmaplus_verifier.h \
  sigma/sigmaplus_verifier.hpp \
  sigma/sigma_primitives.h \
  sigma/sigma_primitives.hpp \
  sigma/coin.h \
  sigma/coin.cpp \
  sigma/coinspend.cpp \
  sigma/coinspend.h \
  sigma/spend_metadata.cpp \
  sigma/spend_metadata.h \
  sigma/remint.h \
  sigma/remint.cpp \
  sigma/remint-blacklist.cpp \
  sigma/params.h \
  sigma/params.cpp \
  sigma/openssl_context.h

if GLIBC_BACK_COMPAT
libbitcoin_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between GravityCoin-cli and GravityCoin-qt
libbitcoin_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libbitcoin_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbitcoin_cli_a_SOURCES = \
  rpc/client.cpp \
  $(BITCOIN_CORE_H)

nodist_libbitcoin_util_a_SOURCES = $(srcdir)/obj/build.h
#

# bitcoind binary #
GravityCoind_SOURCES = bitcoind.cpp
GravityCoind_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
GravityCoind_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
GravityCoind_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
GravityCoind_SOURCES += bitcoind-res.rc
endif

GravityCoind_LDADD = \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_WALLET) \
  $(LIBGRAVITYCOIN_SIGMA) \
  $(LIBBITCOIN_ZMQ) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)



GravityCoind_LDADD += $(TOR_LIBS) $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) -lz

# bitcoin-cli binary #
GravityCoin_cli_SOURCES = bitcoin-cli.cpp
GravityCoin_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS)
GravityCoin_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
GravityCoin_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
GravityCoin_cli_SOURCES += bitcoin-cli-res.rc
endif

GravityCoin_cli_LDADD = \
  $(LIBBITCOIN_CLI) \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CRYPTO)

GravityCoin_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS)
#

# bitcoin-tx binary #
GravityCoin_tx_SOURCES = bitcoin-tx.cpp
GravityCoin_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
GravityCoin_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
GravityCoin_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
GravityCoin_tx_SOURCES += bitcoin-tx-res.rc
endif

GravityCoin_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CONSENSUS) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBSECP256K1)

GravityCoin_tx_LDADD += $(BOOST_LIBS) $(CRYPTO_LIBS)
#

# bitcoinconsensus library #
if BUILD_BITCOIN_LIBS
include_HEADERS = script/bitcoinconsensus.h
libbitcoinconsensus_la_SOURCES = $(crypto_libbitcoin_crypto_a_SOURCES) $(libbitcoin_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libbitcoinconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libbitcoinconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libbitcoinconsensus_la_LIBADD = $(LIBSECP256K1)
libbitcoinconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_BITCOIN_INTERNAL
libbitcoinconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno
CLEANFILES += tor.timestamp tor.timestamp.tmp

DISTCLEANFILES = obj/build.h

EXTRA_DIST = $(CTAES_DIST)

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-$(MAKE) -C tor clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(AM_CXXFLAGS) $(PIE_FLAGS) $(CXXFLAGS) -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

# Exodus
include Makefile.exodus.include
#

// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64_t nTimeLastCheckpoint;
        int64_t nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    bool fEnabled = true;

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (      0, uint256("0x000006cab7aa2be2da91015902aa4458dd5fbb8778d175c36d429dc986f2bff4"))
        (  30350, uint256("0x000000000032d087f157871fbc41541a43ac30291f99ce5225d69fd132f8ecdf"))
        (  66438, uint256("0x00000000000970ae1d1fddcdf363dfc49505caa2884367ad460839d0621d1f56"))
        ( 103010, uint256("0x000000000007204260b891b9aa8eb476132e74eb7539dc3e9ac2fb7bc7104ab8"))
        ( 252509, uint256("0x0000000000090c2b77a3247303784289fb6a18752d54e38e96d2b48eac245016"))
        ( 470201, uint256("0x00000000000102bdfdfc228ee34304f64650825fd1639a1f57a397af854b9df1"))
        ( 523001, uint256("0x00000000000076412e07ded5bcdf11c0ea6bfcada9e339cb31d312d8e60c3ef8"))
        ( 569410, uint256("0x000000000000085bbed51c9196314ee52281428ff5b1d8cade9140efe4b33381"))
        ( 587317, uint256("0x000000000000b81d6626e9fd0c869764dd992d5429442876a75894e24c0c15e2"))
        ( 636469, uint256("0x0000000000038394fe569fbd5a42484c69f15dae9f10982a7a7ed96bff4a359e"))
        ( 720261, uint256("0x000000000005658b461195d927cf3347ebf8a36e987a2d2be26ed4fce0f75b13"))
        ( 815426, uint256("0x000000000004958ee412205bc78e41061e3cb66b55cdd5230efceaaa07990f55"))
        ( 870101, uint256("0x000000000006fcd0f5cff20c46d9da02f7835137bce0629431f1968c6d1dcab5"))
        ( 978901, uint256("0x00000000000456f795ce33e9ad1757150c1b5155230e4438b3690004e00f7ede"))
        (1272500, uint256("0x000000000001567d68a0197b43ec9c764d49a78cee9c318d58c5ae8d3a6a4a88"))
        (1410098, uint256("0x000000000000ad1fe5f741c497aab1f4c9f2799ed2cce1c6715601e84c543368"))
        (1538097, uint256("0x00000000000002f54303f5b45c1ec74c75f085034fe0438834bb6ed2cb2f78f1"))
        (1967101, uint256("0x0000000000006f92c571a6b1a6923efd03320b6bb6bc0656c4f23d01e8664a85"))
        (2062289, uint256("0x000000000001b7e95495d1f418f69498804397745f29e024d40dbe1ef4725af8"))
        (2229225, uint256("0x000000000003c9990b62822e5be8a49bebae5e270c39db223d3504d2ecd38604"))
        (2647621, uint256("0x0000000000019c07fe91065d5dee6b42af812830e04ce59efa7fad10cb020396"))
        (4086591, uint256("0x00000000000284dcc409a09957de00a54bc63bcc3348305375f5df8b150fc4c4"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1449673550, // * UNIX timestamp of last checkpoint block
        4966027,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        2880        // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256("0x000007717e2e2df52a9ff29b0771901c9c12f5cbb4914cdf0c8047b459bb21d8"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1374901773,
        0,
        2880
    };

    static MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"))
        ;
    static const CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

    const CCheckpointData &Checkpoints() {
        if (Params().NetworkID() == CChainParams::TESTNET)
            return dataTestnet;
        else if (Params().NetworkID() == CChainParams::MAIN)
            return data;
        else
            return dataRegtest;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!fEnabled)
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks) {
        if (pindex==NULL)
            return 0.0;

        int64_t nNow = time(NULL);

        double fSigcheckVerificationFactor = fSigchecks ? SIGCHECK_VERIFICATION_FACTOR : 1.0;
        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkpoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!fEnabled)
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!fEnabled)
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}

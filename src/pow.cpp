// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

/*unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    //if (pindexLast->nHeight >= 96999) {
        // For example, set the difficulty to a lower value for blocks after the fork
        //return nProofOfWorkLimit;  // Set to reduced difficulty (adjust to your desired value)
    //}

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}*/

/*unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("params.nPowTargetTimespan = %d    nActualTimespan = %d\n", params.nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}*/

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // If block height is greater than the threshold, apply the new difficulty parameters
    //int nPowTargetTimespan = (pindexLast->nHeight >= params.nSwitchHeight) ? params.nNewPowTargetTimespan : params.nOldPowTargetTimespan;
        // Determine the correct target timespan based on block height
    int nPowTargetTimespan;
    if (pindexLast->nHeight < 97000) {
        nPowTargetTimespan = 14 * 24 * 60 * 60; // 10 minutes for blocks 0 to 96999
    } else if (pindexLast->nHeight <= 97191) {
        nPowTargetTimespan = 60; // 1 minute for blocks 97000 to 97191
    } else {
        nPowTargetTimespan = 6 * 60 * 60; // 10 minutes for blocks 97192 and onward
    }

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params/* nPowTargetTimespan*/);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    int nPowTargetTimespan;

    // Disable difficulty retargeting for blocks before 97000
    if (pindexLast->nHeight < 97000) {
        //LogPrintf("Syncing blocks before block 97000, ignoring difficulty retargeting.\n");
        return pindexLast->nBits;  // Keep the same difficulty for blocks before 97000 112239
    }

    // Special rule for block height 112,239
    if (pindexLast->nHeight == 112239) {
        // Return difficulty divided by 3 for block 112,239
        return pindexLast->nBits / 3;
    }

    // Set target timespan based on block height (after block 97000)
    if (pindexLast->nHeight < 97000) {
        nPowTargetTimespan = 14 * 24 * 60 * 60;  // 10 minutes for blocks 0 to 96999
    } else if (pindexLast->nHeight <= 97191) {
        nPowTargetTimespan = 60;   // 1 minute for blocks 97000 to 97191
    } else {
        nPowTargetTimespan = 6 * 60 * 60;  // 10 minutes for blocks 97192 and onward
    }

    // If retargeting is disabled globally, return the previous block's difficulty
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Calculate the actual timespan for the current block
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;

    // Ensure nActualTimespan is not negative
    if (nActualTimespan < 0)
        nActualTimespan = 0;

    // Apply boundaries to the timespan to avoid drastic difficulty changes
    if (nActualTimespan < nPowTargetTimespan / 4) {
        LogPrintf("  nActualTimespan was too small. Setting it to %d (1/4 of target)\n", nPowTargetTimespan / 4);
        nActualTimespan = nPowTargetTimespan / 4;  // Minimum 1/4th of target timespan
    }
    if (nActualTimespan > nPowTargetTimespan * 4) {
        LogPrintf("  nActualTimespan was too large. Setting it to %d (4x of target)\n", nPowTargetTimespan * 4);
        nActualTimespan = nPowTargetTimespan * 4;  // Maximum 4x of target timespan
    }

    // Perform difficulty adjustment more conservatively
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);  // Max difficulty
    arith_uint256 bnNew;
    arith_uint256 bnOld;

    bnNew.SetCompact(pindexLast->nBits);  // Get the current difficulty
    bnOld = bnNew;

    // Adjust difficulty based on the timespan with a more gradual adjustment
    bnNew *= nActualTimespan;
    bnNew /= nPowTargetTimespan;

    // Limit the adjustment to prevent drastic changes
    const arith_uint256 bnMaxStepIncrease = bnOld * 2;  // Limit increase to 400%
    const arith_uint256 bnMaxStepDecrease = bnOld * 1;  // Limit decrease to 200%

    // Ensure difficulty does not deviate too much from the previous block
    if (bnNew > bnMaxStepIncrease) {
        LogPrintf("  Difficulty increased too much. Setting it to 10%% increase max.\n");
        bnNew = bnMaxStepIncrease;
    }
    if (bnNew < bnMaxStepDecrease) {
        LogPrintf("  Difficulty decreased too much. Setting it to 10%% decrease max.\n");
        bnNew = bnMaxStepDecrease;
    }

    // Enforce the maximum difficulty limit
    if (bnNew > bnPowLimit) {
        LogPrintf("  Difficulty exceeded the limit. Setting it to the maximum difficulty.\n");
        bnNew = bnPowLimit;
    }

    // Debugging logs to better understand the difficulty and timespan calculations
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("nPowTargetTimespan = %d    nActualTimespan = %d\n", nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    // Check the new difficulty after the adjustment
    LogPrintf("Adjusted Difficulty: %s\n", bnNew.ToString());

    return bnNew.GetCompact();
}



/*unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params/*, int nPowTargetTimespan*//*)
{
    //int nPowTargetTimespan = (pindexLast->nHeight >= params.nSwitchHeight) ? params.nNewPowTargetTimespan : params.nOldPowTargetTimespan;
        // Determine the correct target timespan based on block height
    int nPowTargetTimespan;
    if (pindexLast->nHeight < 97000) {
        nPowTargetTimespan = 600; // 10 minutes for blocks 0 to 96999
    } else if (pindexLast->nHeight <= 97191) {
        nPowTargetTimespan = 60; // 1 minute for blocks 97000 to 97191
    } else {
        nPowTargetTimespan = 600; // 10 minutes for blocks 97192 and onward
    }

    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);

    if (nActualTimespan < nPowTargetTimespan / 4)
        nActualTimespan = nPowTargetTimespan / 4;  // No faster than 1/4th of the target timespan
    if (nActualTimespan > nPowTargetTimespan * 4)
        nActualTimespan = nPowTargetTimespan * 4; 

    //if (nActualTimespan < nPowTargetTimespan/4)
        //nActualTimespan = nPowTargetTimespan/4;
    //if (nActualTimespan > nPowTargetTimespan*4)
        //nActualTimespan = nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nActualTimespan;
    bnNew /= nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("params.nPowTargetTimespan = %d    nActualTimespan = %d\n", nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}*/

/*unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("params.nPowTargetTimespan = %d    nActualTimespan = %d\n", params.nPowTargetTimespan, nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}*/


bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("CheckProofOfWork(): hash doesn't match nBits");

    return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}

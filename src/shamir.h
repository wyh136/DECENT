#ifndef SHAMIR_H
#define SHAMIR_H

#include "bignum.h"
#include "uint256.h"

class  Shamir {
public:
    Shamir(unsigned char shares, unsigned char quorum, CBigNum order = CBigNum(0));

    typedef std::pair<unsigned char, uint256> Share;
    typedef std::map<unsigned char, uint256> Shares;

    Shares split(uint256 secret) const;

    uint256 recover(const Shares& shares) const;

    unsigned char quorum() const;

private:
    CBigNum rnd() const;

private:
    unsigned char _shares;
    unsigned char _quorum;
    CBigNum _order;
};

#endif // SHAMIR_H

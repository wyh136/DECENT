// Copyright (c) 2015-2015 Decent developers

#include <openssl/rand.h>
#include "shamir.h"


Shamir::Shamir(unsigned char shares, unsigned char quorum, CBigNum order) : _shares(shares), _quorum(quorum), _order(order) {
    if (_order == CBigNum(0)) {
        _order.SetHex("fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141");
    }
}

Shamir::Shares Shamir::split(uint256 secret) const {
    Shares shares;
    std::vector<CBigNum> coef;
    coef.push_back(CBigNum(secret));
    for (unsigned char i = 1; i < _quorum; ++i)
        coef.push_back(rnd());

    for (unsigned char x = 1; x <= _shares; ++x) {
        CBigNum accum = coef[0];
        for (unsigned char i = 1; i < _quorum; ++i)
            accum = (accum + (coef[i] * ModPow(CBigNum(x), i, _order)) % _order) % _order;
        shares[x] = accum.getuint256();
    }

    return shares;
}

uint256 Shamir::recover(const Shamir::Shares& shares) const
{
    CBigNum accum = 0;
    for(Shares::const_iterator formula = shares.begin(); formula != shares.end(); ++formula)
    {
        CBigNum numerator = 1;
        CBigNum denominator = 1;
        for(Shares::const_iterator count = shares.begin(); count != shares.end(); ++count)
        {
            if(formula == count) continue; // If not the same value
            unsigned char startposition = formula->first;
            unsigned char nextposition = count->first;
            numerator = (-nextposition * numerator) % _order;
            denominator = ((startposition - nextposition)*denominator) % _order;
        }
        accum = (_order + accum + (CBigNum(formula->second) * numerator * ModInverse(denominator, _order))) % _order;
    }
    return accum.getuint256();
}

unsigned char Shamir::quorum() const
{
    return _quorum;
}

CBigNum Shamir::rnd() const
{
    uint256 r;
    RAND_bytes((unsigned char*)&r, 32);
    return CBigNum(r) % _order;
}

#ifndef SHAREDSECRET_H
#define SHAREDSECRET_H

#include <vector>
#include <uint256.h>
#include <cmath>

/** Realization of Shamir's secret sharing algotithm.
    [WIP] Now it doesn't work at all */
class SharedSecret
{
public:
    SharedSecret();
    static int splitSecret(uint256 secret, std::vector<uint256> &outputKeys, const unsigned int amountOfOutputKeys, const unsigned int amountOfNecesseryKeys);
    static uint256 prime;
private:
};

#endif // SHAREDSECRET_H

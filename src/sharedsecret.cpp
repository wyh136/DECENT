#include "sharedsecret.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

SharedSecret::SharedSecret()
{
}

int SharedSecret::splitSecret(uint256 secret, std::vector<uint256> &outputKeys, const unsigned int amountOfOutputKeys, const unsigned int amountOfNecesseryKeys)
{
    std::vector<uint256> coefficients;
    for (int i = 0; i < amountOfNecesseryKeys; i++)
    {
        coefficients.push_back(0);
        coefficients[i].SetRandom();
    }
    outputKeys.clear();
    for (int i = 0; i < amountOfOutputKeys; i++)
    {
        outputKeys.push_back(uint256(i));
    }
    return 0;
}



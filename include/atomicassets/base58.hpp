// Copyright (c) 2014-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//(Slightly modified for the needs of our eosio contract)

#include <string>
#include <vector>

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);

std::string EncodeBase58(const std::vector<unsigned char>& vch);

//Removed the max return length.
bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch);

bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet);
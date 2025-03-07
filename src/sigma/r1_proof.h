#ifndef GRAVITYCOIN_SIGMA_R1_PROOF_H
#define GRAVITYCOIN_SIGMA_R1_PROOF_H

#include <vector>
#include <secp256k1/include/Scalar.h>
#include <secp256k1/include/GroupElement.h>
#include "../../serialize.h"

namespace sigma {

template <class Exponent, class GroupElement>
class R1Proof {

public:
    R1Proof() = default;

    inline int memoryRequired(int n, int m) const {
        return A_.memoryRequired() * 3 + ZA_.memoryRequired() * (m*(n - 1) + 2);
    }

    inline unsigned char* serialize(unsigned char* buffer) const {
        unsigned char* current = A_.serialize(buffer);
        current = C_.serialize(current);
        current = D_.serialize(current);
        for (std::size_t i = 0; i < f_.size(); ++i)
            current = f_[i].serialize(current);
        current = ZA_.serialize(current);
        return ZC_.serialize(current);
    }
    inline unsigned char* deserialize(unsigned char* buffer, int n, int m) {
        unsigned char* current = A_.deserialize(buffer);
        current = C_.deserialize(current);
        current = D_.deserialize(current);
        int f_size =  m * (n - 1);
        f_.resize(f_size);
        for(int i = 0; i < f_size; ++i)
            current = f_[i].deserialize(current);
        current = ZA_.deserialize(current);
        return ZC_.deserialize(current);
    }

    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(A_);
        READWRITE(C_);
        READWRITE(D_);
        READWRITE(f_);
        READWRITE(ZA_);
        READWRITE(ZC_);
    }

    GroupElement A_;
    GroupElement C_;
    GroupElement D_;
    std::vector<Exponent> f_;
    Exponent ZA_;
    Exponent ZC_;
};

} // namespace sigma

#endif // GRAVITYCOIN_SIGMA_R1_PROOF_H

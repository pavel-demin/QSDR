#include <QByteArray>
#include "type.h"
extern "C" {
#include "sha256.h"
}


int HashSHA256 (uint8 *value, int len, uint8* hashcode) 
{
    sha256_state sha256State;

    sha256_init(&sha256State);
    sha256_process(&sha256State, value, len);
    sha256_done(&sha256State, hashcode);

    return 0;
}

QByteArray HashSHA256(QByteArray data) {
	uint8 hash[SHA256_DIGEST_SIZE];
	HashSHA256((uint8*)data.data(), data.size(), hash);
	return QByteArray((const char*)hash, sizeof(hash));
}


#if 0
int VerifyHash (
	ASN1 tbs,
	ASN1_BITSTRING signature,
	ASN1_AlgorithmIdentifier algorithm)
{
    int returnValue = ERR_UNDEF;
    int len,i;
    char hash160[RMDSize/8];
    char csign[MAXRSALEN];

    ARG_UNUSED(algorithm);

    if (GetASN1DataLen (signature, &len))
	assert(0);

    if (GetASN1Bitstring (signature, csign, len))
	assert(0);

    HashRIPEMD160 (tbs, hash160);

    returnValue = ERR_OK;
    for (i=0; (uint32)i<sizeof(hash160); i++ )
	if (hash160[i] != csign[i+1] ) 
	    returnValue = ERR_CHK;

    return returnValue;
	return 0;
}

int SetHash ( ASN1 tbs, ASN1_BITSTRING signature, ASN1_AlgorithmIdentifier algorithm) {
    int returnValue = ERR_UNDEF;
    int len;
    uint8 *hash;

    DeclareASN1Var(oid, ASN1_OBJECTIDENTIFIER);
    DeclareASN1Var(tmpoct, ASN1_OCTETSTRING);

    InitASN1Var(oid, ASN1_OBJECTIDENTIFIER);
    InitASN1Var(tmpoct, ASN1_OCTETSTRING);
    
    ARG_UNUSED(algorithm);

    if (GetASN1DataLen (signature, &len)!=ASN_OK) goto EXIT;
    assert(len==RMDSize/8+1);
    if(len!=RMDSize/8+1) assert(0);

    GetASN1ValuePtr (signature, &hash);

    hash[0]=0;
    HashRIPEMD160 (tbs, (char*)(hash+1));
    returnValue = ERR_OK;

EXIT:
    FreeASN1(oid);
    FreeASN1(tmpoct);

    return returnValue;
}
#endif


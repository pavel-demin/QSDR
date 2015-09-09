
#define SHA256_DIGEST_SIZE	32	/* in Bytes */

int HashSHA256 (uint8 from, int len, uint8* hashcode);
QByteArray HashSHA256(QByteArray data);


//int VerifyHash ( ASN1 tbs, ASN1_BITSTRING signature, ASN1_AlgorithmIdentifier algorithm);
//int SetHash ( ASN1 tbs, ASN1_BITSTRING signature, ASN1_AlgorithmIdentifier algorithm);

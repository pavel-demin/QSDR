/*
 * $Id: sha256.h 9 2011-03-14 16:05:01Z stefan $
 *
 * SHA-256 aus libtomcrypt
 */
#ifndef SHA256_H
#define SHA256_H

#define SHA256_DIGEST_SIZE	32	/* in Bytes */

typedef struct {
    uint64 length;
    uint32 state[8], curlen;
    uint8 buf[64];
} sha256_state;

void sha256_init(sha256_state* md);
void sha256_process(sha256_state* md, const uint8 *in, unsigned long inlen);
void sha256_done(sha256_state* md, uint8 *out);
void sha256_test(void);

#endif /* SHA256_H */

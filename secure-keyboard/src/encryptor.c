#include "encryptor.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define KEY_LENGTH 32
#define IV_LENGTH 12
#define TAG_LENGTH 16

static const unsigned char key[KEY_LENGTH] = {
    0x12, 0x34, 0x56, 0x78, 0xab, 0xcd, 0xef, 0x01,
    0x23, 0x45, 0x67, 0x89, 0xfe, 0xdc, 0xba, 0x98,
    0x76, 0x54, 0x32, 0x10, 0x11, 0x22, 0x33, 0x44,
    0x55, 0x66, 0x77, 0x88, 0xaa, 0xbb, 0xcc, 0xdd
};

struct __attribute__((packed)) EventData {
    struct timeval time;
    uint16_t code;
    uint32_t value;
};

int encrypt_event(struct input_event *ev) {
    EVP_CIPHER_CTX *ctx;
    unsigned char iv[IV_LENGTH];
    unsigned char tag[TAG_LENGTH];
    struct EventData input;

    memcpy(&input.time, &ev->time, sizeof(struct timeval));
    input.code = ev->code;
    input.value = ev->value;

    size_t plaintext_len = sizeof(struct EventData);
    unsigned char ciphertext[128];  // 충분히 큰 버퍼

    if (!RAND_bytes(iv, sizeof(iv))) {
        fprintf(stderr, "IV 생성 실패\n");
        return -1;
    }

    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LENGTH, NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);

    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char *)&input, plaintext_len);

    int ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LENGTH, tag);
    EVP_CIPHER_CTX_free(ctx);

    FILE *fp = fopen("log_encrypted.dat", "ab");
    if (!fp) return -1;

    fwrite(iv, 1, IV_LENGTH, fp);
    fwrite(ciphertext, 1, ciphertext_len, fp);
    fwrite(tag, 1, TAG_LENGTH, fp);
    fclose(fp);

    return 0;
}

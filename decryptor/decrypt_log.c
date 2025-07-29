// gcc decrypt_log.c -o decrypt_log -lssl -lcrypto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <sys/time.h>
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

int decrypt_record(FILE *fp) {
    unsigned char iv[IV_LENGTH];
    unsigned char tag[TAG_LENGTH];
    unsigned char ciphertext[sizeof(struct EventData)];
    unsigned char plaintext[sizeof(struct EventData)];
    int len, plaintext_len;

    if (fread(iv, 1, IV_LENGTH, fp) != IV_LENGTH) return 0; // EOF

    if (fread(ciphertext, 1, sizeof(ciphertext), fp) != sizeof(ciphertext)) {
        fprintf(stderr, "암호문 읽기 실패\n");
        return -1;
    }

    if (fread(tag, 1, TAG_LENGTH, fp) != TAG_LENGTH) {
        fprintf(stderr, "태그 읽기 실패\n");
        return -1;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LENGTH, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);

    if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, sizeof(ciphertext))) {
        fprintf(stderr, "복호화 실패\n");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LENGTH, tag);

    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    EVP_CIPHER_CTX_free(ctx);
    if (ret <= 0) {
        fprintf(stderr, "태그 검증 실패: 데이터 위변조 가능성 있음\n");
        return -1;
    }
    plaintext_len += len;

    struct EventData *ev = (struct EventData *)plaintext;
    printf("시간: %ld.%06ld, 키코드: %u, 상태: %u\n",
           ev->time.tv_sec, ev->time.tv_usec, ev->code, ev->value);

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("사용법: %s <암호화된 로그 파일>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("파일 열기 실패");
        return 1;
    }

    while (1) {
        int res = decrypt_record(fp);
        if (res == 0) break;   // EOF
        if (res < 0) return 1; // 오류
    }

    fclose(fp);
    return 0;
}

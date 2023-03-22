#include <openssl/mem.h>
#include <openssl/ssl.h>
#include <openssl/cipher.h>

extern "C" {
#include <openssl/rsa.h>
}

extern "C" int EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out, int *out_len);

extern "C" int EVP_DecryptFinal(EVP_CIPHER_CTX *ctx, unsigned char *out, int *out_len) {
    return EVP_DecryptFinal_ex(ctx, out, out_len);
}

extern "C" int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, uint8_t *out, int *out_len);

extern "C" int EVP_EncryptFinal(EVP_CIPHER_CTX *ctx, uint8_t *out, int *out_len) {
    int result = EVP_EncryptFinal_ex(ctx, out, out_len);
    EVP_CIPHER_CTX_cleanup(ctx);
    return result;
}

extern "C" void CONF_modules_unload(int p) {}

extern "C" {

#undef CRYPTO_malloc
#undef CRYPTO_realloc
#undef CRYPTO_free

#define OPENSSL_CTRL_SET_MSG_CALLBACK_ARG   16
#define OPENSSL_CTRL_OPTIONS                32
#define OPENSSL_CTRL_MODE                   33
#define OPENSSL_CTRL_GET_READ_AHEAD         40
#define OPENSSL_CTRL_SET_READ_AHEAD         41
#define OPENSSL_CTRL_GET_MAX_CERT_LIST      50
#define OPENSSL_CTRL_SET_MAX_CERT_LIST      51
#define OPENSSL_CTRL_SET_MAX_SEND_FRAGMENT  52
#define OPENSSL_CTRL_GET_RI_SUPPORT         76
#define OPENSSL_CTRL_CLEAR_OPTIONS          77
#define OPENSSL_CTRL_CLEAR_MODE             78

long SSL_ctrl(SSL *s, int cmd, long larg, void *parg) {
    long l;

    switch (cmd) {
	    case OPENSSL_CTRL_GET_READ_AHEAD:
		    return SSL_get_read_ahead(s);

	    case OPENSSL_CTRL_SET_READ_AHEAD:
		    l = SSL_get_read_ahead(s);
		    SSL_set_read_ahead(s, (int)larg);
		    return l;

	    case OPENSSL_CTRL_SET_MSG_CALLBACK_ARG:
		    SSL_set_msg_callback_arg(s, parg);
		    return 1;

	    case OPENSSL_CTRL_OPTIONS:
		    return SSL_set_options(s, larg);

	    case OPENSSL_CTRL_CLEAR_OPTIONS:
		    return SSL_clear_options(s, larg);

	    case OPENSSL_CTRL_MODE:
		    return SSL_set_mode(s, larg);

	    case OPENSSL_CTRL_CLEAR_MODE:
		    return SSL_clear_mode(s, larg);

	    case OPENSSL_CTRL_GET_MAX_CERT_LIST:
		    return SSL_get_max_cert_list(s);

	    case OPENSSL_CTRL_SET_MAX_CERT_LIST:
		    l = SSL_get_max_cert_list(s);
		    SSL_set_max_cert_list(s, larg);
		    return l;

	    case OPENSSL_CTRL_SET_MAX_SEND_FRAGMENT:
		    if (larg < 512 || larg > SSL3_RT_MAX_PLAIN_LENGTH)
			    return 0;
		    SSL_set_max_send_fragment(s, larg);
		    return 1;

	    case OPENSSL_CTRL_GET_RI_SUPPORT:
		    //if (s->s3)
			    return SSL_get_secure_renegotiation_support(s);
		    //else return 0;

	    default:
		    // return(s->method->ssl_ctrl(s,cmd,larg,parg));
		    return 0;
	}
}

void *CRYPTO_malloc(size_t size) {
	return OPENSSL_malloc(size);
}

void *CRYPTO_realloc(void *ptr, size_t new_size) {
	return OPENSSL_realloc(ptr, new_size);
}

void CRYPTO_free(void *ptr) {
	return OPENSSL_free(ptr);
}

void OBJ_cleanup(void) {}

}

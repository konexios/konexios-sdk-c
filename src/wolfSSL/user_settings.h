
#if defined(_ARIS_)
//    #define WOLFSSL_CMSIS_RTOS
//#define HAVE_AESGCM
#define NO_WOLFSSL_MEMORY
#define WOLFSSL_BASE64_ENCODE

//#define WOLFSSL_SHA384
//#define WOLFSSL_SHA512
//#define HAVE_CURVE25519
//#define HAVE_ED25519   /* with HAVE_SHA512 */
//#define HAVE_POLY1305
//#define HAVE_CHACHA
//#define HAVE_ONE_TIME_AUTH
#define WOLFSSL_MALLOC_CHECK

//#define IGNORE_KEY_EXTENSIONS
#define NO_WOLFSSL_DIR
#define DEBUG_WOLFSSL

#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
#define HAVE_TLS_EXTENSIONS

/* Options for Sample program */
//#define WOLFSSL_NO_VERIFYSERVER
//    #define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
#   define TIME_OVERRIDES
#   define XTIME time
#   define XGMTIME localtime
#endif

#elif defined(__MBED__)

    #define MBED
//    #define WOLFSSL_CMSIS_RTOS
    #define WOLFSSL_USER_IO
    #define NO_WRITEV
    #define NO_DEV_RANDOM
    #define HAVE_ECC
    #define HAVE_AESGCM
    
    #define WOLFSSL_SHA384
    #define WOLFSSL_SHA512
    #define HAVE_CURVE25519
    #define HAVE_ED25519   /* with HAVE_SHA512 */
    //#define HAVE_POLY1305
    //#define HAVE_CHACHA
    //#define HAVE_ONE_TIME_AUTH
    
    #define NO_SESSION_CACHE // For Small RAM
    //#define IGNORE_KEY_EXTENSIONS
    #define NO_WOLFSSL_DIR  
//    #define DEBUG_WOLFSSL

    #define WOLFSSL_STATIC_RSA
    #define HAVE_SUPPORTED_CURVES
    #define HAVE_TLS_EXTENSIONS
    
    #define SIZEOF_LONG_LONG  8
    /* Options for Sample program */
    //#define WOLFSSL_NO_VERIFYSERVER
    //#define NO_FILESYSTEM
//    #define HAVE_TM_TYPE
    #ifndef WOLFSSL_NO_VERIFYSERVER
        #define TIME_OVERRIDES
        #define XTIME time
        #define XGMTIME localtime
    #endif
#elif defined(__linux__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
//#define HAVE_ECC
//#define HAVE_AESGCM
#define WOLF_LINUX_OS

//#define WOLFSSL_SHA384
//#define WOLFSSL_SHA512
//#define HAVE_CURVE25519
//#define HAVE_ED25519   /* with HAVE_SHA512 */

//#define NO_SESSION_CACHE // For Small RAM
#define NO_WOLFSSL_DIR
//  #define DEBUG_WOLFSSL

#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
//#define HAVE_TLS_EXTENSIONS

//#define SIZEOF_LONG_LONG  8
/* Options for Sample program */
//#define USE_CYASSL_MEMORY
#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
//#define NO_FILESYSTEM
    #define HAVE_TM_TYPE
#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime_r
#endif

#elif defined(__XCC__)
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_DEV_RANDOM
#define NO_WOLFSSL_DIR
#define NO_SHA512
#define NO_DH
#define NO_DSA
#define NO_HC128
#define SINGLE_THREADED
#define NO_STDIO_FILESYSTEM
#define CTYPE_USER
#define XMALLOC_USER
#define SHA256_DIGEST_SIZE 32
#define WOLFSSL_BASE64_ENCODE
//  #define DEBUG_WOLFSSL

#define WOLFSSL_STATIC_RSA
//#define HAVE_SUPPORTED_CURVES
//#define HAVE_TLS_EXTENSIONS

//#define SIZEOF_LONG_LONG  8
/* Options for Sample program */
//#define USE_CYASSL_MEMORY
//#define NO_WOLFSSL_MEMORY
#define WOLFSSL_NO_VERIFYSERVER
#define NO_FILESYSTEM
#define NO_CERT
#define HAVE_TM_TYPE

#ifndef WOLFSSL_NO_VERIFYSERVER
    #define TIME_OVERRIDES
    #define XTIME time
    #define XGMTIME localtime
#endif

#endif

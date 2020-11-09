# PKI-supported TA verification mechanism

## Overview

OP-TEE version: 3.10.0

### Known Issues

According to the [official documentation of OP-TEE](https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html#trusted-application-private-public-keypair), the key handling in OP-TEE is currently a bit limited since OP-TEE  only supports the same single key for all TA loading verifications. And this single key-based TA verification may cause private key leakage problem. Thus we proposed a public key infrastructure-based TA verification. 

### Main approach

In PKI-support OP-TEE, when the third-party application developers want to develop a TA on the device, they need to follow the steps below. 
First, generate their own key pair (public key and private key) and then convert public key and identifying information into a Certificate Signing Request (CSR) file. Second, submit the CSR file to the device manufacturer’s certificate center to obtain the certificate. Lastly, embed the obtained certificate in the TA file. 
When the TA is loaded into OP-TEE, with the built-in intermediate certificate, OP-TEE verifies whether the certificate embedded in the TA file is trusted (i.e., issued by the same root CA). Besides, OP-TEE uses the third-party developer’s public key stored in the trusted certificate to further verify the digital signature in the TA.


## Modify the crypto library of the OP-TEE 3.10


Modify ``mk/config.mk``
- Set "mbedtls" to CRYPTOLIB.
```shell=
CFG_CRYPTOLIB_NAME ?= mbedtls
CFG_CRYPTOLIB_DIR ?= lib/libmbedtls
```

## Enable OP-TEE kernel to use mbedtls (new crypto library)
Modify `optee_os/lib/libmbedtls/include/mbedtls_config_kernel.h` 

- Add the statements below 
```c=
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_X509_CRT_WRITE_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CHECK_KEY_USAGE
#define MBEDTLS_X509_USE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_CERTS_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PEM_WRITE_C
```
Modify `optee_os/lib/libmbedtls/mbedtls/library/x509_crt.c `
- Add the statement `MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA1 )` 


```c=
const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_default =
{
#if defined(MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_CERTIFICATES)
    /* Allow SHA-1 (weak, but still safe in controlled environments) */
    //MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA1 ) |
#endif
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA1 ) |
    /* Only SHA-2 hashes */
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA224 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    0xFFFFFFF, /* Any PK alg    */
    0xFFFFFFF, /* Any curve     */
    2048,
};
```
Modify `optee_os/lib/libmbedtls/sub.mk`
- Add the two instructions (to make mbedtls support X509 and TLS).
```c=
srcs-y += $(addprefix mbedtls/library/, $(SRCS_X509))
srcs-y += $(addprefix mbedtls/library/, $(SRCS_TLS))
```
## Modify OP-TEE TA file format

\
![](https://i.imgur.com/RmvoBT5.png)

\
Add the certificate field into the TA file (see blue fields in the OP-TEE TA file format above).
For this purpose, we modify `optee_os/scripts/sign_encrypt.py`.

- Add the following code to read certificate
```python=
f = open("/home/optees/new_optee/optee_os/keys/my.crt",'rb')
certificate = f.read()
f.close() 
cert_len = len(certificate)
```

- Add cert_len to shdr
```python=
shdr = struct.pack('<IIIIHHII',
                       magic, img_type, img_size, algo, digest_len, sig_len,cert_len)
```
- Add certificate to digest
```python=
h.update(certificate)
```
- Write certificate to the file
```python=
f.write(certificate)
```

## Parse TA file to access certificate contained therein 
Modify `optee_os/core/include/signed_hdr.h` 

- Add ```uint32_t cert_size``` 
```cpp=
struct shdr {
	uint32_t magic;
	uint32_t img_type;
	uint32_t img_size;
	uint32_t algo;
	uint16_t hash_size;
	uint16_t sig_size;
	uint32_t cert_size;
};
```

- Add SHDR_GET_CERT for certificate access
```cpp=
#define SHDR_GET_CERT(x)	(SHDR_GET_SIG(x) + (x)->sig_size)
```



## Embed the [intermediate certificate](https://en.wikipedia.org/wiki/Public_key_certificate#Intermediate_certificate) into the OP-TEE core

-  Use `optee_test/script/file_to_c.py` to translate intermedia certificate file to `ca_chain.c`, the command is: 

    ```bash
    python file_to_c.py --name ca_chain --out ca_chain.c --inf ../cert/mid.crt
    ```

    Code excerpt：
```cpp=
/* automatically generated */
#include <stdint.h>
#include <stddef.h>

const uint8_t ca_chain[] = {
0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x42, 0x45, 0x47,
0x49, 0x4e, 0x20, 0x43, 0x45, 0x52, 0x54, 0x49,
0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d,
0x2d, 0x2d, 0x2d, 0x0a, 0x4d, 0x49, 0x49, 0x44,
0x49, 0x44, 0x43, 0x43, 0x41, 0x67, 0x69, 0x67,
0x41, 0x77, 0x49, 0x42, 0x41, 0x67, 0x49, 0x4a,
0x41, 0x4c, 0x66, 0x45, 0x4e, 0x61, 0x4d, 0x62,
0x33, 0x49, 0x76, 0x76, 0x4d, 0x41, 0x30, 0x47,
0x43, 0x53, 0x71, 0x47, 0x53, 0x49, 0x62, 0x33,
0x44, 0x51, 0x45, 0x42, 0x43, 0x77, 0x55, 0x41,
..............................................,
..............................................,
0x45, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x0a, '\0'};
const size_t ca_chain_size = sizeof(ca_chain) - 1;
```

- Put `ca_chain.c` in `optee_os/core/arch/arm/kernel/x509/` 
- Add the following line to `optee_os/core/arch/arm/kernel/sub.mk`
```shell
  srcs-y += x509/ca_chain.c
```

## Modify the TA loading verification mechanism

Modify `optee_os/core/arch/arm/kernel/ree_fs_ta.c`. 

- Add some header files and variables we need
    - Add the header files:
    ```c=
    #include <mbedtls/aes.h>
    #include <mbedtls/base64.h>
    #include <mbedtls/bignum.h>
    #include <mbedtls/des.h>
    #include <mbedtls/md5.h>
    #include <mbedtls/rsa.h>
    #include <mbedtls/sha1.h>
    #include <mbedtls/sha256.h>
    #include <mbedtls/x509_crt.h>
    #include <mbedtls/x509_csr.h>
    #include <mbedtls/x509.h>
    #include <tee_internal_api.h>
    #include <mbedtls/md.h>
    #include <mbedtls/pem.h>
    #include <mbedtls/pk.h>
    #include <mbedtls/md.h>
    ```
    - Add extern to get the variable defined in x509/ca_chain.c
    ```c=
    extern const uint8_t ca_chain[];
    extern const size_t ca_chain_size;
    ```
- Create certificate verification function

  Certificate verification function`verify_cert()` consists of three main parts：

    - Verify whether the third-party certificate and the intermediate certificate built in the trusted execution environment are issued by the same root CA. If yes, it is a trusted source.
    - Verify the signature with the public key embedded in the certificate
    - Calculate the digest of the  TA file. To ensure the integrity of TA file, OP-TEE core verifies the content of the digest field in the TA file with  the calculated digest (of the same TA file).

```c=
static TEE_Result verify_cert(const struct shdr *shdr)
{
	TEE_Result res = TEE_SUCCESS;
	unsigned char hash[32]={0};
	int ret = 0;
	uint32_t flags = 0;    
	mbedtls_x509_crt crt = { };
	mbedtls_x509_crt trust_crt = { };
	mbedtls_x509_crt_init(&crt);
	mbedtls_x509_crt_init(&trust_crt);
    
	const uint8_t *ta_cert=SHDR_GET_CERT(shdr);  

	//must +1 in certificate size to prevent format parsing error.
	ret = mbedtls_x509_crt_parse(&crt, ca_chain,
	                                    ca_chain_size+1);
	   if (ret) {
		      EMSG("ca error %d %x \n",ret,ret);
	             EMSG("ca mbedtls_x509_crt_parse: failed: %#x", ret);
	             return TEE_ERROR_BAD_FORMAT;
	   }else{
		        EMSG("ca cert ok \n");
	}
	uint8_t *buff=NULL;
	buff=malloc((shdr->cert_size)+1);
	memset(buff,'\0',(shdr->cert_size)+1);
	memcpy(buff,ta_cert,shdr->cert_size);
	   ret = mbedtls_x509_crt_parse(&trust_crt,buff,(shdr->cert_size)+1);
	if (ret) {
	          EMSG("ca2 mbedtls_x509_crt_parse: failed: %#x", ret);
	          res = TEE_ERROR_BAD_FORMAT;
	          goto out;
	   }else{
		EMSG("ca2  cert ok \n");
	}
	
	ret = mbedtls_x509_crt_verify(&trust_crt,&crt, NULL, NULL, &flags,NULL, NULL);
	   if (ret) {
		      char vrfy_buf[512];
	             EMSG("verify mbedtls_x509_crt_verify: failed: %#x", ret);
	       	  mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );
		      EMSG("error message----> %s\n",vrfy_buf);
	             res = TEE_ERROR_BAD_FORMAT;
	   }else{
		EMSG("compare ca and ca2 cert ok \n");
	}
		//DMSG("pk size=%d",sizeof(trust_crt.pk));
	if(!mbedtls_pk_can_do(&trust_crt.pk, MBEDTLS_PK_RSA )){
		DMSG("public key check error\n");
                res = TEE_ERROR_BAD_FORMAT;
		goto out;
	   }else{
		DMSG("public key check ok\n");
	}
	
//Verify Signature 
if( ( ret = mbedtls_pk_verify(&trust_crt.pk,MBEDTLS_MD_SHA256,SHDR_GET_HASH(shdr),0,SHDR_GET_SIG(shdr),
		   shdr->sig_size ) ) != 0 ){
	DMSG("sigature check error = > %d %x %x\n",ret,-ret,ret);
        res = TEE_ERROR_BAD_FORMAT;
	goto out;
   }else{
	DMSG("sigatrue check ok\n");
}
out:
        mbedtls_x509_crt_free(&trust_crt);
        mbedtls_x509_crt_free(&crt);
        return res;
}
```

- Modify `ree_fs_ta_open()` to invoke added verification mechanism

    - Call `verify_cert()` we added.
    - Add `handle->nw_ta_size = ta_size + shdr->cert_size`.
    ```c=
    static TEE_Result ree_fs_ta_open(const TEE_UUID *uuid,struct user_ta_store_handle **h)
    {
            ....
            verify_cert(shdr);    
            .....

            handle->nw_ta_size = ta_size + shdr->cert_size;
    }                 
    ```

Signed-off-by: Yu-Fan Huang <joe860314@g.ncu.edu.tw>
Acked-by: Chen-Chieh Chiu <ccchiu@g.ncu.edu.tw>
Suggested-by: Che-Chia Chang <vivahavey@gmail.com>

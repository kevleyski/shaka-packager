# Install script for directory: /Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/aes.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/aria.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/asn1.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/asn1write.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/base64.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/bignum.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/build_info.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/camellia.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ccm.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/chacha20.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/chachapoly.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/check_config.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/cipher.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/cmac.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/compat-2.x.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/config_psa.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/constant_time.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ctr_drbg.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/debug.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/des.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/dhm.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ecdh.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ecdsa.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ecjpake.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ecp.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/entropy.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/error.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/gcm.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/hkdf.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/hmac_drbg.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/mbedtls_config.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/md.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/md5.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/memory_buffer_alloc.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/net_sockets.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/nist_kw.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/oid.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/pem.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/pk.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/pkcs12.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/pkcs5.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/platform.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/platform_time.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/platform_util.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/poly1305.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/private_access.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/psa_util.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ripemd160.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/rsa.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/sha1.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/sha256.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/sha512.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ssl.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ssl_cache.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ssl_ciphersuites.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ssl_cookie.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/ssl_ticket.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/threading.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/timing.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/version.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/x509.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/x509_crl.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/x509_crt.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/mbedtls/x509_csr.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_builtin_composites.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_builtin_primitives.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_compat.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_config.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_driver_common.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_driver_contexts_composites.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_driver_contexts_primitives.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_extra.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_platform.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_se_driver.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_sizes.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_struct.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_types.h"
    "/Users/kevin.staunton-lambert/workspace/github_kevleyski/shaka-packager/packager/third_party/mbedtls/source/include/psa/crypto_values.h"
    )
endif()


#include <kamek.hpp>
#include <Network/SHA256.hpp>

#ifndef _WIILINK_TYPES_
#  define _WIILINK_TYPES_

#  define PROD

#  ifndef WWFC_DOMAIN

#    ifdef PROD
#      define WWFC_DOMAIN "zpltest.xyz"
#    else
#      define WWFC_DOMAIN "nwfc.wiinoma.com" // Points to localhost
#    endif

#  endif

#  define PAYLOAD_BLOCK_SIZE 0x20000

extern "C" u32 WWFC_CUSTOM_REGION; // 0x80005EFC

typedef struct {
    char magic[0xC]; // Always "WWFC/Payload"
    u32 total_size;
    u8 signature[0x100]; // RSA-2048 signature
} __attribute__((packed)) wwfc_payload_header;

typedef struct {
    u32 format_version; // Payload format version
    u32 format_version_compat; // Minimum payload format version that this
                               // payload is compatible with
    char name[0xC]; // Payload name (e.g. "RMCPD00")
    u32 version; // Payload version
    u32 got_start;
    u32 got_end;
    u32 fixup_start;
    u32 fixup_end;
    u32 patch_list_offset;
    u32 patch_list_end;
    u32 entry_point;
    u32 entry_point_no_got;
    u32 reserved[0x18 / 4];
    char build_timestamp[0x20];
} __attribute__((packed)) wwfc_payload_info;

typedef struct {
    wwfc_payload_header header;
    u8 salt[SHA256_DIGEST_SIZE];
    wwfc_payload_info info;
} __attribute__((packed)) wwfc_payload;

typedef enum {
    /**
     * Copy bytes specified in `args` to the destination `address`.
     * @param arg0 Pointer to the data to copy from.
     * @param arg1 Length of the data.
     */
    WWFC_PATCH_TYPE_WRITE = 0,

    /**
     * Write a branch: `address` = b `arg0`;
     * @param arg0 Branch destination address.
     * @param arg1 Not used.
     */
    WWFC_PATCH_TYPE_BRANCH = 1,

    /**
     * Write a branch with a branch back: `address` = b `arg0`; `arg1` = b
     * `address` + 4;
     * @param arg0 Branch destination address.
     * @param arg1 Address to write the branch back.
     */
    WWFC_PATCH_TYPE_BRANCH_HOOK = 2,

    /**
     * Write a branch with link: `address` = bl `arg0`
     * @param arg0 Branch destination address.
     * @param arg1 Not used.
     */
    WWFC_PATCH_TYPE_CALL = 3,

    /**
     * Write a branch using the count register:
     * `address` = \
     * lis `arg1`, `arg0`\@h; \
     * ori `arg1`, `arg1`, `arg0`\@l; \
     * mtctr `arg1`; \
     * bctr;
     * @param arg0 Branch destination address.
     * @param arg1 Temporary register to use for call.
     */
    WWFC_PATCH_TYPE_BRANCH_CTR = 4,

    /**
     * Write a branch with link using the count register:
     * `address` = \
     * lis `arg1`, `arg0`\@h; \
     * ori `arg1`, `arg1`, `arg0`\@l; \
     * mtctr `arg1`; \
     * bctrl;
     * @param arg0 Branch destination address.
     * @param arg1 Temporary register to use for call.
     */
    WWFC_PATCH_TYPE_BRANCH_CTR_LINK = 5,
} wwfc_patch_type;

/**
 * Flags for different patch levels.
 */
typedef enum {

    /**
     * Critical, used for security patches and other things required to connect
     * to the server. This has no value and is always automatically applied.
     */
    WWFC_PATCH_LEVEL_CRITICAL = 0, // 0x00

    /**
     * Patches that fix bugs in the game, such as anti-freeze patches.
     */
    WWFC_PATCH_LEVEL_BUGFIX = 1 << 0, // 0x01

    /**
     * Patches required for parity with clients using a regular WWFC patcher.
     */
    WWFC_PATCH_LEVEL_PARITY = 1 << 1, // 0x02

    /**
     * Additional feature, not required to be compatible with regular clients.
     */
    WWFC_PATCH_LEVEL_FEATURE = 1 << 2, // 0x04

    /**
     * General support patches that may be redundant depending on the patcher.
     * Used in cases like URL patches.
     */
    WWFC_PATCH_LEVEL_SUPPORT = 1 << 3, // 0x08

    /**
     * Flag used to disable the patch if it has been already applied.
     */
    WWFC_PATCH_LEVEL_DISABLED = 1 << 4, // 0x10
} wwfc_patch_level;

typedef struct {
    u8 level; // wwfc_patch_level
    u8 type; // wwfc_patch_type
    u8 reserved[2];
    u32 address;

    union {
        u32 arg0;
        const void* arg0p;
        const u32* arg0p32;
    };

    u32 arg1;
} __attribute__((packed)) wwfc_patch;

#  define WL_ERROR_PAYLOAD_OK 0
#  define WL_ERROR_PAYLOAD_STAGE0_MISSING_STAGE1 -20901
#  define WL_ERROR_PAYLOAD_STAGE0_HASH_MISMATCH -20902
#  define WL_ERROR_PAYLOAD_STAGE1_ALLOC -20910
#  define WL_ERROR_PAYLOAD_STAGE1_MAKE_REQUEST -20911
#  define WL_ERROR_PAYLOAD_STAGE1_RESPONSE -20912
#  define WL_ERROR_PAYLOAD_STAGE1_HEADER_CHECK -20913
#  define WL_ERROR_PAYLOAD_STAGE1_LENGTH_ERROR -20914
#  define WL_ERROR_PAYLOAD_STAGE1_SALT_MISMATCH -20915
#  define WL_ERROR_PAYLOAD_STAGE1_GAME_ID_MISMATCH -20916
#  define WL_ERROR_PAYLOAD_STAGE1_SIGNATURE_INVALID -20917
#  define WL_ERROR_PAYLOAD_STAGE1_WAITING -20918
#  define WL_ERROR_PAYLOAD_GAME_MISMATCH -20930

#  ifdef PROD

// Production payload key
static const unsigned char PayloadPublicKey[] = {
    0xfa, 0xb0, 0x97, 0x03, 0xe0, 0x25, 0xf4, 0x55, 0x67, 0x8f, 0xa0, 0x59,
    0xc8, 0xee, 0x2b, 0x54, 0x06, 0xde, 0x15, 0x75, 0xb4, 0x30, 0x31, 0xed,
    0xad, 0x6a, 0x29, 0xeb, 0x92, 0x3a, 0x39, 0x7a, 0xa2, 0x10, 0x6a, 0x03,
    0xff, 0x93, 0xa6, 0x15, 0xd3, 0x40, 0x81, 0x63, 0xd1, 0xa1, 0xe0, 0x73,
    0xd3, 0x93, 0x82, 0x55, 0x5b, 0xb0, 0x2d, 0x83, 0x40, 0xc9, 0x58, 0x8f,
    0x19, 0xdf, 0x3d, 0x85, 0x42, 0x86, 0x44, 0xb7, 0x4b, 0x4c, 0xee, 0xf4,
    0x30, 0xe3, 0x05, 0xb9, 0x61, 0x55, 0x89, 0x95, 0xc3, 0x57, 0x7b, 0x79,
    0x79, 0x59, 0x0b, 0x82, 0x86, 0x6e, 0xfe, 0x5d, 0xec, 0xa7, 0x66, 0x80,
    0xb4, 0x82, 0x0d, 0x75, 0xb9, 0x59, 0xa5, 0x83, 0x85, 0xe6, 0x10, 0x07,
    0x74, 0xf2, 0x4a, 0x52, 0x50, 0x7f, 0x1b, 0xde, 0xe1, 0xf3, 0xfb, 0xd2,
    0x8f, 0x3f, 0x22, 0x75, 0x30, 0x08, 0x29, 0xd7, 0x39, 0xe4, 0xe8, 0x18,
    0x5c, 0x96, 0xfc, 0x07, 0xe4, 0xfe, 0xbd, 0x91, 0xe5, 0x9d, 0x04, 0xda,
    0xbd, 0x99, 0x64, 0x65, 0x23, 0xe4, 0xa6, 0x10, 0x8a, 0x84, 0x76, 0x73,
    0x8f, 0xea, 0x07, 0x23, 0x1c, 0x94, 0x70, 0x93, 0x96, 0x82, 0x1e, 0x94,
    0xc5, 0x27, 0x00, 0x25, 0x61, 0x1c, 0xd4, 0x7d, 0xad, 0x51, 0x34, 0xbf,
    0xea, 0xc0, 0x3a, 0xd3, 0xbd, 0x77, 0x23, 0xfc, 0x1b, 0x86, 0xd1, 0x03,
    0x99, 0xc0, 0x41, 0x58, 0x7d, 0x61, 0x8c, 0xb9, 0xf6, 0x7e, 0x91, 0x64,
    0x0c, 0x56, 0xe7, 0xb5, 0xc8, 0xe4, 0xf5, 0xb4, 0x2c, 0x15, 0xf4, 0x6a,
    0x3d, 0xbd, 0xfd, 0xa1, 0x00, 0x33, 0x87, 0x6d, 0x08, 0x02, 0x93, 0x40,
    0x53, 0x19, 0xea, 0xa6, 0x7d, 0xcc, 0x59, 0x3e, 0xd7, 0x9a, 0xfd, 0x07,
    0x3d, 0xdd, 0xcd, 0x27, 0xa6, 0x7e, 0xba, 0x61, 0xcb, 0xe2, 0x6c, 0x36,
    0x6f, 0x35, 0x04, 0x22, 0xe6, 0xe6, 0xce, 0x41, 0x99, 0x7a, 0x79, 0xe2,
    0x14, 0x56, 0x8e, 0xb7, 0x38, 0xce, 0xa8, 0x2f, 0xcf, 0x2d, 0x26, 0xd9,
    0xd2, 0x01, 0x3e, 0xe6, 0x29, 0x31, 0x60, 0xef, 0x76, 0x33, 0x9d, 0x1a,
    0xac, 0x8a, 0x3e, 0x3b, 0x5a, 0xea, 0xc1, 0x1d, 0x10, 0x0f, 0x11, 0xeb,
    0x92, 0xdf, 0xcb, 0x57, 0x3f, 0x7f, 0x67, 0xf3, 0x84, 0xd5, 0x88, 0xec,
    0x55, 0xc6, 0x88, 0x37, 0xa8, 0xae, 0x7d, 0xd6, 0xb8, 0xb3, 0x9f, 0xa9,
    0x2f, 0x0a, 0x0a, 0xe2, 0x60, 0x42, 0x84, 0x98, 0x1f, 0xb8, 0xfa, 0x67,
    0xdc, 0xcd, 0x16, 0xe5, 0xdd, 0x39, 0x27, 0xff, 0x61, 0x83, 0x7c, 0xd3,
    0x1c, 0x26, 0xef, 0x49, 0xbd, 0x77, 0x22, 0x2a, 0xc2, 0xd0, 0x22, 0x84,
    0xd9, 0x58, 0x0d, 0xd1, 0xfe, 0x49, 0x92, 0x4e, 0xdd, 0x66, 0xa1, 0x0f,
    0x60, 0x22, 0x56, 0x43, 0xca, 0xc3, 0xa4, 0x11, 0x2f, 0x02, 0x73, 0x85,
    0xe4, 0x7e, 0xc5, 0xbb, 0xad, 0xe8, 0xe0, 0xa7, 0x02, 0x3f, 0x42, 0xf0,
    0xfb, 0xb8, 0x3f, 0x29, 0xb3, 0xd6, 0x5b, 0xd0, 0x3b, 0x39, 0x70, 0xcd,
    0xcb, 0x4f, 0xdd, 0xf2, 0xe1, 0x7c, 0xf8, 0xee, 0xde, 0x0f, 0x88, 0x28,
    0xab, 0x01, 0x2e, 0xcd, 0xca, 0xf4, 0x60, 0x57, 0xc1, 0xd3, 0x64, 0x82,
    0xa6, 0x1b, 0x10, 0xa2, 0x93, 0xef, 0xf1, 0x56, 0x3a, 0x0e, 0x25, 0xe5,
    0x84, 0xf7, 0x2e, 0xb4, 0x14, 0x01, 0x87, 0xda, 0x7b, 0x28, 0xcc, 0xb9,
    0xd4, 0xde, 0x7c, 0xc6, 0x16, 0xee, 0x05, 0xae, 0x18, 0x7e, 0x39, 0x85,
    0x4f, 0xa0, 0x10, 0x44, 0x09, 0x21, 0xe8, 0xb1, 0x81, 0x4a, 0x8b, 0xc2,
    0xe3, 0xa3, 0x73, 0x75, 0x8c, 0xbe, 0x60, 0x32, 0xbc, 0x41, 0x16, 0x94,
    0x0a, 0xe9, 0xc9, 0x6a, 0x0f, 0x83, 0xe7, 0x4d, 0x2e, 0xb8, 0xa8, 0x52,
    0xe9, 0x5d, 0xda, 0x96, 0x68, 0x0d, 0x4b, 0xd7, 0x0c, 0x14, 0xbe, 0xb6,
};

#  else

// Development payload key
static const unsigned char PayloadPublicKey[] = {
    0x3f, 0x32, 0xeb, 0x87, 0xb9, 0x75, 0x15, 0xc9, 0x60, 0x17, 0x7c, 0xcf,
    0x52, 0x18, 0x36, 0x66, 0x8e, 0xe9, 0x26, 0xe8, 0x58, 0x1f, 0xff, 0x84,
    0xca, 0x63, 0x63, 0x9f, 0x4f, 0xa5, 0x77, 0x41, 0xe4, 0x4e, 0x5c, 0x49,
    0xb7, 0x54, 0x15, 0xf5, 0x74, 0x70, 0xbc, 0xf4, 0xce, 0x5e, 0x66, 0x78,
    0xf1, 0xe5, 0xfe, 0x3a, 0x98, 0x18, 0xbd, 0x8f, 0x9a, 0xb7, 0x52, 0x99,
    0x7c, 0x80, 0x00, 0x81, 0xf3, 0xc0, 0xce, 0xf1, 0x8b, 0xac, 0x43, 0x18,
    0x44, 0x95, 0x7e, 0xb8, 0x06, 0x41, 0x0e, 0xa7, 0x3e, 0x0e, 0xc0, 0x68,
    0x01, 0x69, 0x88, 0x86, 0x81, 0x2f, 0xfe, 0x5d, 0xfb, 0x62, 0xcf, 0x0e,
    0xdf, 0x4b, 0x1c, 0x67, 0x0f, 0x09, 0xaf, 0x37, 0x40, 0x60, 0x06, 0x34,
    0xe4, 0xc8, 0x98, 0x55, 0xf3, 0xd0, 0xb5, 0xfe, 0x8c, 0x92, 0x92, 0xe0,
    0x69, 0xd9, 0x02, 0x7d, 0xe5, 0x1e, 0x55, 0x00, 0x1c, 0xdf, 0x44, 0xd6,
    0x51, 0x4b, 0xd6, 0xd3, 0xe2, 0xbc, 0xc0, 0xca, 0xf8, 0x42, 0x90, 0xee,
    0x90, 0xf7, 0xc5, 0xce, 0x6d, 0xd2, 0x3d, 0x9e, 0x26, 0x6d, 0x97, 0xe0,
    0xc0, 0x82, 0x8c, 0x63, 0xdc, 0xd6, 0x80, 0x67, 0xc7, 0x58, 0x72, 0xd2,
    0xbe, 0x2f, 0xe4, 0x39, 0x5a, 0x6c, 0x0d, 0x8a, 0xb4, 0x88, 0xef, 0x3d,
    0xd2, 0xde, 0xda, 0x48, 0x62, 0xda, 0x9e, 0x5f, 0xce, 0x3e, 0xd9, 0x09,
    0x1e, 0xc6, 0x74, 0x4d, 0x74, 0xd2, 0x6f, 0xba, 0x83, 0xc0, 0x4b, 0x7e,
    0xe0, 0x76, 0x2c, 0x45, 0xa8, 0x32, 0xac, 0xbc, 0x8d, 0x65, 0xdf, 0xdc,
    0x9d, 0x00, 0x2e, 0x8a, 0xf8, 0x4b, 0x82, 0x35, 0xbd, 0xe3, 0x04, 0x91,
    0xae, 0x3d, 0x5b, 0xe9, 0xef, 0x85, 0xc2, 0xb1, 0xd5, 0xd4, 0xf6, 0x1d,
    0x30, 0xbc, 0xcd, 0x3e, 0x38, 0xf8, 0x51, 0x55, 0xfc, 0x5a, 0x1b, 0x64,
    0x3f, 0x64, 0x37, 0x2e, 0xe5, 0xe7, 0x0d, 0x68, 0x88, 0xfc, 0x55, 0x3d,
    0x69, 0x5e, 0x4d, 0xcd, 0x18, 0xea, 0xd3, 0xd7, 0x41, 0x4c, 0x64, 0x82,
    0x9f, 0x4f, 0xc9, 0x97, 0x0a, 0xe9, 0x74, 0x3c, 0x94, 0xf9, 0x82, 0x7f,
    0x3e, 0x89, 0x9a, 0x4e, 0x62, 0x0e, 0x99, 0xb9, 0x4f, 0xbb, 0xef, 0x05,
    0x18, 0x1f, 0x61, 0xc7, 0xe1, 0xed, 0xf8, 0x70, 0xbf, 0x81, 0xe6, 0x9c,
    0x58, 0xb4, 0x9a, 0xdf, 0x30, 0xdc, 0xbe, 0xb3, 0x38, 0x9f, 0x53, 0x57,
    0x7c, 0xa7, 0x1e, 0xe5, 0xd9, 0x11, 0x67, 0x09, 0x09, 0xc0, 0xc6, 0x20,
    0x69, 0x60, 0xda, 0xd3, 0x1d, 0x87, 0x0e, 0xed, 0x15, 0xf0, 0x85, 0x2c,
    0x68, 0xcd, 0x18, 0xa6, 0x07, 0x04, 0xc1, 0xf9, 0xa4, 0xfb, 0x74, 0x56,
    0x98, 0xd7, 0x46, 0xfc, 0x31, 0x47, 0x6a, 0x09, 0x04, 0xa3, 0x76, 0x3b,
    0x26, 0x83, 0x34, 0xde, 0xd2, 0x22, 0xf2, 0x3f, 0xd4, 0xd9, 0xbb, 0xf0,
    0xba, 0x3f, 0xb3, 0x1e, 0x96, 0x44, 0x04, 0xa1, 0xc8, 0x40, 0xb2, 0x0c,
    0x8a, 0x07, 0xea, 0x16, 0x7d, 0x57, 0x49, 0x9e, 0x6b, 0xec, 0xd3, 0xe1,
    0x77, 0x29, 0x35, 0x6f, 0x82, 0xd7, 0xb8, 0x5a, 0x2b, 0xf3, 0x1a, 0x79,
    0xf9, 0x84, 0xaf, 0x7a, 0x35, 0x85, 0x26, 0xb4, 0xfc, 0x58, 0x2f, 0x89,
    0x2a, 0x97, 0xc8, 0x49, 0xb3, 0x61, 0xbd, 0xf5, 0xda, 0xe9, 0x87, 0x9b,
    0xea, 0x5c, 0x4c, 0x15, 0x19, 0x1f, 0x76, 0x0c, 0x01, 0x6f, 0x32, 0x80,
    0x73, 0xd1, 0xc4, 0x4a, 0x87, 0x32, 0x1e, 0xd6, 0xdf, 0xd9, 0x0a, 0x27,
    0x66, 0xb2, 0x74, 0xc1, 0xca, 0xae, 0x16, 0xca, 0x17, 0x66, 0x09, 0xe9,
    0x37, 0xca, 0x20, 0xfe, 0xa2, 0xa6, 0x7b, 0x2c, 0xd9, 0x6d, 0x91, 0x26,
    0xe6, 0x61, 0xdd, 0xbb, 0x11, 0x18, 0x7c, 0xfb, 0x8f, 0x43, 0x71, 0x6b,
    0x6a, 0xa2, 0x0a, 0xd3, 0xc6, 0x98, 0x94, 0xd8, 0x63, 0xb0, 0x49, 0xf5,
};

#  endif

#endif
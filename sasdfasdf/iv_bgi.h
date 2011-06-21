#ifndef IV_BGI_H_
#define IV_BGI_H_
#include "iv_util.h"
#include "crass_types.h"

#pragma pack (1)
typedef struct {
  s8 magic[12];		/* "PackFile    " */
  u32 entries;
} arc_header_t;

typedef struct {
  s8 name[16];
  u32 offset;
  u32 length;
  u32 reserved[2];
} arc_entry_t;

typedef struct {
  s8 magic[16];			/* "DSC FORMAT 1.00" */
  u32 key;
  u32 uncomprlen;
  u32 dec_counts;			/* 解密的次数 */
  u32 reserved;
} dsc_header_t;

typedef struct {
  s8 magic[16];			/* "SDC FORMAT 1.00" */
  u32 key;
  u32 comprlen;
  u32 uncomprlen;
  u16 sum_check;			/* 累加校验值 */
  u16 xor_check;			/* 累异或校验值 */
} gdb_header_t;

typedef struct {
  u32 header_length;
  u32 unknown0;
  u32 length;				/* 实际的数据长度 */
  u32 unknown[13];
} snd_header_t;

typedef struct {
  u16 width;
  u16 height;
  u32 color_depth;
  u32 reserved0[2];
} grp_header_t;

typedef struct {
  s8 magic[16];			/* "CompressedBG___" */
  u16 width;
  u16 height;
  u32 color_depth;
  u32 reserved0[2];
  u32 zero_comprlen;		/* huffman解压后的0值字节压缩的数据长度 */
  u32 key;				/* 解密叶节点权值段用key */
  u32 encode_length;		/* 加密了的叶节点权值段的长度 */
  u8 sum_check;
  u8 xor_check;
  u16 reserved1;
} bg_header_t;
#pragma pack ()

/********************* DSC FORMAT 1.00 *********************/

struct dsc_huffman_code {
  u16 code;
  u16 depth;		/* 该叶节点所在的depth（0开始） */
};

struct dsc_huffman_node {
  unsigned int is_parent;
  unsigned int code;	/* 对于小于256的，code就是字节数据；对于大于256的，高字节的1是标志（lz压缩），低字节是copy_bytes（另外需要+2） */
  unsigned int left_child_index;
  unsigned int right_child_index;
};

/*********** bits API **************/

struct bits {
  unsigned long curbits;
  unsigned long curbyte;
  unsigned char cache;
  unsigned char *stream;
  unsigned long stream_length;
};

void bits_init(struct bits *bits, unsigned char *stream, unsigned long stream_length);
int bits_get_high(struct bits *bits, bool & retval);
int bits_get_high(struct bits *bits, unsigned int req_bits, unsigned int & retval);
void bits_flush(struct bits *bits);

/**/
int bgi_arc_extract_resource(const char *ifname);


#endif

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
  u32 dec_counts;			/* ���ܵĴ��� */
  u32 reserved;
} dsc_header_t;

typedef struct {
  s8 magic[16];			/* "SDC FORMAT 1.00" */
  u32 key;
  u32 comprlen;
  u32 uncomprlen;
  u16 sum_check;			/* �ۼ�У��ֵ */
  u16 xor_check;			/* �����У��ֵ */
} gdb_header_t;

typedef struct {
  u32 header_length;
  u32 unknown0;
  u32 length;				/* ʵ�ʵ����ݳ��� */
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
  u32 zero_comprlen;		/* huffman��ѹ���0ֵ�ֽ�ѹ�������ݳ��� */
  u32 key;				/* ����Ҷ�ڵ�Ȩֵ����key */
  u32 encode_length;		/* �����˵�Ҷ�ڵ�Ȩֵ�εĳ��� */
  u8 sum_check;
  u8 xor_check;
  u16 reserved1;
} bg_header_t;
#pragma pack ()

/********************* DSC FORMAT 1.00 *********************/

struct dsc_huffman_code {
  u16 code;
  u16 depth;		/* ��Ҷ�ڵ����ڵ�depth��0��ʼ�� */
};

struct dsc_huffman_node {
  unsigned int is_parent;
  unsigned int code;	/* ����С��256�ģ�code�����ֽ����ݣ����ڴ���256�ģ����ֽڵ�1�Ǳ�־��lzѹ���������ֽ���copy_bytes��������Ҫ+2�� */
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

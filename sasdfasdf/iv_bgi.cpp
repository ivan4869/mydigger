#include "iv_util.h"
#include "iv_bgi.h"

//Decryption
static u8 update_key(u32 *key, u8 *magic)
{
  u32 v0, v1;

  v0 = (*key & 0xffff) * 20021;
  v1 = (magic[1] << 24) | (magic[0] << 16) | (*key >> 16);
  v1 = v1 * 20021 + *key * 346;
  v1 = (v1 + (v0 >> 16)) & 0xffff;
  *key = (v1 << 16) + (v0 & 0xffff) + 1;

  return v1 & 0xffff;
}

static int dsc_huffman_code_compare(const void *code1, const void *code2)
{
  struct dsc_huffman_code *codes[2] = { (struct dsc_huffman_code *)code1, (struct dsc_huffman_code *)code2 };
  int compare = (int)codes[0]->depth - (int)codes[1]->depth;

  if (!compare)
    return (int)codes[0]->code - (int)codes[1]->code;

  return compare;
}

static void dsc_create_huffman_tree(struct dsc_huffman_node *huffman_nodes,
  struct dsc_huffman_code *huffman_code, 
  unsigned int leaf_node_counts)
{
  unsigned int nodes_index[2][512];	/* 构造当前depth下节点索引的辅助用数组，
                                    * 其长度是当前depth下可能的最大节点数 */
  unsigned int next_node_index = 1;	/* 0已经默认分配给根节点了 */
  unsigned int depth_nodes = 1;		/* 第0深度的节点数(2^N)为1 */
  unsigned int depth = 0;				/* 当前的深度 */	
  unsigned int switch_flag = 0;

  nodes_index[0][0] = 0;
  for (unsigned int n = 0; n < leaf_node_counts; ) {
    unsigned int depth_existed_nodes;/* 当前depth下创建的叶节点计数 */
    unsigned int depth_nodes_to_create;	/* 当前depth下需要创建的节点数（对于下一层来说相当于父节点） */
    unsigned int *huffman_nodes_index;
    unsigned int *child_index;

    huffman_nodes_index = nodes_index[switch_flag];
    switch_flag ^= 1;
    child_index = nodes_index[switch_flag];

    depth_existed_nodes = 0;
    /* 初始化隶属于同一depth的所有节点 */
    while (huffman_code[n].depth == depth) {
      struct dsc_huffman_node *node = &huffman_nodes[huffman_nodes_index[depth_existed_nodes++]];
      /* 叶节点集中在左侧 */
      node->is_parent = 0;
      node->code = huffman_code[n++].code;
    }
    depth_nodes_to_create = depth_nodes - depth_existed_nodes;
    for (unsigned int i = 0; i < depth_nodes_to_create; i++) {
      struct dsc_huffman_node *node = &huffman_nodes[huffman_nodes_index[depth_existed_nodes + i]];

      node->is_parent = 1;
      child_index[i * 2 + 0] = node->left_child_index = next_node_index++;
      child_index[i * 2 + 1] = node->right_child_index = next_node_index++;
    }
    depth++;
    depth_nodes = depth_nodes_to_create * 2;	/* 下一depth可能的节点数 */
  }
}


static unsigned int dsc_huffman_decompress(struct dsc_huffman_node *huffman_nodes, 
  unsigned int dec_counts, unsigned char *uncompr, 
  unsigned int uncomprlen, unsigned char *compr, 
  unsigned int comprlen)
{
  struct bits bits;
  unsigned int act_uncomprlen = 0;
  int err = 0;

  bits_init(&bits, compr, comprlen);	
  for (unsigned int k = 0; k < dec_counts; k++) {
    bool child = 0;
    unsigned int node_index;
    unsigned int code;

    node_index = 0;
    do {
      if (bits_get_high(&bits, child))
        goto out;

      if (!child)
        node_index = huffman_nodes[node_index].left_child_index;
      else
        node_index = huffman_nodes[node_index].right_child_index;
    } while (huffman_nodes[node_index].is_parent);

    code = huffman_nodes[node_index].code;

    if (code >= 256) {	// 12位编码的win_pos, lz压缩
      unsigned int copy_bytes, win_pos;

      copy_bytes = (code & 0xff) + 2;
      if (bits_get_high(&bits, 12, win_pos)) 
        break;

      win_pos += 2;			
      for (unsigned int i = 0; i < copy_bytes; i++) {
        uncompr[act_uncomprlen] = uncompr[act_uncomprlen - win_pos];
        act_uncomprlen++;
      }
    } else
      uncompr[act_uncomprlen++] = code;
  }
out:
  return act_uncomprlen;
}

static unsigned int dsc_decompress(dsc_header_t *dsc_header, unsigned int dsc_len, 
                                   unsigned char *uncompr, unsigned int uncomprlen)
{
  /* 叶节点编码：前256是普通的单字节数据code；后256是lz压缩的code（每种code对应一种不同的copy_bytes */
  struct dsc_huffman_code huffman_code[512];
  /* 潜在的，相当于双字节编码（单字节ascii＋双字节lz） */
  struct dsc_huffman_node huffman_nodes[1023];	
  unsigned char magic[2];
  unsigned int key;
  unsigned char *enc_buf;	

  enc_buf = (unsigned char *)(dsc_header + 1);
  magic[0] = dsc_header->magic[0];
  magic[1] = dsc_header->magic[1];
  key = dsc_header->key;

  /* 解密叶节点深度信息 */
  memset(huffman_code, 0, sizeof(huffman_code));
  memset(huffman_nodes, 0, sizeof(huffman_nodes));
  unsigned int leaf_node_counts = 0;
  for (unsigned int i = 0; i < 512; i++) {
    u8 depth= enc_buf[i];
    u8 tmp =  update_key(&key, magic);

    depth -= tmp;
    if (depth) {
      huffman_code[leaf_node_counts].depth = depth;
      huffman_code[leaf_node_counts].code = (unsigned short)i;
      leaf_node_counts++;
    }
  }	

  qsort(huffman_code, leaf_node_counts, sizeof(struct dsc_huffman_code), dsc_huffman_code_compare);

  dsc_create_huffman_tree(huffman_nodes, huffman_code, leaf_node_counts);

  unsigned char *compr = enc_buf + 512;
  unsigned int act_uncomprlen;
  act_uncomprlen = dsc_huffman_decompress(huffman_nodes, dsc_header->dec_counts, 
    uncompr, dsc_header->uncomprlen, compr, dsc_len - sizeof(dsc_header_t) - 512);

  return act_uncomprlen;
}

int bgi_arc_extract_resource(char *ifname)
{
  int result=0;
  arc_header_t *arc_header;
  //struct stat ifstat;
  int ifid;
  unsigned long long offset=0;
  arc_entry_t *arc_entry, *arc_entries;// 

  //iv::wstat(ifname, &ifstat);


  ifid = iv::checkopen(ifname, _O_RDONLY|_O_BINARY);

  arc_header = new arc_header_t;
  if(!arc_header){
    iv::close(ifid);
    return -1;
  }
  if(-1 == iv::read(ifid, arc_header, sizeof(*arc_header))){
    delete arc_header;
    iv::close(ifid);
    return -1;
  }

  if(memcmp(arc_header->magic, "PackFile    ", 12) || !arc_header->entries){
    delete arc_header;
    iv::close(ifid);
    return -1;
  }

  arc_entries = arc_entry = new  arc_entry_t[arc_header->entries];
  if(-1 == iv::read(ifid, arc_entry, sizeof(arc_entry_t)*arc_header->entries)){
    delete arc_header;
    delete arc_entries;
    return -1;
  }

  offset = iv::tell(ifid);//密文开始的地方

  for(size_t index=0; index < arc_header->entries; ++index){
    dsc_header_t *dsc_header;
    unsigned char *compr, *uncompr;
    unsigned int act_uncomprlen = 0;

    //if(iv::read(ifid, compr, ifstat.st_size)){
    //  delete [] compr;
    //  return -1;
    //}

    compr = new unsigned char [arc_entry->length];
    if(!compr){
      result = -1;
      break;
    }
    iv::lseek(ifid, offset+arc_entry->offset);
    if(-1 == iv::read(ifid, compr, arc_entry->length)){
      delete [] compr;
      result = -1;
      break;
    }
    dsc_header = (dsc_header_t *) compr;

    if(!memcmp(dsc_header->magic, "DSC FORMAT 1.00", 16)){

      uncompr = (unsigned char *) new unsigned char[dsc_header->uncomprlen];
      if(!uncompr){
        delete [] compr;
        result = -1;
        break;
      }

      act_uncomprlen = dsc_decompress(dsc_header, arc_entry->length, uncompr, dsc_header->uncomprlen);
      if(act_uncomprlen != dsc_header->uncomprlen){
        delete [] uncompr;
        delete [] compr;
        result = -1;
        break;
      }

      char fname[32] = {'0'};
      memcpy(fname, arc_entry->name, sizeof(fname)-1);
      iv::write_file(&fname[0], uncompr, act_uncomprlen);

      delete [] uncompr;
      delete [] compr;
    }
    else{
      char unknown[32] = {'0'};

      memcpy(unknown, dsc_header->magic, 16);
      cerr << "Unknown header was found :" << unknown << endl;

      delete [] compr;
    }

    arc_entry++;
  }//for(size_t index=0; index < arc_header->entries; ++index)

  delete [] arc_entries;
  delete arc_header;

  return result;
}

int bgi_arc_pack(char *ifname[], size_t cnt, char * ofname)
{
  int result=0;
  arc_header_t arc_header;
  struct stat ifstat;
  int ifid, ofid;
  unsigned long long offset=0;
  arc_entry_t *p_arc_entry, *p_arc_entries;// 

  if(!ifname || !cnt || !ofname){
    cerr << "Bad arg in func:bgi_arc_pack" << endl;
    return -1;
  }
  p_arc_entry = p_arc_entries = new arc_entry_t[cnt];
  if(!p_arc_entries){
    cerr << "Failed to open alloc the memory for p_arc_entries" << endl;
    return -1;
  }
  memset(p_arc_entries, '\0', sizeof(arc_entry_t)*cnt);

  for(size_t i=0; i<cnt; i++){
    if(-1 == iv::stat(ifname, &ifstat)){
      delete [] p_arc_entries;
      return -1;
    }
    strncpy(p_arc_entry->name, ifname[i], 15);
    p_arc_entry->length = ifstat.st_size;
    p_arc_entry++;
  }

  ofid = iv::checkopen(ofname, _O_WRONLY|_O_BINARY|_O_TRUNC, _S_IWRITE);
  if(-1 == ofid){
    cerr << "Failed to open file " << ofname << endl;
    delete [] p_arc_entries;
    return -1;
  }

  memcpy(arc_header->magic, "PackFile    ", 12);
  arc_header->entries = cnt;
  iv::write(ofid, &arc_header, sizeof(arc_header));

  arc_entries = arc_entry = new arc_entry_t[arc_header->entries];

  if(-1 == iv::write(ifid, arc_entry, sizeof(arc_entry_t)*arc_header->entries)){
    delete p_arc_header;
    delete p_arc_entries;
    return -1;
  }

  offset = iv::tell(ofid); //密文开始的地方
  p_arc_entry = p_arc_entries;
  for(size_t index=0; index < arc_header->entries; ++index){
    dsc_header_t dsc_header;
    unsigned char *compr, *uncompr;
    unsigned int act_uncomprlen = 0;

    ifid = iv::checkopen(ifname[index], _O_RDONLY|_O_BINARY);

    uncompr = new unsigned char [];
    if(!compr){
      result = -1;
      break;
    }
    iv::lseek(ifid, offset+arc_entry->offset);
    if(-1 == iv::read(ifid, compr, arc_entry->length)){
      delete [] compr;
      result = -1;
      break;
    }
    dsc_header = (dsc_header_t *) compr;

    if(!memcmp(dsc_header->magic, "DSC FORMAT 1.00", 16)){
      uncompr = (unsigned char *) new unsigned char[dsc_header->uncomprlen];
      if(!uncompr){
        delete [] compr;
        result = -1;
        break;
      }

      act_uncomprlen = dsc_decompress(dsc_header, arc_entry->length, uncompr, dsc_header->uncomprlen);
      if(act_uncomprlen != dsc_header->uncomprlen){
        delete [] uncompr;
        delete [] compr;
        result = -1;
        break;
      }

      char fname[32] = {'0'};
      memcpy(fname, arc_entry->name, sizeof(fname)-1);
      iv::write_file(&fname[0], uncompr, act_uncomprlen);

      delete [] uncompr;
      delete [] compr;
    }
    else{
      char unknown[32] = {'0'};

      memcpy(unknown, dsc_header->magic, 16);
      cerr << "Unknown header was found :" << unknown << endl;

      delete [] compr;
    }

    arc_entry++;
  }//for(size_t index=0; index < arc_header->entries; ++index)

  delete [] arc_entries;
  delete arc_header;

  return 0;
}



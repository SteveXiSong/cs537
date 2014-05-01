#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include "udp.h"
#include "mfs.h"
#include "msg.h"

#define MFS_INUM               (4096) // inode number
#define MFS_IMAPNUM            (256)  // imap number
#define MFS_DIRECT_P           (14)   // number of data blocks inode points to
#define MFS_NODE_PER_MAP       (16)
#define MFS_NAME_SIZE          (60)

typedef struct __MFS_CheckRegion_t{
    int log_end;
    int imap_ptr[MFS_IMAPNUM]; // each entry contains the offset of an inode map
} MFS_CR_t;

typedef struct __MFS_inode_t{
    int size;
    int type;
    int ptr[MFS_DIRECT_P];  // each one is the offset of the data block
} MFS_Inode_t;

typedef struct __MFS_imap_t{
    int imap[MFS_NODE_PER_MAP];
} MFS_Imap_t;



// NOTE: called under condition that fd > 0, no inner fd check mechanism
int img_init(char* fimage);
int img_mount(char* fimage);

int img_lookup(int fd, int pinum, char* name);
int img_stat(int fd, int inum, MFS_Stat_t* m);
int img_write(int fd, int inum, char* buffer, int block);
int img_read(int fd, int inum, char*buffer, int block);
int img_creat(int fd, int pinum, int type, char* name);
int img_unlink(int fd, int pinum, char* name);
int img_shut(int fd);

int get_inode(int fd, int inum, MFS_Inode_t* inode);
int get_imap(int fd, int inum, MFS_Imap_t* imap);
int inode_alloc();
int update_CR(int log_end, int inum, int offset, int mode);
int entry_creat(MFS_DirEnt_t* dir, int inum, char* name);

//global variables
MFS_CR_t CR;
int inode_box[MFS_INUM]; // used to alloc inode

int main(int argc, char *argv[])
{
    if(argc != 3){
	printf("usage: server [portnum] [file-system-image] \n");
	return 0;
    }
    int port = atoi(argv[1]);   char* fimage = argv[2];

    // init inode_box
    
    int f_access = access(fimage, W_OK | R_OK);
    int fd;
    if(f_access == -1){
	printf("not exist, create and init\n");
	fd = img_init(fimage);
    } 
    else{
	fd = img_mount(fimage);
	}

    if(fd > 0){

	int sd = UDP_Open(port);
	assert(sd > -1);
	printf(" SERVER:: waiting in loop\n");

	while(1){
	    struct sockaddr_in s;
	    struct msg m;
	    int rc = UDP_Read(sd, &s, (char*)&m, sizeof(m));
	    if(rc > 0){

		switch(m.msg_type){

		case LOOKUP:
		    printf("lookup\n");
		    m.inum  = img_lookup(fd, m.pinum, m.name);
   		    UDP_Write(sd, &s, (char*)&m, sizeof(m));
		    break;

		case STAT:
		    printf("stat\n");
		    m.rspns =  img_stat(fd, m.inum, &(m.stat));
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));

		    break;
		case WRITE:
		    printf("write\n");
		    m.rspns = img_write(fd, m.inum, m.buffer, m.block);
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));

		    break;
		case READ:
		    printf("read\n");
		    m.rspns = img_read(fd, m.inum, m.buffer, m.block);
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));

		    break;
		case CREAT:
		    printf("create\n");
		    m.rspns = img_creat(fd, m.pinum, m.type, m.name);
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));

		    break;
		case UNLINK:
		    printf("unlink\n");
		    m.rspns = img_unlink(fd, m.pinum, m.name);
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));

		    break;

		case SHUT:
		    printf("shutdown\n");
		    m.rspns = img_shut(fd);
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));
		    exit(0);
		    break;

		default:
		    printf("error message\n");
		    m.rspns = -1;
		    UDP_Write(sd, &s, (char*)&m, sizeof(m));
		    break;
		}

	    }
	    else
		return 0; // get message fail
	}

    }
    else {
	printf("init / mount file image error\n");
	return -1;
    }
  return 0;
}

/************************* functions **************************/

int img_shut(int fd){
    int syn = fsync(fd);
    if(syn < 0)
	return -1;
    else
	return 0;
}


int img_unlink(int fd, int pinum, char* name){
    MFS_Stat_t p_stat;
    int p_s = img_stat(fd, pinum, &p_stat);
    if(p_s < 0){printf("img_unlink: get inode fail, invalida inode\n"); return -1;}

    if(p_stat.type != MFS_DIRECTORY){
	printf("img_unlink: given inode number is not directory\n");
	return -1;
    }

    int rm_inum = img_lookup(fd, pinum, name);
    if(rm_inum < 0){ // name doesnt exist
	printf("img_unlink: name doesnt exist, it's ok\n");
	return 0;
    }

    MFS_Inode_t rm_inode;
    int gt = get_inode(fd, rm_inum, &rm_inode);
    if(gt < 0){
	printf("img_unlink: get inode of name error\n");
	return -1;
    }

    if(rm_inode.type == MFS_DIRECTORY){
	// check if it is empty
	int i, j;
	char rd_data[MFS_BLOCK_SIZE];
	for(i = 0; i <MFS_DIRECT_P; i++){
	    int dir_offset = rm_inode.ptr[i];
	    if(dir_offset < 0){
		continue;
	    }
	    lseek(fd, dir_offset, SEEK_SET);
	    int in = read(fd, rd_data, MFS_BLOCK_SIZE);
	    if(in < 0){printf("img_unlink: read dir fail\n"); return -1;}
	    MFS_DirEnt_t* dir_in = (MFS_DirEnt_t*)rd_data;
	    for(j = 0; j < (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t)); j++){

		if((strcmp(dir_in->name, ".") == 0) || (strcmp(dir_in->name, "..") == 0)){
		    dir_in++;  // check next block
		    continue;
		}
		else{
		    if(dir_in->inum != -1){
		    printf("img_unlink: the directory is not empty\n");
		    return -1;  // can't remove it
		    }
		    dir_in++;
		    continue;
		}
	    }
	}
	// after the check, a is free;
    }

    // as file can always be moved , no need to specify the type if can move
    //1. unlink from imap
    MFS_Imap_t rm_imap;
    int gt_imap = get_imap(fd, rm_inum, &rm_imap);
    if(gt_imap < 0){printf("get imap fail\n"); return -1;}
    // unlink and check if rm_inum is the last one
    rm_imap.imap[rm_inum%MFS_NODE_PER_MAP] = -1;
    int i;
    for(i = 0; i < MFS_NODE_PER_MAP; i++){
	if(rm_imap.imap[i] > 0)
	    break;
    }
    if(i == 14){ // it's all empty, unlink from CR
	CR.imap_ptr[rm_inum/MFS_NODE_PER_MAP] = -1;

    } else{
	// attach it to the end
	lseek(fd, 0, SEEK_END);
	int out = write(fd, &rm_imap, sizeof(MFS_Imap_t));
	if(out < 0){printf("img_unlink: WB imap of name 1st time, fail\n"); return -1;}
	CR.imap_ptr[rm_inum/MFS_NODE_PER_MAP] = CR.log_end;
	CR.log_end += sizeof(MFS_Imap_t);
    }
    // write back CR, in case later use
    lseek(fd, 0, SEEK_SET);
    int out = write(fd, &CR, sizeof(MFS_CR_t));
    if(out < 0){printf("img_unlink: WB CR 1st time, fail\n"); return -1;}

    int syn = fsync(fd);
    if(syn < 0)
	return -1;

    // 2. unlink from inode_box
    inode_box[rm_inum] = -1;

    // 3. unlink from p directory
    MFS_Inode_t pinode_in;
    int gt_pinode = get_inode(fd, pinum, &pinode_in);
    if(gt_pinode < 0){
	printf("img_unlink: get pinode error\n");
	return -1; // unlink fail
    }
    int i_b, j_e;
    int found = 0;
    char rd_pdata[MFS_BLOCK_SIZE];

    for(i_b = 0; (i_b < MFS_DIRECT_P) && !found; i_b++){
	int pdir_offset = pinode_in.ptr[i_b];
	if(pdir_offset < 0){ // read error or block empty
	    continue;
	}
	lseek(fd, pdir_offset, SEEK_SET);
	int in = read(fd, rd_pdata, MFS_BLOCK_SIZE);
	if(in < 0){printf("img_unlink: read pdir fail\n"); return -1;}
	MFS_DirEnt_t* pdir_in = (MFS_DirEnt_t*)rd_pdata;

	for(j_e = 0; j_e < (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t)); j_e++){
	    if(pdir_in->inum < 0) continue;
	    if(strcmp(pdir_in->name, name) == 0){
		pdir_in->inum = -1; // unlink from p directory
		strcpy(pdir_in->name, "");
		found = 1;
		break;
	    }
	    pdir_in++;  // check next block
	}

    }

    // name must be there
    int dir_block = i_b - 1; // i_b counts extra 1 when break
    /* int rm_mark = j_e;  // the entry of the name */
    // check if this is the last one
    int last_one = 1; 
    MFS_DirEnt_t* rd_entry = (MFS_DirEnt_t*)rd_pdata;
    for(i = 0; i < (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t)); i++){
	if(rd_entry->inum != -1){
	    last_one = 0;
	    break;
	}
	rd_entry++;
    }

    // update p inode accordingly
    if(last_one){
	// unlink the block from p directory
	pinode_in.ptr[dir_block] = -1;
	pinode_in.size -= MFS_BLOCK_SIZE;
	// update pimap and inode_box
	MFS_Imap_t p_imap;
	int gt = get_imap(fd, pinum, &p_imap);
	if(gt < 0){printf("img_unlink: get pimap in if fail\n"); return -1;}
	inode_box[pinum] = p_imap.imap[pinum%MFS_NODE_PER_MAP] = CR.log_end;

	// and don't write back data , write pinode, pimapm CR
	lseek(fd, 0, SEEK_END);
	int out = write(fd, &pinode_in, sizeof(MFS_Inode_t));
	if(out < 0){printf("img_unlink: WB pinode in if, fail\n"); return -1;}
	out = write(fd, &p_imap, sizeof(MFS_Imap_t));
	if(out < 0){printf("img_unlink: WB pimap in if, fail\n"); return -1;}

	CR.log_end += sizeof(MFS_Inode_t) + sizeof(MFS_Imap_t);
	CR.imap_ptr[pinum/MFS_NODE_PER_MAP] = CR.log_end - sizeof(MFS_Imap_t);
    }
    else{
	// write back the updated data
	pinode_in.ptr[dir_block] = CR.log_end;

	// update pimap and inode_box
	MFS_Imap_t p_imap;
	int gt = get_imap(fd, pinum, &p_imap);
	if(gt < 0){printf("img_unlink: get pimap in else fail\n"); return -1;}
	inode_box[pinum] = p_imap.imap[pinum%MFS_NODE_PER_MAP] = CR.log_end + MFS_BLOCK_SIZE;

	// write back data , pinode, pimap CR
	lseek(fd, 0, SEEK_END);
	int out = write(fd, rd_pdata, MFS_BLOCK_SIZE);
	if(out < 0){printf("img_unlink: WB data in else, fail\n"); return -1;}
	out = write(fd, &pinode_in, sizeof(MFS_Inode_t));
	if(out < 0){printf("img_unlink: WB pinode in else, fail\n"); return -1;}
	out = write(fd, &p_imap, sizeof(MFS_Imap_t));
	if(out < 0){printf("img_unlink: WB pimap in else, fail\n"); return -1;}

	CR.log_end += MFS_BLOCK_SIZE + sizeof(MFS_Inode_t) + sizeof(MFS_Imap_t);
	CR.imap_ptr[pinum/MFS_NODE_PER_MAP] = CR.log_end - sizeof(MFS_Imap_t);
    }

    // write back CR
    lseek(fd, 0, SEEK_SET);
    out = write(fd, &CR, sizeof(MFS_CR_t));
    if(out < 0){printf("img_unlink: WB CR 2nd time, fail\n"); return -1;}

    syn = fsync(fd);
    if(syn < 0)
	return -1;
    else
	return 0;
}

// free block condition untested
int img_creat(int fd, int pinum, int type, char* name){
    if(type != MFS_DIRECTORY && type != MFS_REGULAR_FILE){
	printf("img_creat: type error\n");
	return -1;
    }

    if(sizeof(*name) > MFS_NAME_SIZE){
	printf("img_creat: name too long\n");
	return -1;
    }

    MFS_Stat_t pinode_stat;
    int s = img_stat(fd, pinum, &pinode_stat);
    if(s < 0){printf("img_creat: get pinode stat fail\n"); return -1;} // inode invalid

    if(pinode_stat.type != MFS_DIRECTORY){
	printf("img_creat: given inode is not a directory, can't creat file or dir under it\n");
	return -1;
    }

    int look = img_lookup(fd, pinum, name);
    if(look > 0){
	printf("name exists\n");
	return 0;
    }
    else{  // name doesnt exist
	MFS_Inode_t pinode_in;
	int gt = get_inode(fd, pinum, &pinode_in);
	if(gt < 0){
	    printf("img_creat: get pinode fail\n");
	    return -1;
	}
	// looking for an empty space
	int i_b, j_e;
	int free_b = 0, free_e = 0;
	for(i_b = 0; i_b < MFS_DIRECT_P && !free_e && !free_b; i_b++){
	    if(pinode_in.ptr[i_b] ==-1){
		free_b = 1;
		break;
	    }
	    int pdir_offset = pinode_in.ptr[i_b];
	    lseek(fd, pdir_offset, SEEK_SET);
	    /* int e_offset = 0; */
	    for(j_e = 0; j_e < (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t)); j_e++){
		MFS_DirEnt_t p_entry;
		int in = read(fd, &p_entry, sizeof(MFS_DirEnt_t));
		if(in < 0){
		    printf("img_creat: when looking for empty entry, read dir_in error\n");
		    return -1;
		}
		if(p_entry.inum == -1){
		    free_e = 1;
		    break;
		}
	    }
	}

	// after the loop, there are shoud be free_e, free_b or full
	if(!free_b && !free_e){
	    printf("img_creat:no space, create fail\n");
	    return -1;
	}

	// dont forget to update inode_box after finish
       	int new_inum = inode_alloc();  // used to update the new dir entry
	char new_pdata[MFS_BLOCK_SIZE];

	// update the directory
	if(free_b){  // prepare a new block
	    // i_b is the empty # block in pinode_in
	    MFS_DirEnt_t* wr_entry = (MFS_DirEnt_t*)new_pdata;
	    strcpy(wr_entry->name, name);
	    wr_entry->inum = new_inum;
	    int i = 0;
	    do{
		wr_entry++; 
		strcpy(wr_entry->name, "");
		wr_entry->inum = -1;
		i++;
	    }while(i < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t) -1);
	    pinode_in.size += MFS_BLOCK_SIZE;  // need to update size
	    pinode_in.ptr[i_b] = CR.log_end;

	}

	if(free_e){ // get the old directory block and add an entry
	    int olddir_offset = pinode_in.ptr[i_b - 1]; // coz, j loop break, i_b increment once
	    lseek(fd, olddir_offset, SEEK_SET);
	    int in = read(fd, &new_pdata, MFS_BLOCK_SIZE);
	    if(in < 0){printf("img_creat: read from old direcotry error\n"); return -1;}
	    
	    // write the new entry
	    MFS_DirEnt_t* wr_entry = (MFS_DirEnt_t*)new_pdata;
	    /* int i; */
	    wr_entry = wr_entry + j_e;
	    strcpy(wr_entry->name, name);
	    wr_entry->inum = new_inum;

	    // update pinode_in
	    pinode_in.ptr[i_b - 1] = CR.log_end;
	}

	//update pimap and the inode_box, which all point to one updated inode
	MFS_Imap_t new_pimap;
	int gt_map = get_imap(fd, pinum, &new_pimap);
	if(gt_map < 0){printf("img_creat: get pimap fail\n"); return -1;}
	inode_box[pinum] = new_pimap.imap[pinum%16] = CR.log_end + MFS_BLOCK_SIZE;

	// update pimap in the CR
	CR.imap_ptr[pinum/MFS_NODE_PER_MAP] = inode_box[pinum] + sizeof(MFS_Inode_t);
	CR.log_end = CR.imap_ptr[pinum/MFS_NODE_PER_MAP] + sizeof(MFS_Imap_t);

	// write back parent part
	lseek(fd, 0, SEEK_END);
	int out = write(fd, &new_pdata, MFS_BLOCK_SIZE);
	if(out < 0){printf("img_creat: write back pdata error\n"); return -1;}
	out = write(fd, &pinode_in, sizeof(MFS_Inode_t));
	if(out < 0){printf("img_creat: write back pinode error\n"); return -1;}
	out = write(fd, &new_pimap, sizeof(MFS_Imap_t));
	if(out < 0){printf("img_creat: write back pimap error\n"); return -1;}
	// write back CR
	lseek(fd, 0, SEEK_SET);
	out = write(fd, &CR, sizeof(MFS_CR_t));
	if(out < 0){printf("img_creat: write back CR error\n"); return -1;}

	int syn = fsync(fd);
	if(syn < 0)
	    return -1;

	// prepare the date of the new inode
	char new_data[MFS_BLOCK_SIZE];
	MFS_Inode_t new_inode;
	MFS_Imap_t new_imap;
	if(type == MFS_DIRECTORY){
	    MFS_DirEnt_t* wr_entry = (MFS_DirEnt_t*)new_data;
	    strcpy(wr_entry->name, ".");  // prepare "." dir
	    wr_entry->inum = new_inum;
	    wr_entry++;
	    strcpy(wr_entry->name, ".."); // prepare ".." dir
	    wr_entry->inum = pinum;

	    int i = 0;
	    do{
		wr_entry++;
		strcpy(wr_entry->name, "");
		wr_entry->inum = -1;
		i++;
	    }while(i < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t) -2);
	    
	    // prepare inode 
	    new_inode.size = MFS_BLOCK_SIZE;
	    new_inode.type = MFS_DIRECTORY;
	    for(i = 0; i < MFS_NODE_PER_MAP; i++)
		new_inode.ptr[i] = -1;
	    new_inode.ptr[0] = CR.log_end;  // first block of the new inode

	    // prepare imap
	    int gt_imap = get_imap(fd, new_inum, &new_imap);
	    if(gt_imap < 0){
		// get fail
		printf("img_creat: get imap of the new inode fail\n");
		return -1;
	    }
	    else{
		int n_set = new_inum%16;
		inode_box[new_inum] = new_imap.imap[n_set] = CR.log_end + MFS_BLOCK_SIZE;
		}

	}

	if(type == MFS_REGULAR_FILE){
	    // prepare inode
	    new_inode.size = 0;
	    new_inode.type = MFS_REGULAR_FILE;
	    int i;
	    for(i = 0; i < MFS_NODE_PER_MAP; i++)
		new_inode.ptr[i] = -1;

	    // prepare imap
	    int gt_imap = get_imap(fd, new_inum, &new_imap);
	    if(gt_imap < 0){
		// get fail
		printf("img_creat: get imap of the new inode fail\n");
		return -1;
	    }
	    else{
		int n_set = new_inum%16;
		inode_box[new_inum] = new_imap.imap[n_set] = CR.log_end;
	    }
	}

	// update CR
	CR.imap_ptr[new_inum/MFS_NODE_PER_MAP] = inode_box[new_inum] + sizeof(MFS_Inode_t);
	CR.log_end = CR.imap_ptr[new_inum/MFS_NODE_PER_MAP] + sizeof(MFS_Imap_t);


	// write back new data, new inode and new imap, finally, CR
	lseek(fd, 0, SEEK_END);
	if(type == MFS_DIRECTORY){
	    out = write(fd, &new_data, MFS_BLOCK_SIZE);
	    if(out < 0){printf("img_creat: write back data error\n"); return -1;}
	}  // when type is file, doesnt write data

	out = write(fd, &new_inode, sizeof(MFS_Inode_t));
	if(out < 0){printf("img_creat: write back inode error\n"); return -1;}
	out = write(fd, &new_imap, sizeof(MFS_Imap_t));
	if(out < 0){printf("img_creat: write back imap error\n"); return -1;}

	// write back CR
	lseek(fd, 0, SEEK_SET);
	out = write(fd, &CR, sizeof(MFS_CR_t));
	if(out < 0){printf("img_creat: write back CR error\n"); return -1;}


	syn = fsync(fd);
	if(syn < 0)
	    return -1;
	else
	    return 0;
    }
	printf("img_creat: unknown error\n"); // shouldnt be here
	return -1;
}

// read a block from inode_in
/* int img_read(int fd, MFS_Inode_t* inode_in, char* buffer, int block){ */
/*     if(block < 0 || block >= MFS_DIRECT_P){ */
/* 	printf("img_read: block illegal\n"); */
/* 	return -1; */
/*     } */

/*     if(inode_in->ptr[block] == -1){ */
/* 	printf("img_read: block %d empty\n", block); */
/* 	return -1; */
/*     } */

/*     // read file */
/*     int file_offset = inode_in->ptr[block]; */
/*     lseek(fd, file_offset, SEEK_SET); */
/*     int out = read(fd, buffer, sizeof(MFS_BLOCK_SIZE)); */
/*     if(out < 0){printf("img_read: type-file, read error\n"); return -1;} */

/*     return 0; */


/* } */
int img_read(int fd, int inum, char*buffer, int block){
    if(block < 0 || block > MFS_DIRECT_P){
	printf("img_read: block illegal\n");
	return -1;
    }

    if(sizeof(*buffer) > MFS_BLOCK_SIZE){
	printf("img_read: buffer too big\n");
	return -1;
    }


    MFS_Stat_t inode_stat;
    int s = img_stat(fd, inum, &inode_stat);
    if(s < 0){
	printf("img_read: get stat fail or inum illegal\n");
	return -1;
    }
    if(inode_stat.type == MFS_DIRECTORY){
	// just return stat
	MFS_Inode_t inode_in;
	int gt = get_inode(fd, inum, &inode_in);
	if(gt < 0){
	    printf("img_read:type-dir, get inode fail\n");
	    return -1;
	}

	if(inode_in.ptr[block] == -1){
	    printf("img_read: type-dir, block empty\n");
	    return -1;
	}
	MFS_Stat_t* wrstat = (MFS_Stat_t*)buffer;
	wrstat->type = inode_stat.type;
	wrstat->size = inode_stat.size;
	return 0;
    }

    if(inode_stat.type == MFS_REGULAR_FILE){
	MFS_Inode_t inode_in;
	int gt = get_inode(fd, inum, &inode_in);
	if(gt < 0){
	    printf("img_read:type-file, get inode fail\n");
	    return -1;
	}

	if(inode_in.ptr[block] == -1){
	    printf("img_read: type-file, block empty\n");
	    return -1;
	}
	int file_offset = inode_in.ptr[block];
	// read file
	lseek(fd, file_offset, SEEK_SET);
	int out = read(fd, &buffer, sizeof(MFS_BLOCK_SIZE));
	if(out < 0){printf("img_read: type-file, read error\n"); return -1;}
	return 0;
    }


    printf("unknow error\n");
    return -1;

}



int img_write(int fd, int inum, char* buffer, int block){
    if(block < 0 || block >= MFS_DIRECT_P){
	printf("img_write: block illegal\n");
	return -1;
    }

    MFS_Inode_t inode_in;
    int gt = get_inode(fd, inum, &inode_in);
    if(gt < 0){
	printf("img_write: get inode fail\n");
	return -1;
    }

    if(inode_in.type != MFS_REGULAR_FILE){
	printf("img_write: type error\n");
	return -1;
    }

    if(inode_in.ptr[block] == -1){
	// new data block, update size
	inode_in.size += MFS_BLOCK_SIZE;
    }

    inode_in.ptr[block] = CR.log_end;  // new block location

    MFS_Imap_t imap_in; // get and update old imap
    int imap_offset = CR.imap_ptr[inum/MFS_NODE_PER_MAP];
    lseek(fd, imap_offset, SEEK_SET);
    int in = read(fd, &imap_in, sizeof(MFS_Imap_t));
    if(in < 0){printf("img_write: read imap fail\n"); return -1;}
    // update the imap and inode_box
    inode_box[inum] = imap_in.imap[inum%MFS_NODE_PER_MAP] = CR.log_end + MFS_BLOCK_SIZE;

    // update CR
    int piece_num = inum/MFS_NODE_PER_MAP;
    CR.imap_ptr[piece_num] = CR.log_end + MFS_BLOCK_SIZE + sizeof(MFS_Inode_t);
    CR.log_end = CR.imap_ptr[piece_num] + sizeof(MFS_Imap_t);

    // write back
    lseek(fd, 0, SEEK_END);
    int out = write(fd, buffer, sizeof(MFS_BLOCK_SIZE));
    if(out < 0){ printf("img_write: write buffer error\n"); return -1;}
    out = write(fd, &inode_in, sizeof(MFS_Inode_t));
    if(out < 0){ printf("img_write: write inode error\n"); return -1;}
    out = write(fd, &imap_in, sizeof(MFS_Imap_t));
    if(out < 0){ printf("img_write: write imap error\n"); return -1;}
    
    // write back CR
    lseek(fd, 0,SEEK_SET);
    out = write(fd, &CR, sizeof(MFS_CR_t));
    if(out < 0){ printf("img_write: write CR error\n"); return -1;}


    int syn = fsync(fd);
    if(syn < 0)
	return -1;
    else
	return 0;
}


int img_stat(int fd, int inum, MFS_Stat_t* m){
    // inum check is in the get_inode()
  MFS_Inode_t inode_in;
  int get = get_inode(fd, inum, &inode_in);
  if( get == -1){
    printf("img_stat: get inode fail\n");
    return -1;
  }
    m->type = inode_in.type;
    m->size = inode_in.size;
    return 0;
}

int img_lookup(int fd, int pinum, char* name){
    MFS_Stat_t stat;
    int s = img_stat(fd, pinum, &stat);
    if(s < 0){printf("img_lookup: stat fail\n"); return -1;}
 
    // if it's not -1, get inode successfully
    if(stat.type != MFS_DIRECTORY){
	printf("img_lookup: error, it's not directory\n");
	return -1;
    }

    MFS_Inode_t pinode_in;
    int gt = get_inode(fd, pinum, &pinode_in);
    if( gt == -1){
	printf("img_lookup: get inode stat fail\n");
	return -1;
    }
    // get inode succeed
    int i, j;
    for(i = 0; i < MFS_DIRECT_P; i++){
	int dir_offset = pinode_in.ptr[i];
	if(dir_offset < 0){
	    continue;
	}

	lseek(fd, dir_offset, SEEK_SET); // go to that block
	MFS_DirEnt_t dir_in;
	for(j = 0; j < (MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t)); j++){
	    int in = read(fd, &dir_in, sizeof(MFS_DirEnt_t));
	    if(in < 0){
		printf("img_lookup: read dir_in error\n");
		return -1;
	    }
	    if(dir_in.inum < 0) continue;
	    if(strcmp(dir_in.name, name) == 0){
		printf("lookup: %d\n",dir_in.inum);
		return dir_in.inum;
		}
	}
    }

    printf("not found name in given pinum\n");
    return -1;
}

int img_mount(char* fimage){
    printf("mounting\n");
    int fd = open(fimage, O_RDWR, S_IRWXU);

    if(fd >0){
	lseek(fd, 0, SEEK_SET);
	// caching the checkpoint region
        int in = read(fd, &CR, sizeof(CR));
	if(in < 0){
	  printf("img_mount: read CR error\n");
	    return -1;
	}
	printf("the read value is %d\n", CR.log_end);

	int i;
	for(i = 0; i <MFS_INUM; i++)
	    inode_box[i] = -1;

	// init inode box and load all the inode into inode_box
	MFS_Imap_t* imap_in = (MFS_Imap_t*)inode_box;
	for(i = 0; i < MFS_IMAPNUM; i++){
	  int map_offset = CR.imap_ptr[i];
	  if(map_offset == -1){
	      imap_in ++;
	      continue;
	  }
	  lseek(fd, map_offset, SEEK_SET);
	  int in = read(fd, imap_in, sizeof(*imap_in));
	  if(in < 0){printf("img_mount: read imap fail\n"); return -1;}

	}

	return fd;
    }
    else{
	return -1;
    }

}

int img_init(char* fimage){
  int fd = open(fimage, O_RDWR | O_CREAT, S_IRWXU);
  if(fd > 0){
    printf("img_init: open successfully, fd %d now it can be operated \n", fd);
    //prepare structures
    MFS_Inode_t i_root;
    MFS_Imap_t imap_root;

    // initialize checkpoint region.
    int log_end = sizeof(MFS_CR_t) + MFS_BLOCK_SIZE + sizeof(MFS_Inode_t) + sizeof(MFS_Imap_t);
    int offset = log_end - sizeof(imap_root); // 0th imap piece offset
    // update CR
    CR.log_end = log_end;
    int i;
    for(i = 0; i < MFS_IMAPNUM; i++)
	CR.imap_ptr[i] = -1;
    CR.imap_ptr[0] = offset;

    // initialize dir data block
    char new_data[MFS_BLOCK_SIZE];
    MFS_DirEnt_t* wr_entry = (MFS_DirEnt_t*) new_data;
    strcpy(wr_entry->name, ".");  // prepare "." dir
    wr_entry->inum = 0;
    wr_entry++;
    strcpy(wr_entry->name, ".."); // prepare ".." dir
    wr_entry->inum = 0;

    for(i = 0; i < MFS_BLOCK_SIZE/sizeof(MFS_DirEnt_t) -2; i++){
	wr_entry++;
	strcpy(wr_entry->name, "");
	wr_entry->inum = -1;
    }

    // initialize inode
    i_root.size = MFS_BLOCK_SIZE;
    i_root.type = MFS_DIRECTORY;
    for(i = 1; i < MFS_DIRECT_P; i++)
      i_root.ptr[i] = -1;
    i_root.ptr[0] = sizeof(MFS_CR_t); // the root directory start address

    // initialize imap
    for(i = 0; i < MFS_NODE_PER_MAP; i++)
      imap_root.imap[i] = -1;
    imap_root.imap[0] = sizeof(MFS_CR_t) + MFS_BLOCK_SIZE;

    // init and update inode_box
    for(i = 0; i < MFS_INUM; i++)
	inode_box[i] = -1;
    inode_box[0] = imap_root.imap[0];

    // write to the image, CR|DIR|INODE|IMAP
    lseek(fd, 0, SEEK_SET);
    int out = write(fd, &CR, sizeof(MFS_CR_t));
    if(out < 0){printf("init: CR fail\n"); return -1;}

    out = write(fd, &new_data,MFS_BLOCK_SIZE);
    if(out < 0){printf("init:new dir fail\n");return -1;}

    out = write(fd, &i_root, sizeof(i_root));
    if(out < 0){printf("init: root inode fail\n");return -1;}

    out = write(fd, &imap_root, sizeof(imap_root));
    if(out < 0){printf("init: root imap fail\n");return -1;}
    //    fsync(fd);
    int syn = fsync(fd);
    if(syn < 0)
	return -1;

    return fd;
  }
  else{
    fprintf(stderr, "%s: open error\n", fimage);
    return -1;
  }
}

/********************* helpers **********************/
int inode_alloc(){
    int i = 0;
    while(inode_box[i] != -1){
	i++;
    }
    if(i > MFS_INUM) // no inode available
	return -1;
    return i;
}

int get_inode(int fd, int inum, MFS_Inode_t* inode){
    if(inum < 0 || inum > MFS_INUM){
	printf("get_inode: illegal inode number\n");
	return -1;
    }
	
    int offset = inode_box[inum];
    if(offset < 0){
	printf("get_inode: inode dosent exist\n");
	return -1;
    }

    // read the inode	
    lseek(fd, offset, SEEK_SET);
    int in = read(fd, inode, sizeof(MFS_Inode_t));
    if(in < 0){
	printf("get_inode: read inode error\n");
	return -1;
    }
	return 0;
}

int get_imap(int fd, int inum, MFS_Imap_t* imap){
    if(inum < 0 || inum > MFS_INUM){
	printf("get_inode: illegal inode number\n");
	return -1;
    }
    int offset = CR.imap_ptr[inum/16];
    if(offset == -1){
	printf("get_imap: offset -1, no imap point to such inode number\n");
	return -1;
    }

    lseek(fd, offset, SEEK_SET);
    int in = read(fd, imap, sizeof(MFS_Imap_t));
    if(in < 0){
	printf("get_imap: read imap error\n");
	return -1;
    }
    return 0;

}


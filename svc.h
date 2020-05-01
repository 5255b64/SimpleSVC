#ifndef svc_h
#define svc_h

#include <stdlib.h>

typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char *file_name;
    char *resolved_file;
} resolution;

enum CHANGE_TYPE {  // change信息的类型 如果未指定 则为UNCHANGED
    UNCHANGED = 1, ADDITION, DELETION, MODIFICATION
};

// 存储文件信息(链表存储)
typedef struct file_struct {
    struct file_struct *next;
    enum CHANGE_TYPE tpye;  //
    char* file_name;        // 文件名
    char* file_content;     // 文件内容
    int file_hash;          // 文件hash
} file_struct;

// 存储修改信息（从属于index_struct）
typedef struct change_struct {
    char* file_name;        // 文件名
    enum CHANGE_TYPE change_type;
} change_struct;

// 保存与commit和checkout有关的文件信息
typedef struct index_struct {
    struct file_struct **file_struct;   // 指向文件信息
    struct change_struct **file_message;  // （存储）change信息
    int file_num;
} index_struct;

// commit 只在init和svc_commit时作修改
typedef struct commit {
//    struct commit *parent;      // 父节点
    struct commit **child_list; // （指向）可能有多个子节点
    int child_num;
    char *commit_id;
    char *message;
    struct index_struct *index;  // （存储）commit的index
} commit;

// 根据工作区变化进行实时修改
typedef struct checkout {
    struct checkout *next;        // 指向下一个checkout
    char *branch_name;
    struct commit *commit_ptr;
    struct index_struct *index;  // （存储）工作区的index
} checkout;

// 保存所有数据
typedef struct helper_struct {
    // 指向工作区
    struct checkout *head_checkout;

    // 指向第一个commit
    struct commit *init_commit;
    int commit_num;

    // 指向第一个checkout 所有checkout为单链结构
    struct checkout *init_checkout;
    int checkout_num;

    // 指向第一个file 所有file为单链结构
    struct file_struct *init_file;
    int file_num;
} helper_struct;


void *svc_init(void);

void cleanup(void *helper);

int hash_file(void *helper, char *file_path);

char *svc_commit(void *helper, char *message);

void *get_commit(void *helper, char *commit_id);

char **get_prev_commits(void *helper, void *commit, int *n_prev);

void print_commit(void *helper, char *commit_id);

int svc_branch(void *helper, char *branch_name);

int svc_checkout(void *helper, char *branch_name);

char **list_branches(void *helper, int *n_branches);

int svc_add(void *helper, char *file_name);

int svc_rm(void *helper, char *file_name);

int svc_reset(void *helper, char *commit_id);

char *svc_merge(void *helper, char *branch_name, resolution *resolutions, int n_resolutions);

#endif


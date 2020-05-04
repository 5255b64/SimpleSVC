#ifndef svc_h
#define svc_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef struct resolution {
    // NOTE: DO NOT MODIFY THIS STRUCT
    char *file_name;
    char *resolved_file;
} resolution;

typedef enum CHANGE_TYPE {  // change信息的类型 如果未指定 则为UNCHANGED
    UNCHANGED = 1, ADDITION, DELETION, MODIFICATION
} change_type;

// 存储文件信息(链表存储)
typedef struct file_struct {
    struct file_struct *next;
    char *file_name;        // 文件名
    char *file_content;     // 文件内容
    int file_hash;          // 文件hash
} file_struct;

// 引用file_struct的信息(链表存储)
typedef struct file_list_struct {
    struct file_list_struct *next;
    struct file_struct *ptr;        // （指向）
} file_list_struct;

// 存储修改信息（从属于index_struct）
typedef struct change_struct {
    char *file_name;        // 文件名
    enum CHANGE_TYPE change_type;
    int file_hash;          // 文件hash
    int file_hash_pre;      // 上一个commit中 该文件的hash
} change_struct;

// 引用change_struct的信息(链表存储)
typedef struct change_list_struct {
    struct change_list_struct *next;
    struct change_struct *ptr;          // （存储）
} change_list_struct;

// 保存与commit和checkout有关的文件信息
typedef struct file_track_struct {
    struct file_list_struct *file_list;     // （存储）工作区文件列表
    struct change_list_struct *change_list;  // （存储）change列表信息 在commit时做处理
    int file_num;
} file_track_struct;

// 引用file_struct的信息(链表存储)
typedef struct commit_link_list {
    struct commit_link_list *next;
    struct commit_struct *ptr;
} commit_link_list;

// commit_struct 只在init和svc_commit时作修改
typedef struct commit_struct {
//    struct commit_struct *parent;      // 父节点
    struct commit_link_list *parent_list; // （存储）可能有多个父节点
    int parent_num;
    struct commit_link_list *child_list; // （指向）可能有多个子节点
    int child_num;
    char *commit_id;
    char *message;
    struct file_track_struct *file_track;  // （存储）commit的index
    char *branch_name;
} commit_struct;

// 根据工作区变化进行实时修改
typedef struct checkout_struct {
    struct checkout_struct *next;        // 指向下一个checkout
    char *branch_name;
    struct commit_struct *commit_pre;      // 当前checkout工作区的前一个commit
    struct file_track_struct *file_track;  // （存储）工作区的index
} checkout_struct;

// 保存所有数据
typedef struct helper_struct {
    // 指向工作区
    struct checkout_struct *head_checkout;

    // 指向第一个commit
    struct commit_struct *init_commit;
    int commit_num;

    // 指向第一个checkout 所有checkout为单链结构
    struct checkout_struct *init_checkout;
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


// 构造方法
checkout_struct *construct_checkout(char *branch_name, commit_struct *commit_pre);

change_list_struct *construct_change_list(change_struct *cg);

change_struct *construct_change(char *file_name);

file_list_struct *construct_file_list(file_struct *file);

file_track_struct *construct_file_track();

commit_link_list *construct_commit_link_list(commit_struct *c);

commit_struct *construct_commit(char *message);

helper_struct *construct_helper();

file_struct *construct_file(char *file_name);

file_struct *deepcopy_file(file_struct *file);

file_track_struct *deepcopy_file_track(file_track_struct *ft);

file_list_struct *deepcopy_file_list(file_list_struct *old_fl);

change_list_struct *deepcopy_change_list(change_list_struct *old_cl);

change_struct *deepcopy_change(change_struct *old_cg);

// 析构方法
void free_checkout(checkout_struct *co);

void free_change_list(change_list_struct *cl);

void free_file_list(file_list_struct *fl);

void free_file_list_single(file_list_struct *fl);

void free_file_track(file_track_struct *ft);

void free_commit_link_child_list(commit_link_list *ccl);

void free_commit_link_parent_list(commit_link_list *ccl);

void free_commit(commit_struct *c);

void free_change(change_struct *cg);

void free_file(file_struct *f);

// 其它方法
void commit_add_commit(commit_struct **c_parent_output, commit_struct **c_child_output);

void commit_link_list_add_commit(commit_link_list **ccl_output, commit_struct *c);

int file_list_add_file(file_list_struct **fl_output, file_struct *file);

void set_all_change(change_list_struct **change_list_output, change_type type);

void copy_file_2_change(change_list_struct **change_list_output, file_list_struct *file_list);

void helper_add_commit(helper_struct **helper_output, commit_struct *new_commit);

int helper_add_file(helper_struct **helper_output, file_struct *new_file);

void helper_add_checkout(helper_struct **helper_output, checkout_struct *new_checkout);

void check_disk_file(file_track_struct **file_track_in_out, helper_struct *h);

void new_change_list_from_file_list_compare(change_list_struct **cl_output, file_list_struct *fl_old,
                                            file_list_struct *fl_new);

void file_list_sort(file_list_struct **fl_output);

void change_list_struct_add_change(change_list_struct **cl_output, change_struct *cg);

int checkout_add_file(helper_struct **h_output, file_struct *file);

commit_struct *helper_find_commit(helper_struct *helper, char *commit_id);

char *int2hash(int id);

int check_not_commit_change(helper_struct *helper);

int diy_strcmp(char *str1, char *str2);

void commit_write_all_file(commit_struct *commit);

commit_struct *get_commit_from_child(commit_link_list *child_list, char *commit_id);

commit_struct *get_commit_recurisive(commit_struct *commit, char *commit_id);

file_struct *find_file(helper_struct *helper, int file_hash);

file_struct *disk_read_file(char *file_name);

char *read_file(char *path);

char *deepcopy_str(char *c_input);

int file_list_get_num(file_list_struct *fl);
#endif


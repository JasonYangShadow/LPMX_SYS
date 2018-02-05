#include "hashmap.h"
#include "log.h"

pthread_mutex_t GLOBAL_MAP_LOCK = PTHREAD_MUTEX_INITIALIZER;

void init_map(hmap_t* pmap){
    if(pmap == NULL){
        pthread_mutex_lock(&GLOBAL_MAP_LOCK);
        if(pmap == NULL){
            pmap = create_hmap(TOP_HMAP_SIZE);
        }
        pthread_mutex_unlock(&GLOBAL_MAP_LOCK);
    }
}
    
void add_item_list(const char* key, struct list_head* phead){
    struct list_item * item = (struct list_item*)malloc(sizeof(struct list_item));
    strcpy(item->keys, key);
    LIST_INSERT_HEAD(&phead->head,item,pointers);
}

void delete_item_list(const char* key, struct list_head* phead){
    if(!LIST_EMPTY(&phead->head)){
        struct list_item* pitem;
        LIST_FOREACH(pitem, &phead->head, pointers){
            if(strcmp(pitem->keys, key) == 0 ){
                LIST_REMOVE(pitem,pointers);
                free(pitem);
            }
        }
    }
}

bool find_item_list(const char* key, struct list_head* phead){
    if(!LIST_EMPTY(&phead->head)){
        struct list_item* pitem;
        LIST_FOREACH(pitem, &phead->head, pointers){
            if(strcmp(pitem->keys, key) == 0){
                return true;
            }else{
                return false;
            }
        }
    }
    return false;
}

void clear_item_list(struct list_head* phead){
    while(!LIST_EMPTY(&phead->head)){
        struct list_item* pitem = LIST_FIRST(&phead->head);
        LIST_REMOVE(pitem, pointers);
        free(pitem);
    }
}

hmap_t* create_hmap(size_t h_max_size){
    hmap_t* pmap = (hmap_t*)malloc(sizeof(hmap_t));
    pmap->h_max_size = h_max_size;
    hcreate_r(h_max_size,&pmap->h_map);
    LIST_INIT(&pmap->head.head);
    return pmap;
}

void destroy_hmap(hmap_t* pmap){
    hdestroy_r(&pmap->h_map);
    clear_item_list(&pmap->head);
    free(pmap);
    pmap = NULL;
}

int add_item_hmap(hmap_t* pmap, char* key, void* data){
    unsigned n = 0;
    ENTRY e, *ep;
    e.key = key;
    e.data = data;
    n = hsearch_r(e, ENTER, &ep, &pmap->h_map);
    if(n){
        add_item_list(key,&pmap->head);
    }
    return n;
}

void* get_item_hmap(hmap_t* pmap, char* key){
    ENTRY e, *ep;
    e.key = key;
    if(hsearch_r(e, FIND, &ep, &pmap->h_map)){
        return ep->data;
    }else{
        return NULL;
    }
}

void delete_item_hmap(hmap_t* pmap, char* key){
    ENTRY e, *ep;
    e.key = key;
    if(hsearch_r(e, FIND, &ep, &pmap->h_map)){
        delete_item_list(key,&pmap->head);
    }
}

bool contain_item_hmap(hmap_t* pmap, char* key){
    ENTRY e, *ep;
    e.key = key;
    if(hsearch_r(e,FIND,&ep,&pmap->h_map)){
        if(find_item_list(key, &pmap->head)){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

void update_complex_hmap(hmap_t* pmap,char* top_key, char* second_key, char* third_key, void* data){
    void* fdata = get_item_hmap(pmap, top_key);
    if(!fdata){
        hmap_t* second_hmap = create_hmap(SECOND_HMAP_SIZE);
        hmap_t* third_hmap = create_hmap(THIRD_HMAP_SIZE);
        add_item_hmap(third_hmap, third_key, data);
        add_item_hmap(second_hmap, second_key, (void*)third_hmap);
        add_item_hmap(pmap, top_key, (void*)second_hmap);
    }else{
        hmap_t* second_hmap = (hmap_t *)fdata;
        void* sdata = get_item_hmap(second_hmap, second_key);
        if(!sdata){
            hmap_t* third_hmap = create_hmap(THIRD_HMAP_SIZE);
            add_item_hmap(third_hmap, third_key, data);
            add_item_hmap(second_hmap, second_key, (void*)third_hmap);
        }else{
            hmap_t* third_hmap = (hmap_t*)sdata;
            add_item_hmap(third_hmap, second_key, data);
        }
    }
}

void* get_complex_hmap(hmap_t* pmap, char* top_key, char* second_key, char* third_key){
    void* fdata = get_item_hmap(pmap, top_key);
    if(fdata){
        hmap_t* shmap = (hmap_t*)fdata;
        void* sdata = get_item_hmap(shmap, second_key);
        if(sdata){
            hmap_t* thmap = (hmap_t*)sdata;
            void* data = get_item_hmap(thmap, third_key);
            if(data){
                return data;
            }
        }
    }
    return NULL;
}

/*int main(void)
{
    char top_key[64] = "lever1";
    char second_key[64] = "lever2";
    char third_key[64] = "lever3";
    char data[64] = "data";
    update_complex_hmap(top_key, second_key,third_key, data);

    printf("data:%s\n",(char*)get_complex_hmap(top_key,second_key,third_key));
    printf("contain top_key:%s,? %d\n",top_key,contain_item_hmap(GLOBAL_HMAP,top_key));
    printf("contain other top_key:%s,? %d\n",data,contain_item_hmap(GLOBAL_HMAP,data));
    return 0;
}*/

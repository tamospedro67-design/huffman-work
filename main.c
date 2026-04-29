
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "codigo.h"

typedef struct HuffNode {
    unsigned char ch;          
    U64 freq;                  
    struct HuffNode *left;     
    struct HuffNode *right;    
} HuffNode;

typedef struct {
    HuffNode **data;
    size_t size;
    size_t capacity;
} MinHeap;

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        exit(1);
    }
    return p;
}

static void *xrealloc(void *ptr, size_t n) {
    void *p = realloc(ptr, n);
    if (!p) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        exit(1);
    }
    return p;
}

static HuffNode *node_new(unsigned char ch, U64 freq, HuffNode *left, HuffNode *right) {
    HuffNode *n = (HuffNode *)xmalloc(sizeof(*n));
    n->ch = ch;
    n->freq = freq;
    n->left = left;
    n->right = right;
    return n;
}

static int node_is_leaf(const HuffNode *n) {
    return n && !n->left && !n->right;
}

static void heap_init(MinHeap *h, size_t capacity) {
    h->size = 0;
    h->capacity = capacity ? capacity : 16;
    h->data = (HuffNode **)xmalloc(h->capacity * sizeof(h->data[0]));
}

static void heap_free(MinHeap *h) {
    free(h->data);
    h->data = NULL;
    h->size = 0;
    h->capacity = 0;
}

static void heap_swap(HuffNode **a, HuffNode **b) {
    HuffNode *t = *a;
    *a = *b;
    *b = t;
}

static int heap_less(const HuffNode *a, const HuffNode *b) {
    if (a->freq != b->freq) return a->freq < b->freq;
    /* Desempate estável: folhas antes de internos; e por caractere */
    if (node_is_leaf(a) != node_is_leaf(b)) return node_is_leaf(a) > node_is_leaf(b);
    return a->ch < b->ch;
}

static void heap_sift_up(MinHeap *h, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (!heap_less(h->data[idx], h->data[parent])) break;
        heap_swap(&h->data[idx], &h->data[parent]);
        idx = parent;
    }
}

static void heap_sift_down(MinHeap *h, size_t idx) {
    for (;;) {
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t smallest = idx;

        if (left < h->size && heap_less(h->data[left], h->data[smallest])) smallest = left;
        if (right < h->size && heap_less(h->data[right], h->data[smallest])) smallest = right;
        if (smallest == idx) break;

        heap_swap(&h->data[idx], &h->data[smallest]);
        idx = smallest;
    }
}

static void heap_push(MinHeap *h, HuffNode *n) {
    if (h->size == h->capacity) {
        h->capacity *= 2;
        h->data = (HuffNode **)xrealloc(h->data, h->capacity * sizeof(h->data[0]));
    }
    h->data[h->size] = n;
    heap_sift_up(h, h->size);
    h->size++;
}

static HuffNode *heap_pop(MinHeap *h) {
    if (h->size == 0) return NULL;
    HuffNode *min = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        heap_sift_down(h, 0);
    }
    return min;
}

static void tree_free(HuffNode *root) {
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}

static HuffNode *build_huffman_tree(const U64 freq[256]) {
    MinHeap h;
    heap_init(&h, 64);

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            heap_push(&h, node_new((unsigned char)i, freq[i], NULL, NULL));
        }
    }

    if (h.size == 0) {
        heap_free(&h);
        return NULL;
    }

    
    if (h.size == 1) {
        HuffNode *only = heap_pop(&h);
        HuffNode *parent = node_new(0, only->freq, only, NULL);
        heap_free(&h);
        return parent;
    }

    while (h.size > 1) {
        HuffNode *a = heap_pop(&h);
        HuffNode *b = heap_pop(&h);
        HuffNode *parent = node_new(0, a->freq + b->freq, a, b);
        heap_push(&h, parent);
    }

    HuffNode *root = heap_pop(&h);
    heap_free(&h);
    return root;
}

static void assign_codes_dfs(
    const HuffNode *node,
    char *path,
    size_t depth,
    char *codes[256]
) {
    if (!node) return;

    if (node_is_leaf(node)) {
        if (depth == 0) {
           
            path[0] = '0';
            path[1] = '\0';
        } else {
            path[depth] = '\0';
        }

        size_t len = strlen(path);
        codes[node->ch] = (char *)xmalloc(len + 1);
        memcpy(codes[node->ch], path, len + 1);
        return;
    }

    if (node->left) {
        path[depth] = '0';
        assign_codes_dfs(node->left, path, depth + 1, codes);
    }
    if (node->right) {
        path[depth] = '1';
        assign_codes_dfs(node->right, path, depth + 1, codes);
    }
}

static void free_codes(char *codes[256]) {
    for (int i = 0; i < 256; i++) {
        free(codes[i]);
        codes[i] = NULL;
    }
}

typedef struct {
    FILE *f;
    U8 cur;
    int nbits; 

typedef struct {
    FILE *f;
    int cur;       
    int nleft;      
} BitReader;

static void bw_init(BitWriter *bw, FILE *f) {
    bw->f = f;
    bw->cur = 0;
    bw->nbits = 0;
}

static int bw_write_bit(BitWriter *bw, U8 bit) {
    bw->cur = (U8)((bw->cur << 1) | (bit ? 1 : 0));
    bw->nbits++;
    if (bw->nbits == 8) {
        if (fputc((int)bw->cur, bw->f) == EOF) return 0;
        bw->cur = 0;
        bw->nbits = 0;
    }
    return 1;
}

static int bw_flush(BitWriter *bw) {
    if (bw->nbits == 0) return 1;
    bw->cur <<= (8 - bw->nbits); 
    if (fputc((int)bw->cur, bw->f) == EOF) return 0;
    bw->cur = 0;
    bw->nbits = 0;
    return 1;
}

static void br_init(BitReader *br, FILE *f) {
    br->f = f;
    br->cur = EOF;
    br->nleft = 0;
}


static int br_read_bit(BitReader *br, U8 *bit) {
    if (br->nleft == 0) {
        br->cur = fgetc(br->f);
        if (br->cur == EOF) return 0;
        br->nleft = 8;
    }
    br->nleft--;
    *bit = (U8)(((br->cur >> br->nleft) & 1) ? 1 : 0); 
    return 1;
}

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static int read_frequencies(FILE *in, U64 freq[256], U64 *original_size) {
    for (int i = 0; i < 256; i++) freq[i] = 0;
    *original_size = 0;

    int c;
    while ((c = fgetc(in)) != EOF) {
        freq[(U8)c]++;
        (*original_size)++;
    }
    return ferror(in) ? 0 : 1;
}

static int write_header(FILE *out, U64 original_size, const U64 freq[256]) {
    const unsigned char magic[4] = {'H','U','F','1'};
    if (fwrite(magic, 1, 4, out) != 4) return 0;
    if (fwrite(&original_size, sizeof(original_size), 1, out) != 1) return 0;
    if (fwrite(freq, sizeof(freq[0]), 256, out) != 256) return 0;
    return 1;
}

static int read_header(FILE *in, U64 *original_size, U64 freq[256]) {
    unsigned char magic[4];
    if (fread(magic, 1, 4, in) != 4) return 0;
    if (memcmp(magic, "HUF1", 4) != 0) return 0;
    if (fread(original_size, sizeof(*original_size), 1, in) != 1) return 0;
    if (fread(freq, sizeof(freq[0]), 256, in) != 256) return 0;
    return 1;
}

static int build_codes(const HuffNode *root, char *codes[256]) {
    for (int i = 0; i < 256; i++) codes[i] = NULL;
    char path[512];
    assign_codes_dfs(root, path, 0, codes);
    return 1;
}

static int compress_file(const char *in_path, const char *out_path) {
    FILE *in = fopen(in_path, "rb");
    if (!in) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s' (rb).\n", in_path);
        return 0;
    }

    U64 freq[256];
    U64 original_size;
    if (!read_frequencies(in, freq, &original_size)) {
        fprintf(stderr, "Erro: falha ao ler '%s'.\n", in_path);
        fclose(in);
        return 0;
    }

    HuffNode *root = build_huffman_tree(freq);
    if (!root) {
        
        FILE *out_empty = fopen(out_path, "wb");
        if (!out_empty) {
            fprintf(stderr, "Erro: nao foi possivel abrir '%s' (wb).\n", out_path);
            fclose(in);
            return 0;
        }
        if (!write_header(out_empty, 0, freq)) {
            fprintf(stderr, "Erro: falha ao escrever header.\n");
            fclose(out_empty);
            fclose(in);
            return 0;
        }
        fclose(out_empty);
        fclose(in);
        printf("Compactacao concluida (arquivo vazio).\n");
        return 1;
    }

    char *codes[256];
    build_codes(root, codes);

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s' (wb).\n", out_path);
        free_codes(codes);
        tree_free(root);
        fclose(in);
        return 0;
    }

    if (!write_header(out, original_size, freq)) {
        fprintf(stderr, "Erro: falha ao escrever header.\n");
        fclose(out);
        free_codes(codes);
        tree_free(root);
        fclose(in);
        return 0;
    }

    if (fseek(in, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: falha ao retornar ao inicio do arquivo.\n");
        fclose(out);
        free_codes(codes);
        tree_free(root);
        fclose(in);
        return 0;
    }

    BitWriter bw;
    bw_init(&bw, out);

    int c;
    while ((c = fgetc(in)) != EOF) {
        const char *code = codes[(U8)c];
        for (size_t j = 0; code[j] != '\0'; j++) {
            U8 bit = (code[j] == '1') ? (U8)1 : (U8)0;
            if (!bw_write_bit(&bw, bit)) {
                fprintf(stderr, "Erro: falha ao escrever bits no arquivo compactado.\n");
                fclose(out);
                free_codes(codes);
                tree_free(root);
                fclose(in);
                return 0;
            }
        }
    }
    if (ferror(in)) {
        fprintf(stderr, "Erro: falha ao ler bytes de '%s'.\n", in_path);
        fclose(out);
        free_codes(codes);
        tree_free(root);
        fclose(in);
        return 0;
    }

    if (!bw_flush(&bw)) {
        fprintf(stderr, "Erro: falha ao finalizar escrita.\n");
        fclose(out);
        free_codes(codes);
        tree_free(root);
        fclose(in);
        return 0;
    }

    fclose(out);
    free_codes(codes);
    tree_free(root);
    fclose(in);

    printf("Compactacao concluida: '%s' -> '%s'\n", in_path, out_path);
    return 1;
}

static int decompress_file(const char *in_path, const char *out_path) {
    FILE *in = fopen(in_path, "rb");
    if (!in) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s' (rb).\n", in_path);
        return 0;
    }

    U64 freq[256];
    U64 original_size;
    if (!read_header(in, &original_size, freq)) {
        fprintf(stderr, "Erro: arquivo compactado invalido (header/magic).\n");
        fclose(in);
        return 0;
    }

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s' (wb).\n", out_path);
        fclose(in);
        return 0;
    }

    if (original_size == 0) {
        fclose(out);
        fclose(in);
        printf("Descompactacao concluida (arquivo vazio).\n");
        return 1;
    }

    HuffNode *root = build_huffman_tree(freq);
    if (!root) {
        fprintf(stderr, "Erro: nao foi possivel reconstruir a arvore.\n");
        fclose(out);
        fclose(in);
        return 0;
    }

    
    if (root->left && node_is_leaf(root->left) && root->right == NULL) {
        for (U64 i = 0; i < original_size; i++) {
            if (fputc((int)root->left->ch, out) == EOF) {
                fprintf(stderr, "Erro: falha ao escrever arquivo de saida.\n");
                tree_free(root);
                fclose(out);
                fclose(in);
                return 0;
            }
        }
        tree_free(root);
        fclose(out);
        fclose(in);
        printf("Descompactacao concluida: '%s' -> '%s'\n", in_path, out_path);
        return 1;
    }

    BitReader br;
    br_init(&br, in);

    const HuffNode *cur = root;
    U64 produced = 0;
    while (produced < original_size) {
        U8 bit;
        if (!br_read_bit(&br, &bit)) {
            fprintf(stderr, "Erro: fim inesperado do payload.\n");
            tree_free(root);
            fclose(out);
            fclose(in);
            return 0;
        }

        if (bit == 0) {
            if (cur->left) cur = cur->left;
        } else {
            if (cur->right) cur = cur->right;
        }

        if (node_is_leaf(cur)) {
            if (fputc((int)cur->ch, out) == EOF) {
                fprintf(stderr, "Erro: falha ao escrever arquivo de saida.\n");
                tree_free(root);
                fclose(out);
                fclose(in);
                return 0;
            }
            produced++;
            cur = root;
        }
    }

    tree_free(root);
    fclose(out);
    fclose(in);
    printf("Descompactacao concluida: '%s' -> '%s'\n", in_path, out_path);
    return 1;
}

int main(void) {
    for (;;) {
        char line[32];
        printf("=== COMPACTADOR HUFFMAN ===\n");
        printf("1 - Compactar arquivo\n");
        printf("2 - Descompactar arquivo\n");
        printf("0 - Sair\n");
        printf("Escolha uma opcao: ");

        if (!fgets(line, (int)sizeof(line), stdin)) {
            printf("\n");
            return 0;
        }
        trim_newline(line);

        if (strcmp(line, "0") == 0) {
            return 0;
        } else if (strcmp(line, "1") == 0) {
            char in_path[1024];
            char out_path[1024];

            printf("Caminho do arquivo original: ");
            if (!fgets(in_path, (int)sizeof(in_path), stdin)) return 0;
            trim_newline(in_path);

            printf("Caminho do arquivo compactado (ex: saida.huff): ");
            if (!fgets(out_path, (int)sizeof(out_path), stdin)) return 0;
            trim_newline(out_path);

            (void)compress_file(in_path, out_path);
            printf("\n");
        } else if (strcmp(line, "2") == 0) {
            char in_path[1024];
            char out_path[1024];

            printf("Caminho do arquivo compactado (ex: entrada.huff): ");
            if (!fgets(in_path, (int)sizeof(in_path), stdin)) return 0;
            trim_newline(in_path);

            printf("Caminho do arquivo descompactado (saida): ");
            if (!fgets(out_path, (int)sizeof(out_path), stdin)) return 0;
            trim_newline(out_path);

            (void)decompress_file(in_path, out_path);
            printf("\n");
        } else {
            printf("Opcao invalida.\n\n");
        }
    }
}


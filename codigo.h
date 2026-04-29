#ifndef CODIGO
#define CODIGO

#include "meustiposonlinegdb.h"

typedef struct {
    U8 *byte;           /* vetor dinâmico de bytes */
    U64 capacidade;     /* em bits */
    U64 tamanho;        /* em bits */
} Codigo;

boolean novo_codigo(Codigo *c /* por referência */);
void free_codigo(Codigo *c /* por referência */);

boolean adiciona_bit(Codigo *c /* por referência */,
                     U8 valor /* 0 ou 1 */);
boolean joga_fora_bit(Codigo *c /* por referência */);
boolean pega_byte(Codigo c /* por valor */,
                  U8 qual /* por valor */,
                  U8 *b /* por referência */);

/* Bit índice 0 = primeiro bit da sequência (mesma ordem que toString). */
boolean pega_bit(Codigo c /* por valor */,
                 U64 indice /* 0 .. tamanho-1 */,
                 U8 *bit /* 0 ou 1 */);

char *toString(Codigo c /* por valor */);
boolean clone(Codigo original, /* por valor */
              Codigo *copia /* por referência */);

#endif

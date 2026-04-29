#include <stdlib.h>
#include "codigo.h"

boolean novo_codigo(Codigo *c /* por referência */)
{
    c->byte = (U8 *)malloc(1 * sizeof(U8));
    if (c->byte == NULL)
        return false;

    c->byte[0] = 0;
    c->capacidade = 8;
    c->tamanho = 0;
    return true;
}

void free_codigo(Codigo *c /* por referência */)
{
    if (c->byte != NULL)
        free(c->byte);
    c->byte = NULL;
}

boolean adiciona_bit(Codigo *c /* por referência */,
                     U8 valor /* 0 ou 1 */)
{
    if (c->tamanho == c->capacidade) {
        U8 *novo = (U8 *)malloc((c->capacidade / 8 + 1) * sizeof(U8));
        if (novo == NULL)
            return false;

        for (int i = 0; i < (int)(c->capacidade / 8); i++)
            novo[i] = c->byte[i];

        free(c->byte);
        c->byte = novo;

        c->capacidade += 8;
        c->byte[c->capacidade / 8 - 1] = 0;
    }

    c->byte[c->capacidade / 8 - 1] <<= 1;

    if (valor == 1)
        c->byte[c->capacidade / 8 - 1] |= 1;

    c->tamanho++;
    return true;
}

boolean joga_fora_bit(Codigo *c /* por referência */)
{
    if (c->tamanho == 0)
        return false;

    c->byte[c->capacidade / 8 - 1] >>= 1;
    c->tamanho--;

    if (c->capacidade > 8 && c->capacidade - c->tamanho == 8) {
        U8 *novo = (U8 *)malloc((c->capacidade / 8 - 1) * sizeof(U8));
        if (novo == NULL)
            return false;

        for (int i = 0; i < (int)(c->tamanho / 8); i++)
            novo[i] = c->byte[i];

        free(c->byte);
        c->byte = novo;

        c->capacidade -= 8;
    }

    return true;
}

boolean pega_byte(Codigo c /* por valor */,
                  U8 qual /* por valor */,
                  U8 *b /* por referência */)
{
    if (c.byte == NULL || c.capacidade == 0)
        return false;
    if (qual >= c.capacidade / 8)
        return false;

    *b = c.byte[qual];
    return true;
}

boolean pega_bit(Codigo c /* por valor */,
                 U64 indice,
                 U8 *bit)
{
    if (c.byte == NULL || indice >= c.tamanho || c.tamanho == 0)
        return false;

    int bits_ultimo_byte = (int)(c.tamanho % 8);
    if (bits_ultimo_byte == 0)
        bits_ultimo_byte = 8;

    U64 pos = 0;
    for (U64 b = 0; b < c.capacidade / 8; b++) {
        int bits_neste_byte = (b < c.capacidade / 8 - 1) ? 8 : bits_ultimo_byte;
        for (int i = bits_neste_byte - 1; i >= 0; i--) {
            if (pos == indice) {
                *bit = (U8)((c.byte[b] >> i) & 1);
                return true;
            }
            pos++;
        }
    }
    return false;
}

boolean clone(Codigo original, /* por valor */
              Codigo *copia /* por referência */)
{
    if (original.capacidade == 0 || original.byte == NULL)
        return false;

    copia->byte = (U8 *)malloc((size_t)(original.capacidade / 8) * sizeof(U8));
    if (copia->byte == NULL)
        return false;

    for (U64 i = 0; i < original.capacidade / 8; i++)
        copia->byte[i] = original.byte[i];

    copia->capacidade = original.capacidade;
    copia->tamanho = original.tamanho;
    return true;
}

char *toString(Codigo c)
{
    if (c.byte == NULL || c.tamanho == 0) {
        char *empty = (char *)malloc(1);
        if (empty == NULL)
            return NULL;
        empty[0] = '\0';
        return empty;
    }

    char *retorno = (char *)malloc((size_t)(c.tamanho + 1) * sizeof(char));
    if (retorno == NULL)
        return NULL;

    int bits_ultimo_byte = (int)(c.tamanho % 8);
    if (bits_ultimo_byte == 0)
        bits_ultimo_byte = 8;

    int pos = 0;
    for (U64 b = 0; b < c.capacidade / 8; b++) {
        int bits_neste_byte = (b < c.capacidade / 8 - 1) ? 8 : bits_ultimo_byte;
        for (int i = bits_neste_byte - 1; i >= 0; i--)
            retorno[pos++] = ((c.byte[b] >> i) & 1) ? '1' : '0';
    }

    retorno[c.tamanho] = '\0';
    return retorno;
}

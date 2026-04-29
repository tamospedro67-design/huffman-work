# Compactador Huffman (binário) em C

Este projeto contém um programa em C que implementa **compactação e descompactação Huffman para arquivos binários**:

- Lê arquivos em `rb` (qualquer tipo: `.txt`, `.pdf`, `.png`, `.jpg`, `.zip`, etc.)
- Calcula frequência de **bytes** (0..255)
- Constrói a árvore de Huffman com min-heap manual
- Escreve um arquivo compactado em `wb` com cabeçalho + payload de bits
- Descompacta lendo o cabeçalho e reconstruindo o arquivo original byte a byte

## Como compilar e executar (Windows / PowerShell)

```powershell
cd c:\src\estruturas-trabalho
gcc -std=c11 -Wall -Wextra -O2 main.c codigo.c -o huffman.exe
.\huffman.exe
```

## Exemplo de uso

Ao executar, o menu será:

```
=== COMPACTADOR HUFFMAN ===
1 - Compactar arquivo
2 - Descompactar arquivo
0 - Sair
Escolha uma opcao:
```

- Para compactar: escolha `1`, informe o caminho do arquivo original e o caminho de saída (ex: `saida.huff`)
- Para descompactar: escolha `2`, informe o caminho do `.huff` e o caminho do arquivo reconstruído (ex: `saida.pdf`)


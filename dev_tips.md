# Dev tips

Este arquivo é para ter informações mais detalhadas sobre
  como utilizar e modificar o código nos padrões estabelecidos pelo grupo.

## Git

Instale o git via [link](https://git-scm.com/downloads)

Após instalado, faça as configurações de ssh seguindo a [documentação](https://docs.github.com/pt/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).

Então, configure no terminal

```bash
# Configurar seu nome no git
git config --global user.name "Seu nome aqui"

#Configurar seu email no git
git config --global user.mail "email@email.com"

# Estes comandos para evitar conflitos nas mensagens de commit.
git config --global core.autocrlf input   # Linux/Mac
git config --global core.autocrlf true    # Windows
```

## Visual Studio Code

Tenha a extensão [.editorconfig](https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig) instalada.

## SDL

No windows:
Clone o projeto e crie uma nova pasta com o nome "SDL2" na raiz do projeto. Baixe o arquivo "SDL2-devel-2.32.2-mingw.tar.gz" no site https://github.com/libsdl-org/SDL/releases. Extraia o arquivo instalado fora do projeto. Após a extrair, vá em "\SDL2-2.32.2\i686-w64-mingw32", copie as pastas "include" e "lib" para a pasta "SDL2" do projeto. Por fim, ainda no arquivo extraido, vá em "\SDL2-2.32.2\i686-w64-mingw32\bin" e copie a pasta SDL2.dll para a raiz do projeto.

No linux:
Use o comando 
```bash
yay -S sdl2
```

## Gerando o executável

No windows: 
```bash
gcc -Wall -std=c99 -I SDL2/include -L SDL2/lib ./*.c -lmingw32 -lSDL2main -lSDL2 -lm -o game
```

No linux:
```bash
gcc -Wall -std=c99 ./*.c -lSDL2 -lm -o game
```
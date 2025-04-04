# Dev tips

Este arquivo é para ter informações mais detalhadas sobre
  como utilizar e modificar o código nos padrões estabelecidos pelo grupo.

## Git

Instale o git via [link](https://git-scm.com/downloads)

Após instalado, faça as configurações de ssh seguindo a [documentação.](https://docs.github.com/pt/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).

### Configuração do SSH (Windows/Linux/Mac)

1. Gere uma chave SSH:

   ```bash
   ssh-keygen -t ed25519 -C "seu_email@exemplo.com"
   # Pressione Enter para todas as perguntas
   ```

2. Copie a chave pública

    ```bash
    # Copie a chave
    cat ~/.ssh/id_ed25519.pub
    ```

3. Cole a chave pública em: [GitHub SSH Keys](https://github.com/settings/keys)

Então, configure no terminal

```bash
# Configurar seu nome no git
git config --global user.name "Seu nome aqui"

#Configurar seu email no git
git config --global user.mail "email@email.com"

# Estes comandos para evitar conflitos nas mensagens de commit.
git config --global core.autocrlf input   # Linux/Mac
git config --global core.autocrlf true    # Windows

# Clonar o repositório
git clone git@github.com:mangabyte/surucursed.git

# Antes de iniciar qualquer alteração
git pull origin main #Assim você estará trabalhando com a versão mais atualizada
```

### Git para o dia a dia

Estes comandos devem ser executados sempre que o código for ser iniciado no dia,
para evitar conflitos futuros.

```bash
git status #Verifica qual branch você está no momento.

git checkout main #Volta para branch main, se não estiver nela.

git pull origin main #Baixa as atualizações do github
```

Para criar nova branch, sempre a partir da main.

```bash
git checkout main
git pull origin main
#Este comando faz a troca para uma branch, e a flag "-b" cria a branch nova
git checkout -b nome-da-branch
```

Para fazer commits ([LEMBRE DO PADRÃO AQUI](https://github.com/iuricode/padroes-de-commits))

```bash
git add nomedoarquivo.c

git commit -m "Mensagem do commit aqui"
#Lembre de seguir o padrão estabelecido!

#Quando é o primeiro commit da branch
git push --set-upstream origin nome-da-branch

# Quando a branch já existe no github
git push
```

#### 2. **Padrão de Commits (Exemplos Práticos)**

```markdown
### Exemplos de Commits
| Tipo     | Mensagem                         |
|----------|----------------------------------|
| feat     | `feat: adicionar menu de pausa`  |
| fix       | `fix: corrigir colisão da cobra`  |
| docs     | `docs: atualizar guia de estilo` |
| chore    | `chore: atualizar dependências`  |
```

## Github

### Fluxo Ideal de Trabalho

Fluxo Ideal de Trabalho:

  1. git pull origin main
  2. git checkout -b nova-feature
  3. Desenvolver + commits
  4. git push --set-upstream origin nova-feature
  5. Criar PR no GitHub
  6. Revisão + Merge

Ao submeter uma alteração, lembre de fazer os testes antes de criar o Pull
Request.
Faça a requisição para todos fazerem o review do código, marcando todos do
projeto. (Esta parte é em Reviewers, ao lado do PR.)
Adicione você, como responsável por aquele PR, em assignee
Coloque uma descrição do que você realizou e descreva o máximo possível da
lógica utilizada para facilitar o trabalho de quem vai fazer o code review.
Lembrem-se de não ficarem chateados com possíveis request changes (pedido de
mudança), estamos nos formando para sermos profissionais, e correções são muito
comuns no mercado de trabalho e no meio acadêmico.
Vamos buscar estabelecer um bom projeto com padrões de código, commit e aprender
a usar bem o github, pois vai ser muito útil para o nosso futuro.

Adicione também labels à sua PR para ficar mais fácil o code review.

## Visual Studio Code

Não é obrigatório usar o VSCode, mas é recomendado, pois boa parte dos
integrantes vão utilizá-lo, então é mais fácil se todos usarem a mesma
ferramenta. Mesmo assim, caso prefira usar uma outra IDE, certifique dela ter a
extensão abaixo, EditorConfig, ou então configure a sua IDE para manter o padrão
estabelecido.
Quaisquer dúvidas não devem ser deixadas de lado e podem criar uma issue
(preferível) ou enviar mensagem no grupo de WhatsApp. Dê preferência a criar uma
[issue](https://github.com/mangabyte/surucursed/issues)

Tenha a extensão [.editorconfig](https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig) instalada.

### Outras extensões úteis

#### **Desenvolvimento em C/SDL**  
- **C/C++ (Microsoft) / C/C++ Extension Pack**: Autocompletar, debug e suporte avançado para C/C++.  
- **CMake/CMake Tools**: Suporte para autocompletar e gerenciamento de projetos CMake.  
- **SDL Snippets**: Atalhos para funções e estruturas da SDL.  

#### **Produtividade**  
- **Code Runner**: Execute trechos de código rapidamente com `Ctrl+Alt+N`.  
- **C/C++ Compile Run**: Botão para compilar e executar projetos em C diretamente na interface.
- **Error Lens**: Destaca erros na linha inteira

#### **Qualidade de Código**  
- **Code Spell Checker**: Verificação ortográfica em comentários e textos.  
- **Portuguese - Code Spell Checker**: Dicionário em português para o Spell Checker.  

#### **Controle de Versão (Git)**  
- **GitLens**: Histórico de alterações, blame e insights sobre o código.  

#### **Extras Úteis**  
- **Code Snap**: Tire screenshots estilizadas do código para compartilhar.  
- **WakaTime**: Mede o tempo gasto no projeto (útil para métricas de produtividade).  
- **Discord Presence**: Mostra no Discord o que você está codificando.  

## SDL

### No windows:

- Clone o projeto e crie uma nova pasta com o nome "SDL2" na raiz do projeto.
- Baixe o arquivo "[SDL2-devel-2.32.2-mingw.tar.gz](https://github.com/libsdl-org/SDL/releases)".
- Extraia o arquivo instalado fora do projeto. 
- Após a extrair, vá em "\SDL2-2.32.2\i686-w64-mingw32", copie as pastas "include" e "lib" para a pasta "SDL2" do projeto. 
- Por fim, ainda no arquivo extraido, vá em "\SDL2-2.32.2\i686-w64-mingw32\bin" e copie o arquivo SDL2.dll para a raiz do projeto.

### No linux:

Use o comando 
```bash
yay -S sdl2
```

### Gerando o executável

#### No windows: 
```bash
gcc -Wall -std=c99 -I SDL2/include -L SDL2/lib ./*.c -lmingw32 -lSDL2main -lSDL2 -lm -o game
```

#### No linux:
```bash
gcc -Wall -std=c99 ./*.c -lSDL2 -lm -o game
```

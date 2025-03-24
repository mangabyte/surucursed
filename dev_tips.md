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

# Clonar o repositório
git clone git@github.com:mangabyte/surucursed.git
```

## Visual Studio Code

Tenha a extensão [.editorconfig](https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig) instalada.

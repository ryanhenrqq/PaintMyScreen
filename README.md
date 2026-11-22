# PencilMyScreen - Programa de desenho em linux

- IMPORTANTE: A primeira versão desse programa foi feita inteiramente por IA!

- Feito em GTK4 e pensado pra linux (testado no fedora 44 Gnome)

- Para compilar de a permissao de "chmod +x comp.sh", e rode "./comp.sh" (se quiser outro nome rode "../comp.sh <nome>")

- Meus fins com esse programa não é financeiro, fiz ele pra atender uma necessidade minha, aprender mais sobre C e poder revisitar esse codigo depois. Tentei deixa-lo o mais simples e documentado possivel dentro das linhas de codigo pra ficar facil uma melhoria ou manutenção posterior.

- PencilMyScreen é totalmente livre e de codigo aberto, fique a vontade para baixar e modifica-lo a seu gosto!

## Instalação

- Esta instalação foi pensada pro fedora com pacotes vindos do DNF, dependendo da distro usada ela pode variar.

- 1. Instale os pacotes necessários para compilar:
```bash
	sudo dnf install gcc gtk4-devel cairo-devel pkg-config
```

- 2. Dê a permissão de execução pro compilador embutido:
```bash
	chmod +x build.sh
```

- 3. Para compilar, use:
```bash
	./comp.sh PencilMyScreen
```
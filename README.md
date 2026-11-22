# PencilMyScreen - Programa de desenho em linux

- IMPORTANTE: A primeira versão desse programa foi feita inteiramente por IA!

- Feito em GTK4 e pensado pra linux (testado no fedora 44 Gnome)

- Meus fins com esse programa não é financeiro, fiz ele pra atender uma necessidade minha, aprender mais sobre C e poder revisitar esse codigo depois. Tentei deixa-lo o mais simples e documentado possivel dentro das linhas de codigo pra ficar facil uma melhoria ou manutenção posterior.

- PencilMyScreen é totalmente livre e de codigo aberto, fique a vontade para baixar e modifica-lo a seu gosto!

## Instalação

### Ubuntu/Debian

- 1. Instale os pacotes necessários para compilar:
```bash
	sudo apt update
	sudo apt install git build-essential libgtk-4-dev libcairo2-dev pkg-config
```

- 2. Instale o projeto na sua maquina:
```bash
	git clone https://github.com/ryanhenrqq/PaintMyScreen.git
	cd PaintMyScreen
```

- 3. Dê a permissão de execução pro compilador embutido:
```bash
	chmod +x build.sh
```

- 4. Para compilar, use:
```bash
	./comp.sh PencilMyScreen
```

- 5. Será gerado um arquivo .bin, voce pode abri-lo apenas com 2 cliques!

### Fedora/RedHat

- 1. Instale os pacotes necessários para compilar:
```bash
	sudo dnf install git gcc gtk4-devel cairo-devel pkg-config
```

- 2. Instale o projeto na sua maquina:
```bash
	git clone https://github.com/ryanhenrqq/PaintMyScreen.git
	cd PaintMyScreen
```

- 3. Dê a permissão de execução pro compilador embutido:
```bash
	chmod +x build.sh
```

- 4. Para compilar, use:
```bash
	./comp.sh PencilMyScreen
```

- 5. Será gerado um arquivo .bin, voce pode abri-lo apenas com 2 cliques!

### Windows/Via MSYS2 UCRT64 (EXPERIMENTAL)

- 1. Instale o MSYS2 pelo site msys2.org

- 2. Abra o terminal MSYS2 UCRT64 (não use o terminal padrão do windows!)

- 3. Instale os pacotes necessários para compilar:
```bash
	pacman -Syu
	pacman -S --needed git mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkg-config mingw-w64-ucrt-x86_64-gtk4
```

- 4. Instale o projeto na sua maquina:
```bash
	git clone https://github.com/ryanhenrqq/PaintMyScreen.git
	cd PaintMyScreen
```

- 5. Compile o projeto diretamente usando:
```bash
	gcc $(pkg-config --cflags gtk4) main.c canvas.c ui.c -o PencilMyScreen.exe $(pkg-config --libs gtk4) -lm
```

- 5. Será gerado um arquivo .exe, voce pode abri-lo apenas com 2 cliques!
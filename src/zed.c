// includes
#include <asm-generic/ioctls.h>
#include<errno.h>
#include<stdio.h>
#include<ctype.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<sys/ioctl.h>

// defines
#define CTRL_KEY(k) ((k) & 0x1f)

// data
//Configuración original
struct editorConfig
{
    struct termios og_termios;
    int screenRows;
    int screenCols;
};

struct editorConfig E;

// terminal
void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    
    perror(s); //Imprimir error
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.og_termios) == -1)
    {
        die("tcsetattr");
    }
}

void enableRawMode()
{
    //Activamos el modo 'raw' para poder leer el texto de forma
    // continua sin tener que pulsar 'Enter'

    //Guardamos la configuración actual en og_termios
    if (tcgetattr(STDIN_FILENO, &E.og_termios) == -1) die("tcgetattr");
    //Llamar a la función cuando se llame a 'exit'
    atexit(disableRawMode);
    
    struct termios raw = E.og_termios;
    //Desactivar modo canónico para leer byte por byte
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    raw.c_cc[VMIN] = 0; //Que lea al mínimo byte
    raw.c_cc[VTIME] = 1; //máximo tiempo a esperar para que
    //'read' devuelve
    //Cada carácter se imprime a la terminal
    //lflag es 'local flag'
    // 'ECHO' es una bitflag. El operador '~' es el operador NOT
    // para bits
    // Desactivamos señales para detener y suspender
    // Ctrl-C y Ctrl-z
    // Ctrl-s desactiva la transmisión al terminal
    // Ctrl-Q lo activa de nuevo

    //Le pasamos los atributos al terminal
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;
    
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    printf("\r\n");
    char c;



    editorReadKey();

    return -1;
}

// output
void editorDrawRows()
{
    for (int y = 0; y < E.screenRows; y++)
    {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen()
{
    //4: escribimos 4 bytes al terminal
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

char editorReadKey()
{
    //Esta función espera a leer una entrada de texto y la devuelve.
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN) die("read");
    }

    return c;
}

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    //Error handling
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;

        return 0;
    }
}

void editorProcessKeypress()
{
    char c = editorReadKey();

    switch(c)
    {
        case CTRL_KEY('q'):
            
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

// init
void initEditor()
{
    if (getWindowSize(&E.screenRows, &E.screenCols) == -1) die("getWindowSize");
}

int main()
{
    enableRawMode();
    initEditor();
    
    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    
    return 0;
}

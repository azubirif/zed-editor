#include<stdio.h>
#include<ctype.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>

//Configuración original
struct termios og_termios;

void die(const char *s)
{
    perror(s); //Imprimir error
    exit(1);
}

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &og_termios);
}

void enableRawMode()
{
    //Activamos el modo 'raw' para poder leer el texto de forma
    // continua sin tener que pulsar 'Enter'

    //Guardamos la configuración actual en og_termios
    tcgetattr(STDIN_FILENO, &og_termios);
    //Llamar a la función cuando se llame a 'exit'
    atexit(disableRawMode);
    
    struct termios raw = og_termios;
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
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
    enableRawMode();
    
    char c;
    //Leemos de forma continua 1 byte hasta que no pueda mas
    // Devolverá 0 cuando no pueda seguir leyendo
    while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
        //Si es carácter de control, solo imprimir ASCII
        if (iscntrl(c)) printf("%d\r\n", c);
        else printf("%d - '%c'\r\n", c, c);
    }
    return 0;
}

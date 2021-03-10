#include "env.h"

#include <stdio.h>
#include <stdlib.h>

static FILE* logfile; 

void 
fatal(char* message)
{
    printlog(message);
    stop_logging();
    exit(1);
}

void
exit_prog(char code)
{
    exit(code);
}

void
init_logging()
{
    logfile = fopen("log.txt", "w");
}

void
printlog(char* message)
{
    fprintf(logfile, "%s", message);
}

void
stop_logging()
{
    {
        if (vi->fb->headl != NULL) { 
            printflog("\n\rHEADL -> %s (size: %d) \n\r", vi->fb->headl->data, 
                    vi->fb->headl->size);
        }
        else {printlog("\n\rWARNING HEAD ISNULL\n\r");}
    }
    {
        if (vi->fb->taill != NULL) {
            printflog("TAILL -> %s\n\r", vi->fb->taill->data);
        }
        else { printlog("WARNING TAILL ISNULL\n\r");}

    }
    {
        printflog("CURRENT -> %s\n\r", vi->fb->currentl->data);
    }
    printflog("Buffer: %d, %d (%d, %d)", vi->fb->buffer_r, vi->fb->buffer_c, 
            vi->term->cursor_r, vi->term->cursor_c);
    printlog("vi_line* head\n");
    printlog("       |\n"); 
    struct vi_line* line = vi->fb->headl;
    struct vi_line* prev = NULL;
    while (line != NULL) {
        printflog("       +---- vi_line (size: %d, data: ", line->size);
        vimemset(vi->statusbuf, 0, 30);
        for (uint8_t i = 0; i < line->size - 1; i++) {
            if (i >= line->sog && i <= line->eog) *(vi->statusbuf + i) = '_';
            else *(vi->statusbuf + i) = line->data[i];
        } 
        fprintf(logfile, "%s", vi->statusbuf);

        for (uint32_t i = line->sog + 1; i < line->eog; i++) printlog(" ");
        printlog("^\n");
        {
        printflog("              data_n: %d, sog: %d, eog: %d\n\r", line->data_n, line->sog, line->eog);
        }
        if (line->prevl != prev) {
            printlog("WARNING PREVIOUS POINTER INVALID");
        }
        prev = line;
        line = line->nextl;
    }
    fclose(logfile);
}

#include "getopt.h"
#include "stdio.h"

static void MiClientDaemonExit(void)
{
    HANDLE hShutdownEvent;
    char szShutdownName[128];

    PR_snprintf(szShutdownName, sizeof(szShutdownName), 
        "WaveTop_IO_Daemon_Shutdown_%d", getpid());
    hShutdownEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, szShutdownName);
    if (NULL == hShutdownEvent) {
        return;
    }

    SetEvent(hShutdownEvent);
    CloseHandle(hShutdownEvent);
    for (;;) {
        hShutdownEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, szShutdownName);
        if (NULL == hShutdownEvent) {
            break;
        }

        CloseHandle(hShutdownEvent);
        Sleep(50);
    }
}

int CdaeMain(int argc, char **argv, char **env)
{
    char chCmdopt = '\0';
    int nServiceCtrl = 1;
    HRESULT hr      = 0;
    char szServiceCtrlStr[MAX_PATH];

    while ((chCmdopt = (char)getopt(
        argc, argv, "f:oOP:xXZ:k:K:hH?Lm:M:")) != (char)EOF) {
            switch (chCmdopt) {
            case 'k':
            case 'K':
                if (0 != stricmp(optarg, "ntservice"))
                    exit(0);
                else{
                    hr = StringCchPrintfA(szServiceCtrlStr, sizeof(szServiceCtrlStr), optarg);
                    if(SUCCEED(hr)){
                        nServiceCtrl = 1;
                    }else
                        exit(0);
                }
                break;

            default:
                exit(0);
            }
    }

    if (1 == nServiceCtrl) {
        rv = MiCDaemonServiceControl(szServiceCtrlStr, CdaeMain, MiClientDaemonExit);
        if (rv == WAVETOP_BACKUP_OK) { 
            fprintf(stdout, "%s Mirror Agent Service Successful\n", szServiceCtrlStr);
        }
        else {
            fprintf(stdout, "%s Mirror Agent Service Failed\n", szServiceCtrlStr);
        }
        return (rv == WAVETOP_BACKUP_OK ? 0 : -1);
    }



}


/** The binrary entry **/
int main(int argc, char **argv, char **env)
{
    return CdaeMain(argc, argv, env);
}

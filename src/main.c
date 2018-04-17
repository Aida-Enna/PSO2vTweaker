/*
Humans must learn to apply their intelligence correctly and evolve beyond their current state.
People must change. Otherwise, even if humanity expands into space, it will only create new
conflicts, and that would be a very sad thing. - Aeolia Schenberg, 2091 A.D.
*/
#define VITASDK

#include <psp2/sysmodule.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>
#include <psp2/io/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "debugScreen.h"

#define printf psvDebugScreenPrintf

void netInit() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	SceNetInitParam netInitParam;
	int size = 1*1024*1024;
	netInitParam.memory = malloc(size);
	netInitParam.size = size;
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);
	sceNetCtlInit();
}

void netTerm() {
	sceNetCtlTerm();
	sceNetTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

void httpInit() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceHttpInit(1*1024*1024);
}

void httpTerm() {
	sceHttpTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
}

void download(const char *url, const char *dest) {

	// Create template with user agend "PSO2v Tweaker"
	int tpl = sceHttpCreateTemplate("PSO2v Tweaker", 1, 1);
	//psvDebugScreenPrintf("0x%08X sceHttpCreateTemplate\n", tpl);

	// set url on the template
	int conn = sceHttpCreateConnectionWithURL(tpl, url, 0);

	// create the request with the correct method
	int request = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, url, 0);

	// send the actual request. Second parameter would be POST data, third would be length of it.
	sceHttpSendRequest(request, NULL, 0);
	//int handle = sceHttpSendRequest(request, NULL, 0);

	// open destination file
	int fh = sceIoOpen(dest, SCE_O_WRONLY | SCE_O_CREAT, 0777);

	// create buffer and counter for read bytes.
	unsigned char data[16*1024];
	int read = 0;

	// read data until finished
	while ((read = sceHttpReadData(request, &data, sizeof(data))) > 0) {

		// writing the count of read bytes from the data buffer to the file
		//int write = sceIoWrite(fh, data, read);
	}

	// close file
	sceIoClose(fh);
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file,SCE_O_RDONLY, 0777);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int getFileSize(const char *file) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	int fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);
	sceIoClose(fd);
	return fileSize;
}

int main(int argc, char *argv[]) {
	//cmake . && make
	psvDebugScreenInit();
	
	//http://patorjk.com/software/taag/#p=display&f=Basic&t=PSO2v%20Tweaker
	psvDebugScreenPrintf("\e[34m" "d8888b. .d8888.  .d88b.  .d888b. db    db      d888888b db   d8b   db d88888b  .d8b.  db   dD d88888b d8888b. \n");
	psvDebugScreenPrintf("\e[34m" "88  `8D 88'  YP .8P  Y8. VP  `8D 88    88      `~~88~~' 88   I8I   88 88'     d8' `8b 88 ,8P' 88'     88  `8D \n");
	psvDebugScreenPrintf("\e[34m" "88oodD' `8bo.   88    88    odD' Y8    8P         88    88   I8I   88 88ooooo 88ooo88 88,8P   88ooooo 88oobY' \n");
	psvDebugScreenPrintf("\e[34m" "88~~~     `Y8b. 88    88  .88'   `8b  d8'         88    Y8   I8I   88 88~~~~~ 88~~~88 88`8b   88~~~~~ 88`8b   \n");
	psvDebugScreenPrintf("\e[34m" "88      db   8D `8b  d8' j88.     `8bd8'          88    `8b d8'8b d8' 88.     88   88 88 `88. 88.     88 `88. \n");
	psvDebugScreenPrintf("\e[34m" "88      `8888Y'  `Y88P'  888888D    YP            YP     `8b8' `8d8'  Y88888P YP   YP YP   YD Y88888P 88   YD \n");
	psvDebugScreenPrintf("\e[39;49m" "\n\n");
	
	//psvDebugScreenFont.size_w += 1;
	//psvDebugScreenFont.size_h += 1;
	
	/*TODO
	Have it check to make sure repatch is enabled
	Check to make sure the game is installed/updated
	Read info.txt from website and display info (optional)
	Make it create a blank release_old.txt in ux0:/data/PSO2vTweaker if it's not there
	Download release.txt and compare it to saved one (release_old.txt)
	If they don't match (dates), then download release_url.txt and download the file in that to the repatch folder and overwrite the release_old.txt
	*/
	
	struct stat st = {0};

	if (stat("ux0:/data/PSO2vTweaker", &st) == -1) {
	psvDebugScreenPrintf("Creating app directory...\n");
	sceIoMkdir("ux0:/data/PSO2vTweaker", 0777);
	}
	if (stat("ux0:/rePatch", &st) == -1) {
	sceIoMkdir("ux0:/rePatch", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141", &st) == -1) {
	sceIoMkdir("ux0:/rePatch/PCSG00141", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141/data", &st) == -1) {
	sceIoMkdir("ux0:/rePatch/PCSG00141/data", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141/data/vita", &st) == -1) {
	sceIoMkdir("ux0:/rePatch/PCSG00141/data/vita", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141/data/vita/patches", &st) == -1) {
	sceIoMkdir("ux0:/rePatch/PCSG00141/data/vita/patches", 0777);
	}
	else
	{
		psvDebugScreenPrintf("Clearing patches directory...\n");
		sceIoRmdir("ux0:/rePatch/PCSG00141/data/vita/patches", 0777);
		sceKernelDelayThread(10000);
		sceIoMkdir("ux0:/rePatch/PCSG00141/data/vita/patches", 0777);
	}
	
	printf("This will check for/update the PSO2 vita English Patch to the newest version available.\n"
		"If this program fails to patch/update for some reason, you can download the patch from http://arks-layer.com/pso2v.php.\n\n");
		
	netInit();
	httpInit();

	psvDebugScreenPrintf("Checking for a new version of the patch... ");
    download("http://arks-layer.com/vita/release.txt", "ux0:data/PSO2vTweaker/release.txt");
    
    int size = getFileSize("ux0:data/PSO2vTweaker/release.txt");
    char *releaseinfo = malloc(1024);
    memset(releaseinfo, 0, 1024);
    releaseinfo[1024] = 0x00;
	ReadFile("ux0:data/PSO2vTweaker/release.txt",releaseinfo,size + 1);
	
	psvDebugScreenPrintf("\nDone!\nThe latest patch appears to have been created on ");
	psvDebugScreenPrintf("%c", releaseinfo);
	psvDebugScreenPrintf("\n");
	
	
	httpTerm();
	netTerm();

	psvDebugScreenPrintf("All done! Press X to exit.\n");
	while (1) {
		SceCtrlData pad;
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if (pad.buttons & SCE_CTRL_CROSS)
		{
		sceKernelExitProcess(0);
		return 0;		
		}
		sceKernelDelayThread(10000);
	}
}
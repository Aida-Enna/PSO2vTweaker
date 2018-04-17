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
#include <psp2/shellutil.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
		/*int write = */sceIoWrite(fh, data, read);
	}

	// close file
	sceIoClose(fh);
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
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

bool FileExists(const char* file) {
    struct stat buf;
    return (stat(file, &buf) == 0);
}

void lock_psbutton() {
    sceShellUtilLock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN |
                     SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU);
}

void unlock_psbutton() {
    sceShellUtilUnlock(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN |
                       SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU);
}

int main(int argc, char *argv[]) {
	//cmake . && make
	psvDebugScreenInit();
	
	//http://patorjk.com/software/taag/#p=display&f=Basic&t=PSO2v%20Tweaker
	psvDebugScreenPrintf("\e[34m" "d8888b. .d8888.  .d88b.  " "\e[31m" ".d888b. " "\e[34m" "db    db      " "\e[33m" "d888888b db   d8b   db d88888b  .d8b.  db   dD d88888b d8888b. \n");
	psvDebugScreenPrintf("\e[34m" "88  `8D 88'  YP .8P  Y8. " "\e[31m" "VP  `8D " "\e[34m" "88    88      " "\e[33m" "`~~88~~' 88   I8I   88 88'     d8' `8b 88 ,8P' 88'     88  `8D \n");
	psvDebugScreenPrintf("\e[34m" "88oodD' `8bo.   88    88 " "\e[31m" "   odD' " "\e[34m" "Y8    8P      " "\e[33m" "   88    88   I8I   88 88ooooo 88ooo88 88,8P   88ooooo 88oobY' \n");
	psvDebugScreenPrintf("\e[34m" "88~~~     `Y8b. 88    88 " "\e[31m" " .88'   " "\e[34m" "`8b  d8'      " "\e[33m" "   88    Y8   I8I   88 88~~~~~ 88~~~88 88`8b   88~~~~~ 88`8b   \n");
	psvDebugScreenPrintf("\e[34m" "88      db   8D `8b  d8' " "\e[31m" "j88.    " "\e[34m" " `8bd8'       " "\e[33m" "   88    `8b d8'8b d8' 88.     88   88 88 `88. 88.     88 `88. \n");
	psvDebugScreenPrintf("\e[34m" "88      `8888Y'  `Y88P'  " "\e[31m" "888888D " "\e[34m" "   YP         " "\e[33m" "   YP     `8b8' `8d8'  Y88888P YP   YP YP   YD Y88888P 88   YD \n");
	psvDebugScreenPrintf("\e[39;49m" "\n\n");
	
	//psvDebugScreenFont.size_w += 1;
	//psvDebugScreenFont.size_h += 1;
	
	/*TODO
	[Done!] Check to make sure the game is installed/updated
	[Done!] Make it create a blank release_old.txt in ux0:/data/PSO2vTweaker if it's not there
	[Done!] Download release.txt and compare it to saved one (release_old.txt)
	If they don't match (dates), then download release_url.txt and download the file in that to the repatch folder and overwrite the release_old.txt
	*/
	
	struct stat st = {0};
	bool install_needed = false;

	if (stat("ux0:/app/PCSG00141", &st) == -1) 
	{
	psvDebugScreenPrintf("\e[31m" "Couldn't find PSO2 vita - Are you sure it's installed? (ux0:/app/PCSG00141)\n");
	psvDebugScreenPrintf("\e[31m" "A critical error has occurred. Please press X to exit the program.");
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
	if (stat("ux0:/patch/PCSG00141", &st) == -1) 
	{
	psvDebugScreenPrintf("\e[31m" "Couldn't find PSO2 vita update data - Are you sure it's updated?\nStart the game and select \"Online Login\" to force an update check. (ux0:/patch/PCSG00141)\n");
	psvDebugScreenPrintf("\e[31m" "A critical error has occurred. Please press X to exit the program.");
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
	
	//If our app doesn't have a directory, make it.
	if (stat("ux0:/data/PSO2vTweaker", &st) == -1) 
	{
	psvDebugScreenPrintf("Creating app directory...\n");
	sceIoMkdir("ux0:/data/PSO2vTweaker", 0777);
	}
	
	//Check for old patch info, write a new one if it's not there (first boot)
	if (!FileExists("ux0:/data/PSO2vTweaker/release_old.txt")) 
	{
	psvDebugScreenPrintf("Couldn't find old patch info, writing...\n");	
	WriteFile("ux0:/data/PSO2vTweaker/release_old.txt","1/1/2091",sizeof("1/1/2091"));
	}
	
	//Make the rePatch directories if they don't exist.
	if (stat("ux0:/rePatch", &st) == -1) 
	{
	sceIoMkdir("ux0:/rePatch", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141", &st) == -1) 
	{
	sceIoMkdir("ux0:/rePatch/PCSG00141", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141/data", &st) == -1) 
	{
	sceIoMkdir("ux0:/rePatch/PCSG00141/data", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141/data/vita", &st) == -1) 
	{
	sceIoMkdir("ux0:/rePatch/PCSG00141/data/vita", 0777);
	}
	if (stat("ux0:/rePatch/PCSG00141/data/vita/patches", &st) == -1) 
	{
	sceIoMkdir("ux0:/rePatch/PCSG00141/data/vita/patches", 0777);
	}
	
	printf("This will check for/update the PSO2 vita English patch to the newest version available.\n"
		"If this program fails to patch/update for some reason, you can download the patch from http://arks-layer.com/.\n\n");
		
	psvDebugScreenPrintf("\e[31m" "!!!Please make sure that you have the rePatch plugin installed and enabled.!!!\n\n" "\e[39;49m");
	netInit();
	httpInit();

	psvDebugScreenPrintf("Checking for a new version of the patch... ");
	
	download("http://arks-layer.com/vita/release_url.txt", "ux0:data/PSO2vTweaker/release_url.txt");
	
	int url_size = getFileSize("ux0:data/PSO2vTweaker/release_url.txt");
	char *releaseinfo_url = malloc(1024);
	memset(releaseinfo_url, 0, 1024);
	releaseinfo_url[1024] = 0x00;
	ReadFile("ux0:data/PSO2vTweaker/release_url.txt",releaseinfo_url,url_size);
	//psvDebugScreenPrintf("Download URL: %s.\n",releaseinfo_url);
	
	int SIZE_FILENAME = getFileSize(releaseinfo_url);
	char *filename = (char*)calloc(1, sizeof(SIZE_FILENAME));
	filename = (strrchr(releaseinfo_url, '/'))+1;
	
	char str_output[99]={0};
 
	strcpy(str_output, "ux0:/rePatch/PCSG00141/data/vita/patches/");
	strcat(str_output, filename);
	
	//Download the latest release eg. "4/16/2018"
    download("http://arks-layer.com/vita/release.txt", "ux0:data/PSO2vTweaker/release.txt");
    int size = getFileSize("ux0:data/PSO2vTweaker/release.txt");
    char *releaseinfo = malloc(1024);
    memset(releaseinfo, 0, 1024);
    releaseinfo[1024] = 0x00;
	ReadFile("ux0:data/PSO2vTweaker/release.txt",releaseinfo,size);
	
	size = getFileSize("ux0:data/PSO2vTweaker/release_old.txt");
    char *releaseinfo_old = malloc(1024);
    memset(releaseinfo_old, 0, 1024);
    releaseinfo_old[1024] = 0x00;
	ReadFile("ux0:data/PSO2vTweaker/release_old.txt",releaseinfo_old,size);
	
	psvDebugScreenPrintf("Done!\nThe latest patch appears to have been created on %s.\n",releaseinfo);
	
	if (!FileExists(str_output)) 
	{
		install_needed = true;
	}
	
	if(install_needed == false)
	{
	if(strcmp(releaseinfo_old,"1/1/2091") == 0) 
	{
	psvDebugScreenPrintf("You don't appear to have ever installed the English patch before on this system (using PSO2v Tweaker).\n");
	install_needed = true;
	}
	else
	{
		if(strcmp(releaseinfo,releaseinfo_old) == 0) 
		{
		psvDebugScreenPrintf("You have the latest version of the English patch!\n");
		}
		else
		{
		//psvDebugScreenPrintf("The patch you have installed appears to be from ");
		//psvDebugScreenPrintf(releaseinfo_old);
		//psvDebugScreenPrintf(".\n");
		install_needed = true;
		}	
	}
	}
	
	if(install_needed == true) 
	{
		if (!FileExists(str_output)) 
		{
			if(strcmp(releaseinfo_old,"1/1/2091") != 0) 
			{
			psvDebugScreenPrintf("\e[31m" "Unable to locate the English patch file! Would you like to re-install it?\nPress X for yes, O for no.\n" "\e[39;49m");
			}
		}
		else
		{
			psvDebugScreenPrintf("Would you like to install/update the English patch? Press X for yes, O for no.\n");
		}
		while (1) {
		SceCtrlData pad;
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if (pad.buttons & SCE_CTRL_CROSS)
		{
		lock_psbutton();
		psvDebugScreenPrintf("Downloading patch, please wait...\n");
		
		/*//Download the latest release eg. "4/16/2018"
		size = getFileSize("ux0:data/PSO2vTweaker/release_url.txt");
		char *releaseinfo_url = malloc(1024);
		memset(releaseinfo_url, 0, 1024);
		releaseinfo_url[1024] = 0x00;
		ReadFile("ux0:data/PSO2vTweaker/release_url.txt",releaseinfo_url,size);
		//psvDebugScreenPrintf("Download URL: %s.\n",releaseinfo_url);*/
		
		//Remove all patches before we patch, just in case.
		//psvDebugScreenPrintf("Clearing patches directory...\n");
		sceIoRmdir("ux0:/rePatch/PCSG00141/data/vita/patches", 0777);
		sceKernelDelayThread(10000);
		sceIoMkdir("ux0:/rePatch/PCSG00141/data/vita/patches", 0777);
		
		/*int SIZE_FILENAME = getFileSize(releaseinfo_url);
		char *filename = (char*)calloc(1, sizeof(SIZE_FILENAME));
		filename = (strrchr(releaseinfo_url, '/'))+1;
		//psvDebugScreenPrintf(" found filename: %s \n", filename);*/
		
		download(releaseinfo_url,str_output);
		
		sceKernelDelayThread(10000);
		
		SceUID fdsrc = sceIoOpen("ux0:data/PSO2vTweaker/release.txt", SCE_O_RDONLY, 0);

		int fdsize = sceIoLseek(fdsrc, 0, SEEK_END);
		sceIoLseek(fdsrc, 0, SEEK_SET);
		void *buf = malloc(fdsize);
		sceIoRead(fdsrc, buf, fdsize);
		sceIoClose(fdsrc);

		SceUID fddst = sceIoOpen("ux0:data/PSO2vTweaker/release_old.txt", SCE_O_WRONLY | SCE_O_CREAT, 0777);
		
		sceIoWrite(fddst, buf, fdsize);
		sceIoClose(fddst);
		
		//download("http://arks-layer.com/vita/release.txt", "ux0:data/PSO2vTweaker/release_old.txt");
		psvDebugScreenPrintf("\e[32m" "Patch downloaded to ux0:/rePatch/PCSG00141/data/vita/patches/. Installation complete!\n" "\e[39;49m");
		sceKernelDelayThread(50000);
		break;
		}
		if (pad.buttons & SCE_CTRL_CIRCLE)
		{
		psvDebugScreenPrintf("Installation aborted by user.\n");	
		break;
		}
		sceKernelDelayThread(10000);
		}
	}
	
	httpTerm();
	netTerm();
	unlock_psbutton();
	
	sceKernelDelayThread(20000);
	free(releaseinfo_url);
	free(releaseinfo);
	free(releaseinfo_old);
	psvDebugScreenPrintf("Press X to exit.");
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
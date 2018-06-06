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
#include <stdlib.h>
#include <stdarg.h>

#include "debugScreen.h"

#define printf psvDebugScreenPrintf

char* concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

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

	// Create template with user agent "PSO2v Tweaker"
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
		/*int write =*/ sceIoWrite(fh, data, read);
	}

	// close file
	sceIoClose(fh);
}

int downloadpatch(const char *src, const char *dst) {
	int ret;
	int statusCode;
	int tmplId = -1, connId = -1, reqId = -1;
	SceUID fd = -1;

	ret = sceHttpCreateTemplate("PSO2v Tweaker", SCE_HTTP_VERSION_1_1, SCE_TRUE);
	if (ret < 0)
		goto ERROR_EXIT;

	tmplId = ret;

	ret = sceHttpCreateConnectionWithURL(tmplId, src, SCE_TRUE);
	if (ret < 0)
		goto ERROR_EXIT;

	connId = ret;

	ret = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_GET, src, 0);
	if (ret < 0)
		goto ERROR_EXIT;

	reqId = ret;

	ret = sceHttpSendRequest(reqId, NULL, 0);
	if (ret < 0)
		goto ERROR_EXIT;

	ret = sceHttpGetStatusCode(reqId, &statusCode);
	if (ret < 0)
		goto ERROR_EXIT;

	if (statusCode == 200) {
		uint8_t buf[16*1024];
		uint64_t size = 0;
		uint32_t value = 0;

		ret = sceHttpGetResponseContentLength(reqId, &size);
		if (ret < 0)
			goto ERROR_EXIT;

		ret = sceIoOpen(dst, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 6);
		if (ret < 0)
			goto ERROR_EXIT;

		fd = ret;

		while (1) {
			int read = sceHttpReadData(reqId, buf, sizeof(buf));

			if (read < 0) {
				ret = read;
				break;
			}

			if (read == 0)
				break;

			int written = sceIoWrite(fd, buf, read);

			if (written < 0) {
				ret = written;
				break;
			}

			value += read;

			if ((value * 100 / (uint32_t)size) < 77)
			{
				printf("\rDownloading patch, please wait.....%d%%    ", (value * 100) / (uint32_t)size);
			}
			else
			{
				printf("\rPerforming final download steps.....    ");
			}
		}
		printf("\rPatch downloaded successfully!          \n");
	}

ERROR_EXIT:
	if (fd >= 0)
		sceIoClose(fd);

	if (reqId >= 0)
		sceHttpDeleteRequest(reqId);

	if (connId >= 0)
		sceHttpDeleteConnection(connId);

	if (tmplId >= 0)
		sceHttpDeleteTemplate(tmplId);

	return ret;
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

int launchAppByUriExit(char *titleid) {
  char uri[32];
  sprintf(uri, "psgm:play?titleid=%s", titleid);

  sceAppMgrLaunchAppByUri(0xFFFFF, uri);
  sceKernelExitProcess(0);

  return 0;
}

int main(int argc, char *argv[]) {
	//cmake . && make
	psvDebugScreenInit();
	
	 // Close other apps
	sceAppMgrDestroyOtherApp();
	
	char *vpk_internal_ver = malloc(1024);
	memset(vpk_internal_ver, 0, 1024);
	vpk_internal_ver[1024] = 0x00;
	vpk_internal_ver = "1.03";
	
	//http://patorjk.com/software/taag/#p=display&f=Basic&t=PSO2v%20Tweaker
	psvDebugScreenPrintf("\e[94m" "d8888b. .d8888.  .d88b.  " "\e[91m" ".d888b. " "\e[94m" "db    db      " "\e[93m" "d888888b db   d8b   db d88888b  .d8b.  db   dD d88888b d8888b. \n");
	psvDebugScreenPrintf("\e[94m" "88  `8D 88'  YP .8P  Y8. " "\e[91m" "VP  `8D " "\e[94m" "88    88      " "\e[93m" "`~~88~~' 88   I8I   88 88'     d8' `8b 88 ,8P' 88'     88  `8D \n");
	psvDebugScreenPrintf("\e[94m" "88oodD' `8bo.   88    88 " "\e[91m" "   odD' " "\e[94m" "Y8    8P      " "\e[93m" "   88    88   I8I   88 88ooooo 88ooo88 88,8P   88ooooo 88oobY' \n");
	psvDebugScreenPrintf("\e[94m" "88~~~     `Y8b. 88    88 " "\e[91m" " .88'   " "\e[94m" "`8b  d8'      " "\e[93m" "   88    Y8   I8I   88 88~~~~~ 88~~~88 88`8b   88~~~~~ 88`8b   \n");
	psvDebugScreenPrintf("\e[94m" "88      db   8D `8b  d8' " "\e[91m" "j88.    " "\e[94m" " `8bd8'       " "\e[93m" "   88    `8b d8'8b d8' 88.     88   88 88 `88. 88.     88 `88. \n");
	psvDebugScreenPrintf("\e[94m" "88      `8888Y'  `Y88P'  " "\e[91m" "888888D " "\e[94m" "   YP         " "\e[93m" "   YP     `8b8' `8d8'  Y88888P YP   YP YP   YD Y88888P 88   YD " "\e[39;49m" );
	printf(vpk_internal_ver);
	printf("\n\n");
	
	//psvDebugScreenFont.size_w += 1;
	//psvDebugScreenFont.size_h += 1;
	
	/*TODO
	[Done!] Cart version has "gro0:app/PCSG00141/"
	[Done!] Make it launch the game
	[Too complicated?] Make it update like Vitashell (Thank TheFlow in credits)
	[Done!] Brighten colors
	[Done!] Make it do a version check - Write a file at startup, then check the remote one. Display a message if they don't match.
	[Todo] Offer to download the new VPK to ux0:/downloads and quit if they do.
	*/
	
	/*
	 * Terminal pimping using CSI sequence "\e[#;#;#X"
	 * where X is the CSI code, #;#;# are the comma separated params
	 * see https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences
	 * Colors = 0:black, 1:red, 2: green, 3:yellow, 4:blue, 5:magenta, 6:cyan, 7:white
	 *
	 * printf("\e[91m"     "A Red text ");            // 3X = set the foreground color to X
	 * printf("\e[30;42m"  "Black text on Green BG ");// 4X = set the background color to X 
	 * printf("\e[39;49m"  "default\n");              // 39/49 = reset FG/BG color 
	 * printf("\e[97m"     "White+ text ");           /* 9X = set bright foreground color (keep green BG)
	 * printf("\e[91;106m" "Red+ text on Cyan+ BG "); /* 10X= set bright background color
	 * printf("\e[m"       "default\n");              /* no param = reset FG/BG
	 */
	
	
	struct stat st = {0};
	bool install_needed = false;
	bool update_needed = false;

	if (stat("ux0:/app/PCSG00141", &st) == -1 && stat("gro0:/app/PCSG00141", &st) == -1) 
	{
	psvDebugScreenPrintf("\e[91m" "Couldn't find PSO2 vita - Are you sure it's installed/inserted?\n");
	psvDebugScreenPrintf("\e[91m" "A critical error has occurred. Please press X to exit the program.");
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
	psvDebugScreenPrintf("\e[91m" "Couldn't find PSO2 vita update data - Are you sure it's updated?\nStart the game and select \"Online Login\" to force an update check. (ux0:/patch/PCSG00141)\n");
	psvDebugScreenPrintf("\e[91m" "A critical error has occurred. Please press X to exit the program.");
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
	
	netInit();
	httpInit();
	
	printf("Checking for a new version of the PSO2v Tweaker...");
	
	//Zero out the remote file
	WriteFile("ux0:/data/PSO2vTweaker/vpk_ver_remote.txt","",sizeof(""));
	
	download("http://arks-layer.com/vita/vpk_ver_remote.txt", "ux0:data/PSO2vTweaker/vpk_ver_remote.txt");
	
	int ver_info_size = getFileSize("ux0:data/PSO2vTweaker/vpk_ver_remote.txt");
	char *ver_info = malloc(1024);
	memset(ver_info, 0, 1024);
	ver_info[1024] = 0x00;
	ReadFile("ux0:data/PSO2vTweaker/vpk_ver_remote.txt",ver_info,ver_info_size);
	
	if(strcmp(ver_info,vpk_internal_ver) != 0)
	{
		psvDebugScreenPrintf("\e[92m" "\rA new version of the PSO2v Tweaker is available on the website! (");
		psvDebugScreenPrintf(ver_info);
		psvDebugScreenPrintf(")\nWould you like to download it? Press X to download or R to ignore." "\e[39;49m" "\n");	
		while (1) {
		SceCtrlData pad;
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if (pad.buttons & SCE_CTRL_CROSS)
		{		
			char *vpkname;
				
			vpkname = concat(3,"ux0:/download/pso2v_tweaker_",ver_info,".vpk");
				
			//If there's no download folder, make it.
			if (stat("ux0:/download", &st) == -1) 
			{
				psvDebugScreenPrintf("Creating app directory...\n");
				sceIoMkdir("ux0:/download", 0777);
			}
				
			download("http://arks-layer.com/vita/pso2v_tweaker.vpk",vpkname);
			psvDebugScreenPrintf("Download complete! VPK saved as %s.\n",vpkname);
			psvDebugScreenPrintf("\e[91m" "Please install the VPK listed above to update the PSO2v Tweaker. Press X to exit.");
			
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
		else if (pad.buttons & (SCE_CTRL_RTRIGGER | SCE_CTRL_R1))
		{
				break;
		}
		}
	}
	else
	{
		psvDebugScreenPrintf("\e[39;49m" "\r");	
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
	
	psvDebugScreenPrintf("This will check for/update the PSO2 vita English patch to the newest version available.\nIf this program fails to patch/update for some reason, you can download the patch from http://arks-layer.com/.\n\n");
		
	psvDebugScreenPrintf("\e[91m" "!!!Please make sure that you have the rePatch plugin installed and enabled!!!\n\n" "\e[39;49m");

	psvDebugScreenPrintf("Checking for a new version of the patch... ");
	WriteFile("ux0:data/PSO2vTweaker/release_url.txt","",sizeof(""));
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
	WriteFile("ux0:data/PSO2vTweaker/release.txt","",sizeof(""));
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
	
	psvDebugScreenPrintf("Done!\n");
	psvDebugScreenPrintf("The latest patch was created on %s.\n",releaseinfo);
	
	if(strcmp(releaseinfo_old,"1/1/2091") == 0) 
	{
	psvDebugScreenPrintf("You don't appear to have ever installed the English patch before on this system (using PSO2v Tweaker).\n");
	install_needed = true;
	}
	else
	{
		if(strcmp(releaseinfo,releaseinfo_old) == 0) 
		{
			if (!FileExists(str_output)) 
			{
				install_needed = true;
			}
			else
			{
				psvDebugScreenPrintf("You have the latest version of the English patch!\n");
			}
		}
		else
		{
		//psvDebugScreenPrintf("The patch you have installed appears to be from ");
		//psvDebugScreenPrintf(releaseinfo_old);
		//psvDebugScreenPrintf(".\n");
		install_needed = true;
		update_needed = true;
		}	
	}
	
	if(install_needed == true) 
	{
		if (!FileExists(str_output)) 
		{
			if (update_needed == false)
			{
			if(strcmp(releaseinfo_old,"1/1/2091") != 0) 
			{
			psvDebugScreenPrintf("\e[91m" "Unable to locate the English patch file! Would you like to re-install it?\nPress X for yes, O for no.\n" "\e[39;49m");
			}
			else
			{
				psvDebugScreenPrintf("You don't appear to have ever installed the English patch before on this system (using PSO2v Tweaker).\nWould you like to install it?\nPress X for yes, O for no.\n");
			}
			}
			else
			{
				psvDebugScreenPrintf("Would you like to install/update the English patch? Press X for yes, O for no.\n");
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
		//psvDebugScreenPrintf("Downloading patch, please wait.....\n");
		
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
		
		downloadpatch(releaseinfo_url,str_output);
		
		sceKernelDelayThread(10000);
		
		SceUID fdsrc = sceIoOpen("ux0:data/PSO2vTweaker/release.txt", SCE_O_RDONLY, 0);

		int fdsize = sceIoLseek(fdsrc, 0, SEEK_END);
		sceIoLseek(fdsrc, 0, SEEK_SET);
		void *buf = malloc(fdsize);
		sceIoRead(fdsrc, buf, fdsize);
		sceIoClose(fdsrc);

		//Zero out the release_old file
		WriteFile("ux0:/data/PSO2vTweaker/release_old.txt","",sizeof(""));
		
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
	
	sceKernelDelayThread(20000);
	free(releaseinfo_url);
	free(releaseinfo);
	free(releaseinfo_old);
	psvDebugScreenPrintf("\e[36m" "Press X to launch the game or press [] to quit.");
	while (1) {
		SceCtrlData pad;
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if (pad.buttons & SCE_CTRL_SQUARE)
		{
		sceKernelExitProcess(0);
		return 0;		
		}
		
		if (pad.buttons & SCE_CTRL_CROSS)
		{
		//Launch the game
		launchAppByUriExit("PCSG00141");
		break;
		}
		sceKernelDelayThread(10000);
	}
}
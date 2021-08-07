#include "console_check.h"

bool check_parent_is_console(void)
{
#ifdef _WIN32
	HANDLE th32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	HANDLE parent;
	IMAGE_DOS_HEADER *dos_hdr;
	IMAGE_NT_HEADERS32 *nt32_hdrs = nullptr;
	IMAGE_NT_HEADERS64 *nt64_hdrs = nullptr;
	SIZE_T parent_hdrs_size;
	PROCESSENTRY32W ent;
	char parent_img_name[MAX_PATH];
	HMODULE parent_base_addr = 0;
	HMODULE *parent_modules;
	unsigned long parent_module_count;
	char tmp[512];

	/* Check if we have something graphical as our parent process */
	ent.dwSize = sizeof(PROCESSENTRY32W);
	if (!Process32FirstW(th32, &ent))
		return false;

	/* Find the parent process */
	while (Process32NextW(th32, &ent)) {
		if (ent.th32ProcessID == GetCurrentProcessId())
			break;
	}

	/* Open the parent process */
	parent = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
			     ent.th32ParentProcessID);
	if (!parent)
		return false;

	/* Get the image name for later */
	K32GetModuleFileNameExA(parent, NULL, parent_img_name, MAX_PATH);

	/* Get the list of loaded modules */
	parent_modules = new HMODULE[1024];
	K32EnumProcessModules(parent, parent_modules, 512 * sizeof(HMODULE),
			      &parent_module_count);
	if (parent_module_count / sizeof(HMODULE) > 1024) {
		delete[] parent_modules;
		return true;
	}

	/* Find the executable */
	for (size_t i = 0; i < parent_module_count / sizeof(HMODULE); i++) {
		/* Get the name of the current module */
		K32GetModuleFileNameExA(parent, parent_modules[i], tmp, 512);

		/* Check the name against the one from before */
		if (strcasecmp(parent_img_name, tmp) == 0) {
			parent_base_addr = parent_modules[i];
			break;
		}
	}

	/* Read the header of the image */
	ReadProcessMemory(parent, parent_base_addr, tmp, 512,
			  &parent_hdrs_size);
	if (!parent_hdrs_size)
		return false; /* Windows can clean this up, it's only a couple unimportant kilobytes */

	/* Parse the header */
	dos_hdr = (IMAGE_DOS_HEADER *)tmp;
	if (dos_hdr->e_magic != IMAGE_DOS_SIGNATURE)
		return false;
	if (*(unsigned short *)(tmp + dos_hdr->e_lfanew +
				sizeof(unsigned long)) ==
	    IMAGE_FILE_MACHINE_I386) {
		/* Get the optional and file headers */
		nt32_hdrs = (IMAGE_NT_HEADERS32 *)((unsigned char *)tmp +
						   (dos_hdr->e_lfanew));
		if (nt32_hdrs->Signature != IMAGE_NT_SIGNATURE ||
		    nt32_hdrs->OptionalHeader.Magic !=
			    IMAGE_NT_OPTIONAL_HDR32_MAGIC)
			return false;

		/* Check the subsystem */
		if (nt32_hdrs->OptionalHeader.Subsystem ==
		    IMAGE_SUBSYSTEM_WINDOWS_GUI) {
			return true;
		}
	} else {
		/* Get the optional and file headers */
		nt64_hdrs = (IMAGE_NT_HEADERS64 *)((unsigned char *)tmp +
						   (dos_hdr->e_lfanew));
		if (nt64_hdrs->Signature != IMAGE_NT_SIGNATURE ||
		    nt64_hdrs->OptionalHeader.Magic !=
			    IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			return false;

		/* Check the subsystem */
		if (nt64_hdrs->OptionalHeader.Subsystem ==
		    IMAGE_SUBSYSTEM_WINDOWS_GUI) {
			return true;
		}
	}
#endif

	return false;
}

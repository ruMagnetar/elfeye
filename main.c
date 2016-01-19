
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defines ----------------------------------------------------------------- */
#define NSYMBONLINE 16

/* Data structs ------------------------------------------------------------ */
typedef enum
{
	CMD_UNKNOWN,
	CMD_DUMP,
	CMD_PARSE,
	CMD_HELP
} T_COMMAND;


typedef struct
{
    unsigned char *content;
    long size;
} T_MEMFILE;


typedef struct
{
	unsigned char e_ident[4];
} T_ELFHEADER;


typedef struct
{
	T_ELFHEADER header;
} T_ELFFILE;

/* Prototypes -------------------------------------------------------------- */
void printHelp(void);
T_COMMAND getCommand(char *scmd);
T_MEMFILE *loadFile(char *filename);
void closeFile(T_MEMFILE *file);
void dumpFile(T_MEMFILE *file, int n);
void parseFile(T_MEMFILE *file);
/* ----------------------------------------------------------------------------
 * Entry point
 */
int main(int argc, char **argv)
{
    // no command for the program
    if(argc == 1)
    {
		printHelp();
        return 1;
    }
    else if(argc >= 2)
	{
		//printf("debug: %d\n", __LINE__);
		T_COMMAND cmd = getCommand(argv[1]);
		//printf("debug: %d\n", cmd);
		
		T_MEMFILE *memfile = NULL;
		switch(cmd)
		{
			case CMD_HELP:
				printHelp();
				break;
				
			case CMD_DUMP:
				// no file for the dump
				if(argc == 2)
				{
					printf("No input file\n");
					printHelp();
					return 1;
				}
				
				// load file in memory
				memfile = loadFile(argv[2]);
				if(memfile == NULL || memfile->content == NULL)
				{
					printf("Cannot read input file\n");
					return 1;
				}
					
				// get amount of bytes for print
				int bytes = -1;
				if(argc == 4)
				{
					bytes = atoi(argv[3]);
				}
				
				// dump content of the file
				dumpFile(memfile, bytes);
				
				closeFile(memfile);
				break;
			
			case CMD_PARSE:
				// load file in memory
				memfile = loadFile(argv[2]);
				if(memfile == NULL || memfile->content == NULL)
				{
					printf("Cannot read input file\n");
					return 1;
				}
				
				// parse content of the file
				parseFile(memfile);
				
				closeFile(memfile);
				
				break;
			
			case CMD_UNKNOWN: // as default
			default:
				printf("%s is unknown command\n", argv[1]);
				printHelp();
				return 1;
		}
	}

    return 0;
}

/* ----------------------------------------------------------------------------
 * Parse the file
 */
void parseFile(T_MEMFILE *file)
{
	if(file == NULL)
	{
		printf("printFile() error\n");
		return;
	}
	
	T_ELFFILE *elf = (T_ELFFILE *) file->content;
	unsigned char elf_shtamp[] = {0x7f, 0x45, 0x4c, 0x46};
	unsigned char *ch1 = (unsigned char *) elf;
	unsigned char *ch2 = elf_shtamp;
	
	// check that the file is in ELF format
	if( strncmp((unsigned char *)elf->header.e_ident, elf_shtamp, 4) == 0)
	{
		printf("ELF file detected\n");
	}
	else
	{
		printf("This is not ELF file format\n");
	}
}

/* ----------------------------------------------------------------------------
 * Detect command
 */
T_COMMAND getCommand(char *scmd)
{
	if(scmd == NULL)
	{
		printf("getCommand error, arg is NULL");
		return CMD_UNKNOWN;
	}
	
	//printf("debug: getCommand took \"%s\"\n", scmd);
	if( !strcmp(scmd, "-h"))     return CMD_HELP;
	if( !strcmp(scmd, "--help")) return CMD_HELP;
	if( !strcmp(scmd, "dump"))   return CMD_DUMP;
	if( !strcmp(scmd, "parse"))  return CMD_PARSE;
	//printf("debug: %d\n", __LINE__);
	return CMD_UNKNOWN;
}

/* ----------------------------------------------------------------------------
 * Attempt to load file in memory
 */
void dumpFile(T_MEMFILE *file, int par_bytes)
{
	//printf("debug: requested = %d; filesize = %d", par_bytes, file->size);
	
	if(file == NULL)
    {
        printf("printFile() error\n");
        return;
    }
    
    // how many bytes to dump
    int bytes = par_bytes;
	if(bytes == -1 || bytes > file->size)
	{
		// whole file
		bytes = file->size;
	}
	for(int i = 0; i < bytes; i++) // FIXEME: i <= bytes ?
    {
		// before each line to print address
        if(i % NSYMBONLINE == 0)
        {
            printf("\n0x%08x | ", i);
        }
        else if(i % 8 == 0) // split group of 8 bytes by space
		{
			printf(" ");
		}
		
		// print byte
		printf("%02x ", file->content[i]);
    }
    printf("\n");
	if(bytes != file->size)
	{
		printf("... and so on ...\n");
	}
}

/* ----------------------------------------------------------------------------
 * Attempt to load file in memory
 */
T_MEMFILE *loadFile(char *filename)
{
    FILE *elffile = NULL;
    unsigned char *memfile = NULL;
	T_MEMFILE *pmf = NULL;
	long filesize = 0;
	size_t readed = 0;

    // opening specified file
    elffile = fopen(filename, "r");
    if(elffile == NULL)
    {
        printf("Don't see file with such name\n");
        return NULL;
    }
    
    // obtain file size
    fseek(elffile, 0, SEEK_END);
    filesize = ftell(elffile);
    rewind(elffile);

    // allocate memory to contain the whole file
    memfile = (unsigned char *) malloc(sizeof(unsigned char) * filesize);
    if(memfile == NULL)
    {
        printf("Memalloc error for file loading\n");
        return NULL;
    }

    pmf = (T_MEMFILE *) malloc(sizeof(T_MEMFILE));
    if(pmf == NULL)
    {
        printf("Memalloc error for file loading\n");
        return NULL;
    }
    
    // load file's content in memory and form data structure for it
    readed = fread(memfile, 1, filesize, elffile);
    pmf->content = memfile;
    pmf->size    = filesize;
	
	fclose(elffile);
    
    return pmf;
}

/* ----------------------------------------------------------------------------
 * Free memory of file
 */
void closeFile(T_MEMFILE *file)
{
	free(file->content);
	free(file);
}

/* ----------------------------------------------------------------------------
 * Print help
 */
void printHelp(void)
{
	printf("Usage:\n");
	printf("        elfparser [-h|--help] - this message\n");
	printf("        elfparser print <file> <n> - dump first n bytes of the file\n");
	printf("                                     (or whole, if where is no <n>)\n");
	printf("        elfparser parse <file> - print data of the file in ELF format\n");
}

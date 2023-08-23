//============================================================
//
//  droiddir.c - core directory access functions
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  MAME4DROID MAME4iOS by David Valdeita (Seleuco)
//
//============================================================

#include "osdcore.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "osdcore.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#include "myosd.h"

struct _osd_directory
{
	osd_directory_entry ent;

	struct dirent *data;

	DIR *fd;
	
	int safDirId;
	
	char buff[64];
};


static osd_dir_entry_type get_attributes_stat(const char *file)
{
	struct stat st;
	if(stat(file, &st))
		return(osd_dir_entry_type)0;

	if (S_ISDIR(st.st_mode)) return ENTTYPE_DIR;

	return ENTTYPE_FILE;
}


static UINT64 osd_get_file_size(const char *file)
{

	struct stat st;
	if(stat(file, &st))
		return 0;

	return st.st_size;
}

//============================================================
//  osd_opendir
//============================================================

osd_directory *osd_opendir(const char *dirname)
{
	osd_directory *dir = NULL;
	
	//__android_log_print(ANDROID_LOG_INFO, "mame4", "Leido directorio %s",dirname);

	dir = (osd_directory *) malloc(sizeof(osd_directory));
	if (dir)
	{
		memset(dir, 0, sizeof(osd_directory));
		dir->fd = NULL;
		dir->safDirId = 0;
	}
	else
	    return NULL;
	
	if(myosd_using_saf == 1 && strcmp(myosd_rompath,dirname) == 0)
	{
		//__android_log_print(ANDROID_LOG_INFO, "mame4", "USANDO SAF readdir....");
		dir->safDirId = myosd_safReadDir((char*)dirname, myosd_reload);		          
		//__android_log_print(ANDROID_LOG_INFO, "mame4", "myosd_safReadDir %d", dir->safDirId );
		if(!dir->safDirId)
		{
			free(dir);
			dir = NULL;
		}
		else
		{
			myosd_reload = 0;
		}
	}
	else
	{
	   dir->fd = opendir(dirname);
	   
	   if (dir && (dir->fd == NULL))
	   {
		  free(dir);
		  dir = NULL;
	   }
	}

	return dir;
}


//============================================================
//  osd_readdir
//============================================================

const osd_directory_entry *osd_readdir(osd_directory *dir)
{
	 //__android_log_print(ANDROID_LOG_INFO, "mame4", "osd_readdir");

    if(dir->safDirId)
	{	   
	   char *entryName = myosd_safGetNextDirEntry(dir->safDirId);
		
	   if(entryName==NULL)			
		   return NULL;
		
	   if(strlen(entryName)>64)//safety
	       entryName[63]=0;
		
	   strcpy(dir->buff,entryName);
	   free(entryName);
	   
	   dir->ent.name = dir->buff;
	   dir->ent.type = ENTTYPE_FILE;
	   dir->ent.size = 0;

	   //__android_log_print(ANDROID_LOG_INFO, "mame4", "osd_readdir %s",dir->name);	  	   
	}
	else
	{
	   dir->data = readdir(dir->fd);
	   
	   if (dir->data == NULL)
		  return NULL;

	   dir->ent.name = dir->data->d_name;
	   dir->ent.type = get_attributes_stat(dir->data->d_name);
	   dir->ent.size = osd_get_file_size(dir->data->d_name);
	}
	

	return &dir->ent;
}


//============================================================
//  osd_closedir
//============================================================

void osd_closedir(osd_directory *dir)
{
	  //__android_log_print(ANDROID_LOG_INFO, "mame4", "osd_closedir");
     
	 if(dir->safDirId)
	 {
	     myosd_safCloseDir(dir->safDirId);
	 }
	 else
	 {
	     if (dir->fd != NULL)
		     closedir(dir->fd);
	 }
	
	free(dir);
}

/*=================================================================================================
#                Copyright (c) 2015 Teradyne
#                Unpublished - All Rights Reserved
#  =================================================================================================
#      Filename        :           izn_ug_export_import_assy.h
#      Module          :           izn_ug_export_import_assy
#      Project         :
#      Author          :           Vikash B
#  =================================================================================================
#  Date                                  Name                               Description of Change
#  22-March-2022                       Vikash B                   	             Initial Code
#  =================================================================================================*/

//==============================================================
//C++ Header File.
//==============================================================
#include <iostream>
#include <time.h>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <direct.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fclasses/tc_date.h>
#include <algorithm>
#include <map>
#include <filesystem>
#include <vector>
#include <list>

//=================================================================
//TC Header File. 
//=================================================================
extern "C"
{
#include <tc/tc.h>
#include <tccore/tctype.h>
#include <tccore/aom_prop.h>
#include <tccore/aom.h>
#include <epm/cr.h>
#include <fclasses/tc_string.h>
#include <fclasses/tc_date.h>
#include <tccore/grm.h>
#include <tccore/workspaceobject.h>
#include <tc/emh.h>
#include <pom/pom/pom.h>
#include <property/propdesc.h>
#include <form/formtype.h>
#include <tccore/item.h>
#include <tc/preferences.h>
#include <lov/lov.h>
#include <res/res_itk.h>
#include <sa/user.h>
#include <pom/enq/enq.h>
#include <serviceprocessing/serviceprocessing.h>
#include <user_exits/epm_toolkit_utils.h>
#include <tc/emh.h>
#include <sys/timeb.h>
#include <tccore/item.h>
#include <tccore/aom_prop.h>
#include <ae/datasettype.h>
#include <ae/dataset.h>
#include <tccore/grm.h>
#include <sa/tcfile.h>
#include <ae/dataset.h>
#include <user_exits/epm_toolkit_utils.h>
#include <res/res_itk.h>
#include <sa/tcfile_cache.h>
#include <tc/folder.h>
#include <sa/user.h>
}
using namespace std;
#define SAFE_MEM_free(x)     \
  MEM_free(x);               \
  x = NULL;

#define SAFE_MEM_FREE1( a )   \
do                          \
{                           \
    if ( (a) != NULL )      \
    {                       \
        MEM_free( (a) );    \
        (a) = NULL;         \
    }                       \
}                           \
while ( 0 )                      \
 
#define TIAUTO_ITKCALL(x) {           \
	int stat;                     \
	char *err_string;             \
	if( (stat = (x)) != ITK_ok)   \
	{                             \
	fprintf (logFilefp, "ERROR: %d ERROR MSG: %s.\n", stat, err_string);           \
	fprintf (logFilefp, "FUNCTION: %s\nFILE: %s LINE: %d\n\n",#x, __FILE__, __LINE__); \
	if(err_string) MEM_free(err_string);                                  \
	}                                                                    \
}




//Errors Msg.
#define IZN_TC_LOGGEDIN_STATUS      "\n TC Login success \n"
#define IZN_ERR_TC_LOGIN_FAIL        "\n Login failed \n"
#define IZN_SUCCESS_STATUS          "\n Completed successfully \n"


//Function Declarations
int util_display_help();
int assy_display_usage(void);
int export_and_import_assy(map<string, string>itemReport, map<string, string> mapAssyParts, string sClonFilePath, string sBatchFilePath, string sTopAssy);
string izn_local_time(string sDateFormat);
string izn_mkdir(char* sFilePath);
void ug_export_assy(string sBatchFilePath, string sClonFilePath, string sTopAssy);
void replace_values(string sFileName, string sOldValue, string sNewValue);
string izn_file_path(string sPath);
//string izn_dataset_file_path(string sPath);
string toknize(string sPartRev, string sDB);
void izn_replace_part(string sPath);
void izn_replace_clone_name(string sPath);
void izn_replace_ass_dir(string sPath);
void izn_replace_def_dir(string sClonFilePath, string sPath);
void izn_general_fun(string sPath, map<string, string> mapDatasetLoc);
//void izn_replace_dwg_prt(string sPath);
void izn_replace_wit_new_id(map<string, string> itemReport, string sPath, string sNewPartWithRev);
void ug_import_assy(string sBatchFilePath, string sClonFilePath);
string izn_log_file_path(string sPath);
tag_t createBuyPart(char* objectname);
tag_t createElectronicComp(char* objectname, char* cAttriComp);
tag_t createSoftwareComp(char* objectname);
tag_t createEngProd(char* objectname);
tag_t createEngineeringPart(char* objectname, char* cAttriValue);
void replace_part_type(string sFileName);
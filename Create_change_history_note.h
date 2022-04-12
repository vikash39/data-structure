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
#include <filesystem>
//=================================================================
//TC Header File. 
//=================================================================
#include <tc/tc.h>
#include <sys/timeb.h>
#include <tccore/item.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <ae/datasettype.h>
#include <ae/dataset.h>
#include <tccore/grm.h>
#include <sa/tcfile.h>
#include <ae/dataset.h>
#include <user_exits/epm_toolkit_utils.h>
#include <res/res_itk.h>
#include <sa/tcfile_cache.h>

#define TERADYNE_CALL_RETURN(x) {  \
  int stat;                     \
  char *err_string;             \
  if( (stat = (x)) != ITK_ok)   \
  {                             \
    EMH_ask_error_text (stat, &err_string); \
    TC_write_syslog ("ERROR: %d ERROR MSG: %s \n", stat, err_string); \
    TC_write_syslog ("Function: %s FILE: %s LINE: %d \n",#x, __FILE__, __LINE__); \
    MEM_free (err_string); \
    return (stat); \
  } \
 }


#define TERADYNE_GRM_RELATION_TYPE         "TD4RefMaterialRel"//"IMAN_specification"  TD4RefMaterialRel
#define TERADYNE_DATASET_DESC              "Change History Note"
#define TERADYNE_DATASET_TYPE              "Text"
#define TERADYNE_OBJECT_NAME               "object_name"
#define TERADYNE_CLOSE_INPUT_FILE          "\n file closed \n"
#define TERADYNE_ESI_NOTES_INFO           "This is the ESI, Notes information that was migrated in as part of the ESI to Oracle transition project, "

//Errors Msg.
#define TERADYNE_ERR_PART_NOT_FOUND       "TERA::Error: The Part number is not found in tc "
#define TERADYNE_ERR_DATASET_NOT_CREATED  "TERA::Error: Dataset is not attached with the part number "
#define TERADYNE_ERR_DELETE_TXT_FILE      "\n Error deleting file \n"
#define TERADYNE_ERR_OPEN_INPUT_FILE      "\n\n Failed to open Input file \n\n"
#define TERADYNE_ERR_IMF_IMPORTING_FILE   "\n Error While IMF importing file \n"
#define TERADYNE_ERR_TC_LOGIN_FAIL        "\n Login failed \n"
#define TERADYNE_ERR_DIR_NOT_CREATED      "\n Tc Change history note directories are not created \n"
#define TERADYNE_ERR_INPUT_ARGS           "TERA::Error: User Id, Password, Group & File name should not be empty. Please provide valid inputs.\n"
#define TERADYNE_ERR_PART_NUM_CHECKOUT    "TERA::Error: The Part number is checked out , "
#define TERADYNE_ERR_DATASET_CHECKOUT     "TERA::Error: The dataset is checked out , "
#define TERADYNE_ERR_MISSING_CSV_FILE     "\nPlease provide .csv file in input. "
#define TERADYNE_ERR_DATASET_NOT_FOUND    "TERA::Error: Dataset is not found in TC. "

//Success Msg.
#define TERADYNE_PATH_EXIST              "\n Change history notes dataset & logs file path is exist \n"
#define TERADYNE_TC_LOGGEDIN_STATUS      "\n TC Login success \n"
#define TERADYNE_SUCCESS_STATUS          "\n---- Create change history note is successfully completed ----"

using namespace std;

int create_change_history_note(string sItemid, string sPath);
int save_object(tag_t object_tag);
void teradyne_writeToLogFile(string sErrorMsg, string sLogFilePath);
void teradyne_mkdir(char* sLogsFilePath);
bool IsPathExist(const std::string &s);
int teradyne_display_usage(void);
int teradyne_display_help();
string teradyne_log_local_time(string sDateFormat);
tag_t teradyne_IsDataSetExist(tag_t tPartLatestRev, string sDataSetName);
int teradyne_find_and_replace_namedRef(const char* cDatasetRefName, char* sReferenceName, tag_t tagDataset, tag_t tTargetObject);
int teradyne_find_dataset_and_update_namedRef(string sPartId, string sDatasetFilePath);
void teradyne_delete_dataset(string sPartId, string sDatasetFilePath);
void teradyne_get_input_dir(string sDatasetFilePath);
string teradyne_dataset_file_path(string sPath);
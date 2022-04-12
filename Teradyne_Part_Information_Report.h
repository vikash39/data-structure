/*=================================================================================================
#                Copyright (c) 2015 Teradyne
#                Unpublished - All Rights Reserved
#  =================================================================================================
#      Filename        :           Teradyne_Part_Information_Report.h
#      Module          :           Teradyne_Part_Information_Report
#      Project         :           Teradyne_Part_Information_Report
#      Author          :           Vikash B
#  =================================================================================================
#  Date                                  Name                               Description of Change
#  16-August-2021                       Vikash B                   	             Initial Code
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
#include <tc/folder.h>

#define TERADYNE_ERROR_CALL(x) { \
	int stat; \
	char *err_string = NULL; \
	if( (stat = (x)) != ITK_ok) \
{ \
	EMH_ask_error_text (stat, &err_string); \
	if(err_string != NULL) {\
	TC_write_syslog ("ERROR: %d ERROR MSG: %s \n", stat, err_string); \
	MEM_free (err_string);\
	err_string=NULL;\
	}\
} \
}
//TC_write_syslog ("Function: %s FILE: %s LINE: %d \n",#x, __FILE__, __LINE__); \
//TC_write_syslog("Teradyne_Part_Information_Report Error [%d]: %s\n\t(FILE: %s, LINE:%d)\n", stat, err_string, __FILE__, __LINE__);\

 //Errors Msg.
#define TERADYNE_TC_LOGGEDIN_STATUS      "\n TC Login success \n"
#define TERADYNE_ERR_OPEN_INPUT_FILE      "\n\n Failed to open Input file \n\n"
#define TERADYNE_ERR_TC_LOGIN_FAIL        "\n Login failed \n"
#define TERADYNE_ERR_MISSING_CSV_FILE     "\nPlease provide .csv file in input. "
#define TERADYNE_ERR_INPUT_ARGS           "TERA::Error: User Id, Password, Group & File name should not be empty. Please provide valid inputs.\n"
#define TERADYNE_ERR_LOG_FILE_PATH        "\nPlease provide the logs file path.\n"

//Success Msg.
#define TERADYNE_PATH_EXIST              "\n Tc part information report logs file path is exist \n"
#define TERADYNE_SUCCESS_STATUS          "\n---- Create part information report is successfully completed ----"

//Object Properties.
#define TER_ITEM_ID                       "item_id"
#define TER_OBJECT_TYPE                   "object_type"
#define TER_COMM_PART_DESCRIPTION         "td4Description"
#define TER_DIV_PART_DESCRIPTION          "td4StandardDescription"
#define TER_COMMPART_REVISION             "TD4CommPart"
#define TER_DIV_PART_REV                  "TD4DivPart"
#define TER_CTRL_BUSS_UNIT                "td4ControllingBussUnit"
#define TER_ITEM_STATUS                   "td4ItemStatus"
#define TER_PROJECT_NAME                  "td4ProjectName"
#define TER_COUNTRY_OF_ORIGIN             "td4_Country_of_Origin"
#define TER_MATLCOST                      "td4MatlCost"
#define TER_DISCIPLINE_SPECIFIC_REL       "TD4DisciplineSpecificRel"
#define TER_CHG_ADMIN_FORM                "TD4ChgAdminForm"
#define TER_TERADYNE_REV_STAMP            "td4TeradyneRevStamp"
#define TER_MINSHIP_REV                   "td4MinShipRev"
#define TER_ENGINEERING_CONTROLLED        "td4EngineeringControlled"
#define TER_TRADE_COMPL_FORM              "TD4TradeComplForm"
#define TER_ECCN                          "td4ECCN"
#define TER_HTS                           "td4HTS"
#define TER_SAFETY_FORM                   "TD4SafetyForm"
#define TER_SAFETY_LICENSE                "td4SafetyLicense"
#define TER_VENDOR_PARTS                  "vendorparts"
#define TER_MFG_PART                      "TD4MfgPart"
#define TER_VENDOR_PART_NUM               "vendor_part_num"
#define TER_VENDOR_NAME                   "vendor_name"
#define TER_COUNTRY_OF_ORIGIN_LIST       "td4_Country_of_Origin_List"
#define TER_RELEASE_STATUS_LIST          "release_status_list"
#define TER_PREFREED_STATUS              "preferred_status"
#define TER_OBJECT_NAME					"object_name"

#define TER_BINARY_FORMATTYPE			"BINARY"
#define TER_TEXT_FORMATTYPE				"TEXT"
#define TER_MSEXCEL_DATASETTYPE			"MSExcel"
#define TER_TEXT_DATASETTYPE			"Text"
#define TER_MSEXCEL_REFERENCE_NAME		"excel"
#define TER_TEXT_REFERNCE_NAME			"Text"
#define TER_GENERAL_QUERY				 "General..."
#define TER_TYPE_INPUT					"Type"
#define TER_NAME_INPUT                   "Name"
#define TER_FOLDER_TYPE					"Folder"
#define TER_DATASET_PART_INFO_RPT		"Part_Information_Report"
#define TER_MSEXCEL_FILE_EXTN			"xls"
#define TER_TEXT_FILE_EXTN				"txt"

using namespace std;

int teradyne_get_part_info(string sPartid, char *cLogFilePath, int iHedCnt, string sOutputFileName, int *iIsLoged);
void teradyne_write_output_report(const char* cpPartNumber, const char* cpPartType, const char* cpDescription, const char* cpControllingBusinessUnit, string sEngineeringControlled, const char* cpItemStatus, const char* cpRevStamp,
	const char* cpMinimumShippableRevision, const char* cpProjectName, double dItemCost, const char* cpCountryofOrigin, const char* cpExportControlClassificationNumber, const char* cpHarmonizedTariffSchedule,
	const char* cpTeradyneSystemSafetyLicense,string cpVendorName, string cpVendorPartNumber, string sCountryofOriginList, ofstream *myfile, int iHedCnt);
int teradyne_display_usage(void);
int teradyne_display_help();
void teradyne_mkdir(char* sLogsFilePath);
void teradyne_delete_exist_file(char* cPath, string sOutputFileName);
void teradyne_delete_files(char *cLogFilePath, string sOutputFileName);
void teradyne_logs(string sErrorMsg, string sLogFilePath, string sOutputFileName);
string teradyne_dataset_file_path(string sPath);
string teradyne_log_local_time(string sDateFormat);
void teradyne_problem_parts(string sPartid, ofstream *myfile, int iHedCnt);
tag_t teradyne_get_released_rev(tag_t tItemTag);
int teradyne_find_and_create_folder(const char *cFolderName, const char *cLogFilePath, string sOutputFileName, int *iIsLoged);
int teradyne_create_dataset(const char *cDatasetName, const char *cDatasetDesc, const char *cDatasetType, const char *cFormatName, const char *cDatasetRefName, const char *cReferenceName, AE_reference_type_t ref_type, tag_t *tagNew_dataset);
int teradyne_find_execute_qry(const char *pcQueryName, std::map<string, string> strQryEntriesMap, int* count, tag_t** ptLatestRev);
string teradyne_create_dataset_name(string sOutputfile);
int teradyne_create_folder(const char *cFolderName, tag_t *tFolderTag);
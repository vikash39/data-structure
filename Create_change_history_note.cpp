/*=================================================================================================
#                Copyright (c) 2015 Teradyne
#                Unpublished - All Rights Reserved
#  =================================================================================================
#      Filename        :           CreateChangeHistoryNote.cpp
#      Module          :           CreateChangeHistoryNote.exe
#      Project         :           CreateChangeHistoryNote
#      Author          :           Vikash
#  =================================================================================================
#  Date                              Name                               Description of Change
#  17-may-2021                       Vikash                   	        Initial Code
#  =================================================================================================*/

#include "Create_change_history_note.h"

using namespace std;


int ITK_user_main(int argc, char* argv[])
{
	int iFail = ITK_ok,
		iCnt = 0,
		iLine = 1,
		iColumn = 0,
		iCunt = 0,
		iCrMkdir = 0;

	char *cUserName = NULL,
		*cPassWord = NULL,
		*cUsergroup = NULL,
		*cInputFile = NULL,
		*cLogsFilePath = NULL,
		**cValues = NULL;

	string sline = "",
		sItemId = "";

	fstream   finput_file;

	try {

		cUserName = ITK_ask_cli_argument("-u=");

		cPassWord = ITK_ask_cli_argument("-p=");

		cUsergroup = ITK_ask_cli_argument("-g=");

		cInputFile = ITK_ask_cli_argument("-file_or_mode=");

		cLogsFilePath = ITK_ask_cli_argument("-logs_path=");

		/*------DISPLAY HELP--------*/
		if (ITK_ask_cli_argument("-help") || ITK_ask_cli_argument("-h")) {

			teradyne_display_help();
			return iFail;
		}

		string sDatasetFilePath = cLogsFilePath;

		if (strcmp(cInputFile, "Dataset") != 0) {

			if (strcmp(strrchr(cInputFile, '\0') - 4, ".csv")) {
				cout << TERADYNE_ERR_MISSING_CSV_FILE << endl;
				return -1;
			}
		}

		if ((cUserName == NULL) || (cPassWord == NULL) || (cUsergroup == NULL) || (cInputFile == NULL))
		{
			teradyne_writeToLogFile(TERADYNE_ERR_INPUT_ARGS, sDatasetFilePath);
			teradyne_display_usage();
			return -1;
		}

		if (ITK_init_module(cUserName, cPassWord, cUsergroup) != ITK_ok)
		{
			printf(TERADYNE_ERR_TC_LOGIN_FAIL);
			TC_write_syslog(TERADYNE_ERR_TC_LOGIN_FAIL);

			exit(1);
		}

		
		TC_write_syslog(TERADYNE_TC_LOGGEDIN_STATUS);
		ITK_set_bypass(true);

		if (strcmp(cInputFile, "Dataset") == 0) {

			teradyne_get_input_dir(sDatasetFilePath);
		}
		else {
			teradyne_mkdir(cLogsFilePath);
			finput_file.open(cInputFile);

			if (!(finput_file.is_open()))
			{
				TC_write_syslog(TERADYNE_ERR_OPEN_INPUT_FILE);
				exit(1);
			}

			while (getline(finput_file, sline, '$')) {

				if (iColumn == iCunt) {

					if (!sline.empty()) {

						sline.resize(sline.size() - 1);
						sline.erase(sline.begin() + 0);
						ofstream file;
						string sDataSetName = "ESI Change History " + sItemId + ".txt";
						string sTempfilepath = sDatasetFilePath + "/dataset\\\\";
						string sPath = teradyne_dataset_file_path(sTempfilepath);
						file.open(sPath + sDataSetName);
						string sCurrDate = teradyne_log_local_time("noteDate");
						file << TERADYNE_ESI_NOTES_INFO + sCurrDate + "\n\n";
						file << sline;
						file.close();

						if (sItemId != "") {

							iFail = create_change_history_note(sItemId, sDatasetFilePath);

							if (iFail == ITK_ok) {

								teradyne_delete_dataset(sItemId, sDatasetFilePath);
							}

							else if (iFail == 1) {
								string sErrorMsg = TERADYNE_ERR_PART_NOT_FOUND + sItemId;
								teradyne_writeToLogFile(sErrorMsg, sDatasetFilePath);
							}
							else if (iFail == 2) {
								string sErrorMsg = TERADYNE_ERR_PART_NUM_CHECKOUT + sItemId;
								teradyne_writeToLogFile(sErrorMsg, sDatasetFilePath);
							}
							else {
								string sErrorMsg = TERADYNE_ERR_DATASET_NOT_CREATED + sItemId;
								teradyne_writeToLogFile(sErrorMsg, sDatasetFilePath);
							}
						}
					}
					iCunt++;
				}

				else {

					if (!sline.empty()) {
						sline.resize(sline.size() - 1);
						sline.erase(sline.begin() + 0);

						if (sline != "") {
							sItemId = sline;
						}
						iColumn++;

					}
				}
			}

			finput_file.close();
			TC_write_syslog(TERADYNE_CLOSE_INPUT_FILE);

		}

	}

	catch (...) {

		TC_write_syslog("\n ##### In catch block for error values \n");

	}
	cout << TERADYNE_SUCCESS_STATUS << endl;
	ITK_set_bypass(false);
	ITK_exit_module(true);
	return ITK_ok;
}

/*****************************************************************************************************
 * Function Name			: create_change_history_note
 * Description			    : find the part latest revision, check that part rev. have dataset or not, if doesn't getting datset
 *                            then it'll create dataset & attache the dataset using namereference.
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : input - part number.
 *							  msg(O)        -  Error Msg for the given istatus.
 * RETURN VALUE				: int : 0/error code
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/

int create_change_history_note(string sItemid, string sPath)
{
	int iFail = ITK_ok,
		iCount = 0,
		iFound = 0;

	tag_t tPartId = NULLTAG,
		tPartLatestRev = NULLTAG,
		tDatasettype = NULLTAG,
		tNewDataset = NULLTAG,
		tRelation = NULLTAG,
		tNewfile = NULLTAG,
		tGrmRelation = NULLTAG;

	tag_t default_tool_tag;

	char *cItemId = NULL;
	char *cItemRev = NULL;
	char **list = NULL;
	const char *cDatasetName = NULL;

	logical isCheckOut = false;

	IMF_file_t  fileDescriptor;

	AE_reference_type_t reference_type;


	try
	{
		TERADYNE_CALL_RETURN(iFail = ITK_set_bypass(true));
		TERADYNE_CALL_RETURN(iFail = ITEM_find_item(sItemid.c_str(), &tPartId));

		string sDatasetPath = sPath+"/dataset/";
		string sDataSetName = "ESI Change History " + sItemid + ".txt";
		string sCrDataSetName = "ESI Change History " + sItemid;

		if (tPartId != NULLTAG) {

			TERADYNE_CALL_RETURN(iFail = ITEM_ask_latest_rev(tPartId, &tPartLatestRev));
			TERADYNE_CALL_RETURN(iFail = RES_is_checked_out(tPartLatestRev, &isCheckOut));

			if (isCheckOut == true) {
				return 2;
			}

			TERADYNE_CALL_RETURN(iFail = AE_find_datasettype2(TERADYNE_DATASET_TYPE, &tDatasettype));

			//the set of tools that can be used with a particular datasettype
			TERADYNE_CALL_RETURN(iFail = AE_ask_datasettype_def_tool(tDatasettype, &default_tool_tag));
			TERADYNE_CALL_RETURN(iFail = AE_ask_datasettype_refs(tDatasettype, &iCount, &list));

			tNewDataset = teradyne_IsDataSetExist(tPartLatestRev, sCrDataSetName);
			if (tNewDataset == NULLTAG) {

				TERADYNE_CALL_RETURN(iFail = AE_create_dataset_with_id(tDatasettype, sCrDataSetName.c_str(), TERADYNE_DATASET_DESC, 0, 0, &tNewDataset));
				TERADYNE_CALL_RETURN(iFail = AE_set_dataset_tool(tNewDataset, default_tool_tag));
			}
			else {
				string  sDatasetName = sDatasetPath + sDataSetName;
				cDatasetName = sDatasetName.c_str();
				TERADYNE_CALL_RETURN(iFail = RES_is_checked_out(tNewDataset, &isCheckOut));
				if (isCheckOut == true) {
					teradyne_writeToLogFile(TERADYNE_ERR_DATASET_CHECKOUT + sItemid, sPath);
					return 0;
				}
				teradyne_find_and_replace_namedRef(cDatasetName, list[0], tNewDataset, tPartLatestRev);

				return 0;
			}
			string  sDatasetName = sDatasetPath + sDataSetName;
			cDatasetName = sDatasetName.c_str();

			if (IMF_import_file(cDatasetName, NULL, SS_TEXT, &tNewfile, &fileDescriptor) != ITK_ok)
				TC_write_syslog(TERADYNE_ERR_IMF_IMPORTING_FILE);

			TERADYNE_CALL_RETURN(iFail = save_object(tNewfile));

			IMF_close_file(fileDescriptor);
			reference_type = AE_PART_OF;

			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tPartLatestRev, true));
			TERADYNE_CALL_RETURN(iFail = GRM_find_relation_type(TERADYNE_GRM_RELATION_TYPE, &tRelation));
			TERADYNE_CALL_RETURN(iFail = AE_add_dataset_named_ref2(tNewDataset, list[0], reference_type, tNewfile));
			TERADYNE_CALL_RETURN(iFail = save_object(tNewDataset));
			TERADYNE_CALL_RETURN(iFail = GRM_create_relation(tPartLatestRev, tNewDataset, tRelation, NULLTAG, &tGrmRelation));
			TERADYNE_CALL_RETURN(iFail = GRM_save_relation(tGrmRelation));
			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tPartLatestRev, false));
			TERADYNE_CALL_RETURN(iFail = ITK_set_bypass(false));
		}
		else {
			return iFail = 1;
		}
	}
	catch (...)
	{
	}

	if (list != NULL)
		MEM_free(list);

	return  iFail;
}

/*******************************************************************************
 * Function Name			: save_object
 * Description				: save the created object in tc.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				: int : 0/error code
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 ******************************************************************************/

int save_object(tag_t object_tag)
{
	int ReturnCode;
	/* Save the object. */
	ReturnCode = AOM_save(object_tag);
	if (ReturnCode != ITK_ok)
	{
		return (ReturnCode);
	}
	/* Unlock the object. */

	ReturnCode = AOM_unlock(object_tag);
	return (ReturnCode);
}

/*******************************************************************************
 * Function Name			: teradyne_writeToLogFile
 * Description				: write the error log files.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : string msg.
 * RETURN VALUE				: void
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 ******************************************************************************/

void teradyne_writeToLogFile(string sErrorMsg, string sLogFilePath) {

	string sLogPath = sLogFilePath + "/tc_change_history_logs_notes/Change_History_Note_logs_";
	string slogFilPath = teradyne_dataset_file_path(sLogPath);
	string sDateTime = teradyne_log_local_time("");
	ofstream  log_file(slogFilPath + sDateTime + ".txt", std::ios_base::out | std::ios_base::app);
	log_file << sErrorMsg + "\n";
}

/*******************************************************************************
 * Function Name			: teradyne_mkdir
 * Description				: making the dataset & log files directories.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				: void
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 ******************************************************************************/

void teradyne_mkdir(char* sLogsFilePath) {
	bool bExist = false;

	try {
		string sDataset = sLogsFilePath;
		string dataset = sDataset + "/dataset";
		string stclogs = sDataset+"/tc_change_history_logs_notes";

		mkdir(sLogsFilePath);
		mkdir(dataset.c_str());
		mkdir(stclogs.c_str());
	}
	catch (...) {}
}

/*******************************************************************************
 * Function Name			: IsPathExist
 * Description				: Check dataset & logs file paths exist or not
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				: bool : true/false
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 ******************************************************************************/

bool IsPathExist(const std::string &s)
{
	struct stat buffer;
	return (stat(s.c_str(), &buffer) == 0);
}


/*******************************************************************************
 * Function Name			: teradyne_display_help
 * Description				: Displays help message for the utility
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				: int : 0/error code
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 ******************************************************************************/
int teradyne_display_help()
{
	cout << endl;
	cout << "Usage:" << endl;
	cout << "Change_history_notes";
	cout << " -u=<Userid>";
	cout << " -p=<Password>";
	cout << " -p=<Group>";
	cout << " -file=<Filename> ";
	cout << "[-help]" << endl;
	cout << "Where" << endl;
	cout << "-u         - TC user ID should be \"admin\"." << endl;
	cout << "-p         - TC password." << endl;
	cout << "-g         - TC group." << endl;
	cout << "-file      - Full path with file name. File should be \".csv\"" << endl;

	return 0;
}

/*****************************************************************************************************
 * Function Name			: teradyne_display_usage
 * Description			    : Prints the console output while giving wrong input.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				:
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/

int teradyne_display_usage(void)
{
	int iFail = ITK_ok;
	cout << "********************** Input_Command_Usage **********************\n";
	cout << "Error: Input command line arguments are less than expected!" << endl;
	cout << "[-u= username -p=password -g=group]\n";
	cout << "[-file = Full path with input file name(input should be .csv file) ]" << endl;
	cout << "********************** Input_Command_Usage **********************\n";
	return iFail;
}


/*****************************************************************************************************
 * Function Name			: teradyne_log_local_time
 * Description			    : Prints the Local time to the logfile
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				:
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/
string teradyne_log_local_time(string sDateFormat)
{
	struct timeb timebuffer;
	ftime(&timebuffer);

	time_t tt = timebuffer.time;
	struct tm *ptm = localtime(&tt);
	date_t curDate;
	curDate.year = ptm->tm_year + 1900;
	curDate.month = ptm->tm_mon;
	curDate.day = ptm->tm_mday;
	curDate.hour = ptm->tm_hour;
	curDate.minute = ptm->tm_min;
	curDate.second = ptm->tm_sec;

	int msec = timebuffer.millitm;

	char *strDate = NULL;
	char *strDateReturn = NULL;

	if (tc_strcmp(sDateFormat.c_str(), "noteDate") == 0) {
		DATE_date_to_string(curDate, "%d-%m-%Y", &strDate);
		return strDate;
	}
	else
		DATE_date_to_string(curDate, "%Y%m%d%H", &strDateReturn);


	TC_write_syslog("\n%s.%03d", strDate, msec);
	return strDateReturn;
}

/*****************************************************************************************************
 * Function Name			: teradyne_IsDataSetExist
 * Description			    : check dataset exist or not.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				:
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/

tag_t teradyne_IsDataSetExist(tag_t tPartLatestRev, string sDataSetName) {

	int iFail = ITK_ok,
		iCunt = 0;

	tag_t tRelation = NULLTAG,
		*tSecondaryObjects = NULLTAG;

	char *cpDataSetName = NULL;

	try {

		TERADYNE_CALL_RETURN(iFail = GRM_find_relation_type(TERADYNE_GRM_RELATION_TYPE, &tRelation));

		TERADYNE_CALL_RETURN(iFail = GRM_list_secondary_objects_only(tPartLatestRev, tRelation, &iCunt, &tSecondaryObjects));

		for (int iCnt = 0; iCnt < iCunt; iCnt++) {

			TERADYNE_CALL_RETURN(iFail = AOM_ask_value_string(tSecondaryObjects[iCnt], TERADYNE_OBJECT_NAME, &cpDataSetName));

			if (tc_strcasecmp(cpDataSetName, sDataSetName.c_str()) == 0) {
				return tSecondaryObjects[iCnt];
			}
		}
	}
	catch (...) {}

	return NULLTAG;
}

/*****************************************************************************************************
 * Function Name			: teradyne_find_and_replace_namedRef
 * Description			    : if dataset is already created then update the namereference only.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : cDatasetRefName, cReferenceName, tagDataset, tTargetObject
 * RETURN VALUE				:int : 0/error code
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/

int teradyne_find_and_replace_namedRef(const char* cDatasetRefName, char* cReferenceName, tag_t tagDataset, tag_t tTargetObject)
{
	int   iFail = ITK_ok;

	tag_t tagFileTag = NULLTAG;
	tag_t tagReferencedObj = NULLTAG;

	IMF_file_t file_desc;

	AE_reference_type_t RefType;


	try
	{
		TERADYNE_CALL_RETURN(iFail = IMF_import_file(cDatasetRefName, NULL, SS_TEXT, &tagFileTag, &file_desc));

		if (tagFileTag != NULLTAG)
		{
			TERADYNE_CALL_RETURN(iFail = ITK_set_bypass(true));
			TERADYNE_CALL_RETURN(iFail = IMF_close_file(file_desc));

			TERADYNE_CALL_RETURN(iFail = AOM_save_without_extensions(tagFileTag));

			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tTargetObject, true));

			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tagDataset, true));

			TERADYNE_CALL_RETURN(iFail = AE_ask_dataset_named_ref2(tagDataset, cReferenceName, &RefType, &tagReferencedObj));

			TERADYNE_CALL_RETURN(iFail = AE_replace_dataset_named_ref2(tagDataset, tagReferencedObj, cReferenceName, AE_PART_OF, tagFileTag));

			TERADYNE_CALL_RETURN(iFail = AOM_save_without_extensions(tagDataset));

			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tagDataset, false));

			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tTargetObject, false));
			TERADYNE_CALL_RETURN(iFail = ITK_set_bypass(false));
		}
	}
	catch (...)
	{
		AOM_refresh(tagDataset, false);
		ITK_set_bypass(false);
	}

	return iFail;
}



int teradyne_find_dataset_and_update_namedRef(string sPartId, string sDatasetFilePath) {

	int iFail = ITK_ok,
		iCount = 0;

	tag_t tDataset = NULLTAG,
		tPartId = NULLTAG,
		tPartLatestRev = NULLTAG,
		tDatasetType = NULLTAG,
		default_tool_tag = NULLTAG,
		tDatasetExist = NULLTAG,
		tNewfile = NULLTAG,
		tRelation = NULLTAG,
		tGrmRelation = NULLTAG,
		tReferencedObj = NULLTAG;

	logical isCheckOut = false;

	IMF_file_t  fileDescriptor;
	AE_reference_type_t reference_type;

	char  **cList = NULL;

	const char *cDatasetName = NULL;

	string sDataSetName = "ESI Change History " + sPartId;

	TERADYNE_CALL_RETURN(iFail = ITK_set_bypass(true));

	TERADYNE_CALL_RETURN(iFail = AE_find_dataset2(sDataSetName.c_str(), &tDataset));

	if (tDataset != NULLTAG) {

		string sDatasetPath = sDatasetFilePath + "/dataset/";
		string sNameRefDataset = "ESI Change History " + sPartId + ".txt";
		string  sDatasetName = sDatasetPath + sNameRefDataset;
		cDatasetName = sDatasetName.c_str();

		TERADYNE_CALL_RETURN(iFail = ITEM_find_item(sPartId.c_str(), &tPartId));

		if (tPartId != NULLTAG) {

			TERADYNE_CALL_RETURN(iFail = ITEM_ask_latest_rev(tPartId, &tPartLatestRev));
			TERADYNE_CALL_RETURN(iFail = RES_is_checked_out(tPartLatestRev, &isCheckOut));

			if (isCheckOut == true) {
				teradyne_writeToLogFile(TERADYNE_ERR_PART_NUM_CHECKOUT + sPartId, sDatasetFilePath);
				return 0;
			}

			TERADYNE_CALL_RETURN(iFail = AE_find_datasettype2(TERADYNE_DATASET_TYPE, &tDatasetType));

			//the set of tools that can be used with a particular datasettype
			TERADYNE_CALL_RETURN(iFail = AE_ask_datasettype_def_tool(tDatasetType, &default_tool_tag));
			TERADYNE_CALL_RETURN(iFail = AE_ask_datasettype_refs(tDatasetType, &iCount, &cList));

			tDatasetExist = teradyne_IsDataSetExist(tPartLatestRev, sDataSetName);

			if (tDatasetExist != NULLTAG) {

				teradyne_find_and_replace_namedRef(cDatasetName, cList[0], tDataset, tPartLatestRev);
				teradyne_delete_dataset(sPartId, sDatasetFilePath);

				return 0;
			}
			if (IMF_import_file(cDatasetName, NULL, SS_TEXT, &tNewfile, &fileDescriptor) != ITK_ok)
				TC_write_syslog(TERADYNE_ERR_IMF_IMPORTING_FILE);

			IMF_close_file(fileDescriptor);
			reference_type = AE_PART_OF;
			TERADYNE_CALL_RETURN(iFail = AOM_save(tNewfile));
			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tDataset, true));

			TERADYNE_CALL_RETURN(iFail = AE_ask_dataset_named_ref2(tDataset, cList[0], &reference_type, &tReferencedObj));
			if (tReferencedObj != NULLTAG) {
				teradyne_find_and_replace_namedRef(cDatasetName, cList[0], tDataset, tPartLatestRev);
				teradyne_delete_dataset(sPartId, sDatasetFilePath);
			}
			else {
				TERADYNE_CALL_RETURN(iFail = AE_add_dataset_named_ref2(tDataset, cList[0], reference_type, tNewfile));
			}
			TERADYNE_CALL_RETURN(iFail = AOM_save(tDataset));
			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tDataset, false));
			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tPartLatestRev, true));

			TERADYNE_CALL_RETURN(iFail = GRM_find_relation_type(TERADYNE_GRM_RELATION_TYPE, &tRelation));
			TERADYNE_CALL_RETURN(iFail = GRM_create_relation(tPartLatestRev, tDataset, tRelation, NULLTAG, &tGrmRelation));
			TERADYNE_CALL_RETURN(iFail = GRM_save_relation(tGrmRelation));
			TERADYNE_CALL_RETURN(iFail = AOM_refresh(tPartLatestRev, false));
			TERADYNE_CALL_RETURN(iFail = ITK_set_bypass(false));

			teradyne_delete_dataset(sPartId, sDatasetFilePath);
		}
		else {
			teradyne_writeToLogFile(TERADYNE_ERR_PART_NOT_FOUND + sPartId, sDatasetFilePath);
			return 0;
		}

		return 0;
	}
	else {
		iFail = create_change_history_note(sPartId, sDatasetFilePath);

		if (iFail == ITK_ok) {
			teradyne_delete_dataset(sPartId, sDatasetFilePath);
		}
		else if (iFail == 1) {
			string sErrorMsg = TERADYNE_ERR_PART_NOT_FOUND + sPartId;
			teradyne_writeToLogFile(sErrorMsg, sDatasetFilePath);
		}
		else {
			string sErrorMsg = TERADYNE_ERR_DATASET_NOT_CREATED + sPartId;
			teradyne_writeToLogFile(sErrorMsg, sDatasetFilePath);
		}	
	}

	return iFail;
}



void teradyne_delete_dataset(string sPartId, string sDatasetFilePath)
{
	string sDataSetN = "ESI Change History " + sPartId + ".txt";
	string sTempfilepath = sDatasetFilePath + "/dataset\\\\";
	string sPath = teradyne_dataset_file_path(sTempfilepath);
	string  filename = sPath + sDataSetN;

	const char* datasetName = filename.c_str();
	if (remove(datasetName) != 0)
		TC_write_syslog(TERADYNE_ERR_DELETE_TXT_FILE);
}




void teradyne_get_input_dir(string sDatasetFilePath) {

	namespace fs = std::filesystem;
	std::string path = sDatasetFilePath+"/dataset/";
	int length = path.size();

	for (const auto & entry : fs::directory_iterator(path)) {
		string sPartId(entry.path().string());
		sPartId.erase(0, length);
		sPartId.erase(0, 19);
		sPartId.erase(sPartId.size() - 4);
		teradyne_find_dataset_and_update_namedRef(sPartId, sDatasetFilePath);
	}

}


string teradyne_dataset_file_path(string sPath) {
	size_t found = 0, next = 0;
	while ((found = sPath.find('/', next)) != std::string::npos)
	{
		sPath.insert(found, "\\\\");
		next = found + 4;
	}
	sPath.erase(remove(sPath.begin(), sPath.end(), '/'), sPath.end());

	return sPath;
}
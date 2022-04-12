/*=================================================================================================
#                Copyright (c) 2015 Teradyne
#                Unpublished - All Rights Reserved
#  =================================================================================================
#      Filename        :           Teradyne_Part_Information_Report.cpp
#      Module          :           Teradyne_Part_Information_Report
#      Project         :           Teradyne_Part_Information_Report
#      Author          :           Vikash B
#  =================================================================================================
#  Date                                  Name                               Description of Change
#  16-August-2021                       Vikash B                   	             Initial Code
#  =================================================================================================*/

#include "Teradyne_Part_Information_Report.h"

using namespace std;

int ITK_user_main(int argc, char* argv[])
{

	int iFail = ITK_ok,
		iCnt = 0,
		iLine = 1,
		iColumn = 0,
		iCunt = 0,
		iCrMkdir = 0,
		iHedCnt = 0;
	int iIsLoged = 0;

	char *cUserName = NULL,
		*cPassWord = NULL,
		*cUsergroup = NULL,
		*cLogFilePath = NULL,
		**cValues = NULL;

	string sPartId = "",
		sItemId = "",
		sPartNum = "",
		sOutputFileName = "";


	fstream   finput_file;

	try {

		cUserName = ITK_ask_cli_argument("-u=");

		cPassWord = ITK_ask_cli_argument("-p=");

		cUsergroup = ITK_ask_cli_argument("-g=");

		cLogFilePath = ITK_ask_cli_argument("-inputpath=");

		/*------DISPLAY HELP--------*/
		if (ITK_ask_cli_argument("-help") || ITK_ask_cli_argument("-h")) {

			teradyne_display_help();
			return iFail;
		}

		if (cLogFilePath != NULL) {
			teradyne_mkdir(cLogFilePath);
		}

		if ((cUserName == NULL) || (cPassWord == NULL) || (cUsergroup == NULL) || (cLogFilePath == NULL))
		{
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
		cout << TERADYNE_TC_LOGGEDIN_STATUS << endl;
		string filepath = cLogFilePath;

		namespace fs = std::filesystem;
		std::string path = filepath;

		string sDateTime = teradyne_log_local_time("");

		for (const auto & entry : fs::directory_iterator(path))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) == "csv")
			{
				string soutputfile = sFileName.substr((sFileName.find_last_of("\\") + 1));
				soutputfile.erase(soutputfile.size() - 4);
				finput_file.open(sFileName);

				sOutputFileName = teradyne_create_dataset_name(soutputfile);

				if (!(finput_file.is_open()))
				{
					TC_write_syslog(TERADYNE_ERR_OPEN_INPUT_FILE);
					exit(1);
				}
				teradyne_get_part_info(sPartNum, cLogFilePath, iHedCnt, sOutputFileName, &iIsLoged);
				if (iHedCnt == 0)
					iHedCnt++;

				int indx = 0;

				while (getline(finput_file, sPartNum, '$')) {

					if (indx != 0)
						sPartNum.erase(sPartNum.begin() + 0);
					sPartNum.resize(sPartNum.size() - 1);
					indx++;
					
					teradyne_get_part_info(sPartNum, cLogFilePath, iHedCnt, sOutputFileName, &iIsLoged);
					//if (iHedCnt == 0)
						//iHedCnt++;
				}

				finput_file.close();
				iHedCnt = 0;
				teradyne_find_and_create_folder(TER_DATASET_PART_INFO_RPT, cLogFilePath, sOutputFileName, &iIsLoged);
				teradyne_delete_files(cLogFilePath, sOutputFileName);
			}

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) == "csv")
			{
				string sInputDir = cLogFilePath;
				string sPath = sInputDir + "/";
				string sTargPath = sPath + sDateTime;

				fs::path sourceFile = sFileName;
				fs::path targetParent = sTargPath;
				auto target = targetParent / sourceFile.filename();


				try
				{
					fs::create_directories(targetParent);
					fs::copy_file(sourceFile, target, fs::copy_options::overwrite_existing);
					std::filesystem::remove(sourceFile);
				}
				catch (std::exception& e)
				{
					//TC_write_syslog("\n");
					//TC_write_syslog(e.what());
				}
			}
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
 * Function Name			: teradyne_get_part_info
 * Description			    : Get the part information.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : sPartid, iHedCnt
 * RETURN VALUE				:int : 0/error code
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/
int teradyne_get_part_info(string sPartid, char *cLogFilePath, int iHedCnt, string sOutputFileName, int *iIsLoged)
{
	int iFail = ITK_ok,
		iCunt = 0,
		iVendorCnt = 0,
		iCunOrgList = 0,
		iChgAdminCunt = 0,
		iReleaseListCunt = 0,
		iListAllRev = 0;

	tag_t tItemTag = NULLTAG,
		tRelationTag = NULLTAG,
		*tVendorpartsTag = NULLTAG,
		tDisSpecificRelTag = NULLTAG,
		*tChgAdminFormTag = NULLTAG,
		*tRevListTag = NULLTAG,
		*tReleaseStatListTag = NULLTAG,
		tVMRepresentsRelObj = NULLTAG;

	tag_t tVMRelationTag = NULLTAG;

	char *cpPartNumber = NULL,
		*cpPartyType = NULL,
		*cpDescription = NULL,
		*cpVendors = NULL,
		*cpControllingBusinessUnit = NULL,
		*cpItemStatus = NULL,
		*cpProjectName = NULL,
		*cpVendorPartNum = NULL,
		*cpVendorName = NULL,
		*cObjectType = NULL,
		*cpRevStamp = NULL,
		*cpMinimumShippableRevision = NULL,
		*cpCountryofOrigin = NULL,
		*cpExpCntrlClassificationNum = NULL,
		*cpHarTariffSchedule = NULL,
		*cpTeraSysSafetyLicense = NULL,
		**cpCountryofOriginList = NULL,
		*cpChgAdminObjType = NULL,
		**cpRevList = NULL,
		*cpStatusName = NULL;

	double dItemCost = 0;

	string sCountryofOriginList = "",
		sEngineeringControlled = "False",
		sVendorPartNum = "",
		sVendorName = "";

	logical logicalEngineeringControlled = false;
	std::ofstream myfile;
	string sOutputFilePath = teradyne_dataset_file_path(cLogFilePath);
	sOutputFilePath = sOutputFilePath + "//tc_part_info_report//";
	sOutputFilePath = sOutputFilePath + sOutputFileName + "." + TER_MSEXCEL_FILE_EXTN;
	myfile.open(sOutputFilePath, std::ios::out | std::ios::app);

	try
	{
		TERADYNE_ERROR_CALL(iFail = ITK_set_bypass(true));
		TERADYNE_ERROR_CALL(iFail = ITEM_find_item(sPartid.c_str(), &tItemTag));
		
		if (tItemTag != NULLTAG)
		{
			tag_t tLatestRevTag = NULLTAG;
			tLatestRevTag = teradyne_get_released_rev(tItemTag);
			//TERADYNE_ERROR_CALL(iFail = ITEM_ask_latest_rev(tItemTag, &tLatestRevTag)); // only changes 03-09-2021

			if (tLatestRevTag != NULLTAG)
			{
				TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_ITEM_ID, &cpPartNumber));
				TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tItemTag, TER_OBJECT_TYPE, &cpPartyType));

				if (tc_strcmp(cpPartyType, TER_DIV_PART_REV) == 0)
				{
					TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_DIV_PART_DESCRIPTION, &cpDescription));
				}

				if (tc_strcmp(cpPartyType, TER_COMMPART_REVISION) == 0)
				{
					TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_CTRL_BUSS_UNIT, &cpControllingBusinessUnit));
					TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_COMM_PART_DESCRIPTION, &cpDescription));
				}

				TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_ITEM_STATUS, &cpItemStatus));
				TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_PROJECT_NAME, &cpProjectName));
				TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tLatestRevTag, TER_COUNTRY_OF_ORIGIN, &cpCountryofOrigin));
				TERADYNE_ERROR_CALL(iFail = AOM_ask_value_double(tLatestRevTag, TER_MATLCOST, &dItemCost));

				TERADYNE_ERROR_CALL(iFail = GRM_find_relation_type(TER_DISCIPLINE_SPECIFIC_REL, &tDisSpecificRelTag));
				TERADYNE_ERROR_CALL(iFail = GRM_list_secondary_objects_only(tLatestRevTag, tDisSpecificRelTag, &iChgAdminCunt, &tChgAdminFormTag));

				if (iChgAdminCunt != 0)
				{
					for (int jCnt = 0; jCnt < iChgAdminCunt; jCnt++)
					{
						TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_OBJECT_TYPE, &cpChgAdminObjType));

						if (tc_strcmp(cpChgAdminObjType, TER_CHG_ADMIN_FORM) == 0)
						{
							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_TERADYNE_REV_STAMP, &cpRevStamp));
							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_MINSHIP_REV, &cpMinimumShippableRevision));
							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_logical(tChgAdminFormTag[jCnt], TER_ENGINEERING_CONTROLLED, &logicalEngineeringControlled));

							if (logicalEngineeringControlled == 1)
								sEngineeringControlled = "True";

							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_CTRL_BUSS_UNIT, &cpControllingBusinessUnit));
						}

						if (tc_strcmp(cpChgAdminObjType, TER_TRADE_COMPL_FORM) == 0)
						{
							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_ECCN, &cpExpCntrlClassificationNum));
							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_HTS, &cpHarTariffSchedule));
						}

						if (tc_strcmp(cpChgAdminObjType, TER_SAFETY_FORM) == 0)
						{
							TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tChgAdminFormTag[jCnt], TER_SAFETY_LICENSE, &cpTeraSysSafetyLicense));
						}
					}
				}

				TERADYNE_ERROR_CALL(iFail = GRM_find_relation_type(TER_VENDOR_PARTS, &tRelationTag));
				TERADYNE_ERROR_CALL(iFail = GRM_list_secondary_objects_only(tLatestRevTag, tRelationTag, &iCunt, &tVendorpartsTag));

				if (iCunt != 0)
				{
					for (int iCnt = 0; iCnt < iCunt; iCnt++)
					{
						TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tVendorpartsTag[iCnt], TER_OBJECT_TYPE, &cObjectType));

						if (tc_strcmp(cObjectType, TER_MFG_PART) == 0)
						{

							TERADYNE_ERROR_CALL(iFail = GRM_find_relation_type("VMRepresents", &tVMRelationTag));
							iFail = GRM_find_relation(tLatestRevTag, tVendorpartsTag[iCnt], tVMRelationTag, &tVMRepresentsRelObj);
							iFail = AOM_ask_value_string(tVMRepresentsRelObj, TER_PREFREED_STATUS, &cpStatusName);

							if (tc_strcmp(cpStatusName, "Removed") != 0)
							{
								TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tVendorpartsTag[iCnt], TER_VENDOR_PART_NUM, &cpVendorPartNum));
								TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tVendorpartsTag[iCnt], TER_VENDOR_NAME, &cpVendorName));
								TERADYNE_ERROR_CALL(iFail = AOM_ask_value_strings(tVendorpartsTag[iCnt], TER_COUNTRY_OF_ORIGIN_LIST, &iCunOrgList, &cpCountryofOriginList));

								if (iCunOrgList != 0)
								{

									for (int iCt = 0; iCt < iCunOrgList; iCt++)
									{
										sCountryofOriginList = sCountryofOriginList + cpCountryofOriginList[iCt];
										sCountryofOriginList = sCountryofOriginList + ",\n";
									}
									sCountryofOriginList = sCountryofOriginList.substr(0, sCountryofOriginList.size() - 3);
								}
	
								sVendorPartNum = cpVendorPartNum;
								sVendorName = cpVendorName;

									teradyne_write_output_report(cpPartNumber, cpPartyType, cpDescription, cpControllingBusinessUnit, sEngineeringControlled, cpItemStatus, cpRevStamp,
										cpMinimumShippableRevision, cpProjectName, dItemCost, cpCountryofOrigin, cpExpCntrlClassificationNum, cpHarTariffSchedule,
										cpTeraSysSafetyLicense, sVendorName, sVendorPartNum, sCountryofOriginList, &myfile, iHedCnt);

								iVendorCnt++;
								iHedCnt++;
								sCountryofOriginList = "";
								if (cpCountryofOriginList != NULL)
									MEM_free(cpCountryofOriginList);
							}
						}
					}
				}

				if (iVendorCnt == 0) {


					teradyne_write_output_report(cpPartNumber, cpPartyType, cpDescription, cpControllingBusinessUnit, sEngineeringControlled, cpItemStatus, cpRevStamp,
						cpMinimumShippableRevision, cpProjectName, dItemCost, cpCountryofOrigin, cpExpCntrlClassificationNum, cpHarTariffSchedule,
						cpTeraSysSafetyLicense, sVendorName, sVendorPartNum, sCountryofOriginList, &myfile, iHedCnt);
					iHedCnt++;
				}
			}
		}
		else {

			teradyne_problem_parts(sPartid, &myfile, iHedCnt);
			string sErrorMsg = "Part number '" + sPartid + "' not found in tc.";
			if (sPartid != "")
			{
				teradyne_logs(sErrorMsg, cLogFilePath, sOutputFileName);
				*iIsLoged = 1;
			}

		}
		myfile.close();
	}
	catch (...) {}

	if (tChgAdminFormTag != NULLTAG)
		MEM_free(tChgAdminFormTag);
	if (tVendorpartsTag != NULLTAG)
		MEM_free(tVendorpartsTag);

	TERADYNE_ERROR_CALL(iFail = ITK_set_bypass(false));
	return iFail;
}



/*****************************************************************************************************
 * Function Name			: teradyne_write_output_report
 * Description			    : Writing the parts information in .csv file.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : Required output parts information.
 * RETURN VALUE				:void : 0/error code
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					: char* cpPartType, char* cpDescription
 *
 *******************************************************************************************************/
void teradyne_write_output_report(const char* cpPartNumber, const char* cpPartType, const char* cpDescription, const char* cpControllingBusinessUnit, string sEngineeringControlled, const char* cpItemStatus, const char* cpRevStamp,
	const char* cpMinimumShippableRevision, const char* cpProjectName, double dItemCost, const char* cpCountryofOrigin, const char* cpExportControlClassificationNumber, const char* cpHarmonizedTariffSchedule,
	const char* cpTeradyneSystemSafetyLicense, string cpVendorName, string cpVendorPartNumber, string sCountryofOriginList, ofstream *myfile, int iHedCnt)
{
	try
	{
		if (iHedCnt == 0)
		{
			*myfile << " <table border=1>\n";
			*myfile << "    <tr>\n";
			*myfile << "  		<th>Part Number</th>\n"
				"		<th>Part Type</th>\n"
				"		<th>Description</th>\n"
				" 		<th>Constrolling Business Unit</th>\n"
				" 		<th>Engineering Controlled </th>\n"
				" 		<th>Item Status</th>\n"
				"		<th>Rev Stamp</th>\n"
				" 		<th>Minumum Shippable Revision</th>\n"
				" 		<th>Project Name</th>\n"
				" 		<th>Item Cost</th>\n"
				"		<th>Country of Origin</th>\n"
				" 		<th>Export Control Classification Number</th>\n"
				" 		<th>Harmonized Tariff Schedule</th>\n"
				" 		<th>Teradyne System Saftey License</th>\n"
				" 		<th>Vendor Name</th>\n"
				" 		<th>Vendor Part Number</th>\n"
				" 		<th>Country of Origin List</th>\n";
			*myfile << "   </tr>\n";

		}
		else
		{
			*myfile << "   <tr>\n";
			//*myfile << "	<td>" << "&nbsp;" << cpPartNumber << "</td>\n";
			*myfile << "	<td>" << " " << cpPartNumber << "</td>\n";
			*myfile << "	<td>" << cpPartType << "</td>\n";
			*myfile << "  	<td>" << cpDescription << "</td>\n";
			*myfile << "  	<td>" << cpControllingBusinessUnit << "</td>\n";
			*myfile << "  	<td>" << sEngineeringControlled << "</td>\n";
			*myfile << "  	<td>" << cpItemStatus << "</td>\n";
			*myfile << "  	<td>" << cpRevStamp << "</td>\n";
			*myfile << "  	<td>" << cpMinimumShippableRevision << "</td>\n";
			*myfile << "	<td>" << cpProjectName << "</td>\n";
			*myfile << "	<td>" << dItemCost << "</td>\n";
			*myfile << "	<td>" << cpCountryofOrigin << "</td>\n";
			*myfile << "	<td>" << cpExportControlClassificationNumber << "</td>\n";
			*myfile << "	<td>" << cpHarmonizedTariffSchedule << "</td>\n";
			*myfile << "	<td>" << cpTeradyneSystemSafetyLicense << "</td>\n";
			*myfile << "	<td>" << cpVendorName << "</td>\n";
			if (cpVendorPartNumber.empty())
			{
				*myfile << "	<td></td>\n";
			}
			else { 
				//*myfile << "	<td>" << "&nbsp;" << cpVendorPartNumber << "</td>\n"; 
				*myfile << "	<td>" << " " << cpVendorPartNumber << "</td>\n";
			}
			*myfile << "	<td>" << sCountryofOriginList << "</td>\n";
			*myfile << "  </tr>\n";
		}

	}
	catch (...) {}

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
	cout << "[-logPath = Provide logs file path" << endl;
	cout << "********************** Input_Command_Usage **********************\n";
	return iFail;
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
	cout << "Teradyne_Part_Information_Report";
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

	try
	{
		string sPartInfo = sLogsFilePath;
		string sPartRep = sPartInfo + "/tc_part_info_report";
		string stclogs = sPartInfo + "/tc_part_info_logs";

		mkdir(sLogsFilePath);
		mkdir(sPartRep.c_str());
		mkdir(stclogs.c_str());
	}
	catch (...) {}
}

/*******************************************************************************
 * Function Name			: teradyne_delete_exist_file
 * Description				: delete the old existing file.
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

void teradyne_delete_exist_file(char* cPath, string sOutputFileName)
{
	try
	{
		string slogPath(cPath);
		string filename = slogPath + "//tc_part_info_report//" + sOutputFileName + "." + TER_MSEXCEL_FILE_EXTN;
		const char* datasetName = filename.c_str();
		if (remove(datasetName) != 0)
			TC_write_syslog("\nNot able to delete the file %s\n", filename);
		//TC_write_syslog(TERADYNE_ERR_DELETE_TXT_FILE);
	}
	catch (...) {}
}

/*******************************************************************************
 * Function Name			: teradyne_delete_files
 * Description				: delete the created files.
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
void teradyne_delete_files(char *cLogFilePath, string sOutputFileName)
{
	try
	{
		string slogPath(cLogFilePath);
		string filename = slogPath + "//tc_part_info_report//" + sOutputFileName + "." + TER_MSEXCEL_FILE_EXTN;
		const char* datasetName = filename.c_str();
		if (remove(datasetName) != 0)
			TC_write_syslog("\nNot able to delete the file %s\n", filename);

		string filename1 = slogPath + "//tc_part_info_logs//" + sOutputFileName + "_log." + TER_TEXT_FILE_EXTN;
		const char* datasetName1 = filename1.c_str();

		if (remove(datasetName1) != 0)
			TC_write_syslog("\nNot able to delete the file %s\n", filename);
	}
	catch (...) {}

}


/*******************************************************************************
 * Function Name			: teradyne_logs
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

void teradyne_logs(string sErrorMsg, string sLogFilePath, string sOutputFileName)
{
	try
	{
		string sFileName = sOutputFileName;
		string slogFilPath = sLogFilePath + "//tc_part_info_logs//";
		string sLogPath = teradyne_dataset_file_path(slogFilPath);
		sLogPath = sLogPath + sFileName + "_log" + "." + TER_TEXT_FILE_EXTN;
		ofstream  log_file(sLogPath, std::ios_base::out | std::ios_base::app);
		log_file << sErrorMsg + "\n";
	}
	catch (...) {}
}

/*****************************************************************************************************
 * Function Name			: teradyne_dataset_file_path
 * Description			    : It's creating the file paths.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : string sPath
 * RETURN VALUE				: string sPath
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/


string teradyne_dataset_file_path(string sPath) {

	size_t found = 0, next = 0;

	try
	{
		while ((found = sPath.find('/', next)) != std::string::npos)
		{
			sPath.insert(found, "\\\\");
			next = found + 4;
		}
		sPath.erase(remove(sPath.begin(), sPath.end(), '/'), sPath.end());
	}
	catch (...) {}

	return sPath;
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
		DATE_date_to_string(curDate, "%Y-%m-%d_%H-%M-%S", &strDateReturn);


	//TC_write_syslog("\n%s.%03d", strDate, msec);
	return strDateReturn;
}


/*****************************************************************************************************
 * Function Name			: teradyne_problem_parts
 * Description			    : The problem parts insert row in .csv file with empty value.
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

void teradyne_problem_parts(string sPartid, ofstream *myfile, int iHedCnt)
{
	try {

		char* cpPartNum = strdup(sPartid.c_str());
		teradyne_write_output_report(cpPartNum, " ", " ", " ", " ", " ", " ", " ", " ", 0, " ", " ", " ", " ", " ", " ", " ", myfile, iHedCnt);
		iHedCnt++;
		if (cpPartNum != NULL)
			free(cpPartNum);
	}
	catch (...) {}
}


/*****************************************************************************************************
 * Function Name			: teradyne_get_released_rev
 * Description			    : This function is checking release status and returning the rev tag.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      : itemTag
 * RETURN VALUE				: tag_t tRevisionTag
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/


tag_t teradyne_get_released_rev(tag_t tItemTag)
{
	int iFail = ITK_ok,
		iReleaseListCunt = 0,
		iRevListCunt = 0,
		iReleaseStatusCount = 0;

	tag_t tRevisionTag = NULLTAG,
		*tReleaseStatListTag = NULLTAG,
		*tRevListTag = NULLTAG,
		*tReleaseStatusList = NULLTAG,
		tCopmLatestRevTag = NULLTAG;

	char* cpPartyType = NULL;

	TERADYNE_ERROR_CALL(iFail = AOM_ask_value_string(tItemTag, TER_OBJECT_TYPE, &cpPartyType));
	if (tc_strcmp(cpPartyType, TER_COMMPART_REVISION) == 0)
	{
		TERADYNE_ERROR_CALL(iFail = ITEM_ask_latest_rev(tItemTag, &tCopmLatestRevTag));
		return tCopmLatestRevTag;
	}

	TERADYNE_ERROR_CALL(iFail = ITEM_ask_latest_rev(tItemTag, &tRevisionTag));
	TERADYNE_ERROR_CALL(iFail = AOM_ask_value_tags(tRevisionTag, TER_RELEASE_STATUS_LIST, &iReleaseListCunt, &tReleaseStatListTag));

	if (iReleaseListCunt == 0)
	{
		TERADYNE_ERROR_CALL(iFail = ITEM_list_all_revs(tItemTag, &iRevListCunt, &tRevListTag));

		for (int i = iRevListCunt; i-- > 0; ) {

			TERADYNE_ERROR_CALL(iFail = AOM_ask_value_tags(tRevListTag[i], TER_RELEASE_STATUS_LIST, &iReleaseStatusCount, &tReleaseStatusList)); //"release_status_list"

			if (iReleaseStatusCount != 0)
			{
				return tRevListTag[i];
			}
		}
		return tRevisionTag;
	}
	else {

		return tRevisionTag;
	}
}


/*******************************************************************************
* Function Name  	: teradyne_create_dataset_name
* Description		: create dataset name with outputfile_timestamp with file extension.
*
* REQUIRED HEADERS	:
* INPUT PARAMS		:
*
* RETURN VALUE		: int iFail - Error Code
* GLOBALS USED		:
* FUNCTIONS CALLED	:
*
* ALGORITHM		    :
*
*
* NOTES			    :
*------------------------------------------------------------------------------*/
string teradyne_create_dataset_name(string sOutputfile)
{
	string sOutputFileName = sOutputfile;
	sOutputFileName = sOutputFileName + "_" + teradyne_log_local_time("");
	return sOutputFileName;
}

/*******************************************************************************
* Function Name  	: teradyne_find_and_create_folder
* Description		: if folder found with input name then create dataset else
*                     create and attach dataset in the newly created folder
*					  with name as inupufile_timestamp.
*
* REQUIRED HEADERS	:
* INPUT PARAMS		:  const char *cFolderName
*					   const char * cLogFilePath
*					   string sOutputFileName
* RETURN VALUE		: int iFail - Error Code
* GLOBALS USED		:
* FUNCTIONS CALLED	:
*
* ALGORITHM		    :
*
*
* NOTES			    :
*------------------------------------------------------------------------------*/
int teradyne_find_and_create_folder(const char *cFolderName, const char * cLogFilePath, string sOutputFileName, int* iIsLoged)
{
	int iFail = ITK_ok;
	int iCount = 0;
	tag_t tFolderTypeTag = NULLTAG;
	tag_t tCreateInputTag = NULLTAG;
	tag_t tFolderTag = NULLTAG;
	tag_t tSubFolderTag = NULLTAG;
	tag_t tDatasetTag = NULLTAG;
	tag_t tDatasetTag1 = NULLTAG;
	string sDatasetName = sOutputFileName;
	string sLogName = sOutputFileName + "_log";
	AE_reference_type_t refType = AE_PART_OF;
	tag_t *tFldrTag = NULLTAG;
	string sLogFilePath = "";


	string sOutputFile = teradyne_dataset_file_path(cLogFilePath);
	sOutputFile = sOutputFile + "//tc_part_info_report//" + sOutputFileName + "." + TER_MSEXCEL_FILE_EXTN;
	sLogFilePath = teradyne_dataset_file_path(cLogFilePath);
	sLogFilePath = sLogFilePath + "//tc_part_info_logs//" + sOutputFileName + "_log" + "." + TER_TEXT_FILE_EXTN;

	const char *cLogRefName = sLogFilePath.c_str();
	const char *cDataRefName = sOutputFile.c_str();

	std::map<string, string> strPropNameValueMap;
	strPropNameValueMap.insert(::make_pair((string)TER_TYPE_INPUT, TER_FOLDER_TYPE));
	strPropNameValueMap.insert(::make_pair((string)TER_NAME_INPUT, TER_DATASET_PART_INFO_RPT));
	char * cFlag = NULL;
	try
	{
		TERADYNE_ERROR_CALL(iFail = teradyne_find_execute_qry(TER_GENERAL_QUERY, strPropNameValueMap, &iCount, &tFldrTag));
		if (iCount > 0)
		{
			TERADYNE_ERROR_CALL(iFail = AOM_ask_name(*tFldrTag, &cFlag));
			if (*tFldrTag != NULLTAG)
			{
				TERADYNE_ERROR_CALL(iFail = teradyne_create_folder(sOutputFileName.c_str(), &tSubFolderTag));

				TERADYNE_ERROR_CALL(iFail = FL_insert(*tFldrTag, tSubFolderTag, 999));

				TERADYNE_ERROR_CALL(iFail = AOM_save(*tFldrTag));

				TERADYNE_ERROR_CALL(iFail = teradyne_create_dataset(sDatasetName.c_str(), NULL, TER_MSEXCEL_DATASETTYPE, TER_BINARY_FORMATTYPE, cDataRefName, TER_MSEXCEL_REFERENCE_NAME, refType, &tDatasetTag));

				TERADYNE_ERROR_CALL(iFail = FL_insert(tSubFolderTag, tDatasetTag, 999));

				TERADYNE_ERROR_CALL(iFail = AOM_save(tSubFolderTag));
				if (*iIsLoged)
				{

					TERADYNE_ERROR_CALL(iFail = teradyne_create_dataset(sLogName.c_str(), NULL, TER_TEXT_DATASETTYPE, TER_TEXT_FORMATTYPE, cLogRefName, TER_TEXT_REFERNCE_NAME, refType, &tDatasetTag1));

					TERADYNE_ERROR_CALL(iFail = FL_insert(tSubFolderTag, tDatasetTag1, 999));

					TERADYNE_ERROR_CALL(iFail = AOM_save(tSubFolderTag));
					*iIsLoged = 0;
				}

			}
		}
		else
		{
			TERADYNE_ERROR_CALL(iFail = teradyne_create_folder(cFolderName, &tFolderTag));

			if (tFolderTag != NULLTAG)
			{
				TERADYNE_ERROR_CALL(iFail = teradyne_create_folder(sOutputFileName.c_str(), &tSubFolderTag));

				TERADYNE_ERROR_CALL(iFail = FL_insert(tFolderTag, tSubFolderTag, 999));

				TERADYNE_ERROR_CALL(iFail = AOM_save(tFolderTag));

				TERADYNE_ERROR_CALL(iFail = teradyne_create_dataset(sDatasetName.c_str(), NULL, TER_MSEXCEL_DATASETTYPE, TER_BINARY_FORMATTYPE, cDataRefName, TER_MSEXCEL_REFERENCE_NAME, refType, &tDatasetTag));

				TERADYNE_ERROR_CALL(iFail = FL_insert(tSubFolderTag, tDatasetTag, 999));

				TERADYNE_ERROR_CALL(iFail = AOM_save(tSubFolderTag));
				if (*iIsLoged)
				{

					TERADYNE_ERROR_CALL(iFail = teradyne_create_dataset(sLogName.c_str(), NULL, TER_TEXT_DATASETTYPE, TER_TEXT_FORMATTYPE, cLogRefName, TER_TEXT_REFERNCE_NAME, refType, &tDatasetTag1));

					TERADYNE_ERROR_CALL(iFail = FL_insert(tSubFolderTag, tDatasetTag1, 999));

					TERADYNE_ERROR_CALL(iFail = AOM_save(tSubFolderTag));

					*iIsLoged = 0;
				}


			}
		}
	}
	catch (...) {}

}
/***************************************************************************************
*   Function Name       : teradyne_create_dataset
*   INPUT/OUTPUT PARAMS : char  *cDatasetName
*                         char  *cDatasetDesc
*                         char  *cDatasetType
*                         const char *cFormatName
*                         char  *cDatasetRefName
*						  const char *cReferenceName
*					      AE_reference_type_t refType
*                         tag_t *tagNew_dataset
*
*   RETURN VALUE        :ITK_ok for success
*                        iStatus value in case of failures
*   GLOBALS USED        :None
*   FUNCTIONS CALLED    :
*
*   ALGORITHM           :
*
*********************************************************************************************/
int teradyne_create_dataset(const char *cDatasetName,
	const char *cDatasetDesc,
	const char *cDatasetType,
	const char *cFormatName,
	const char *cDatasetRefName,
	const char *cReferenceName,
	AE_reference_type_t refType,
	tag_t *tNewDataset

)
{
	//Declaration For  Newly Created DataSet
	int   iFail = ITK_ok;

	tag_t tagDefaultToolTag = NULLTAG;
	tag_t tagDatasetType = NULLTAG;
	tag_t tagRelationType = NULLTAG;
	tag_t tagRelation = NULLTAG;
	tag_t tagFileTag = NULLTAG;


	IMF_file_t file_desc;


	try
	{
		TERADYNE_ERROR_CALL(iFail = AE_find_datasettype2(cDatasetType, &tagDatasetType));

		if (tagDatasetType != NULLTAG)
		{
			//creates a Dataset by specified type, cName, description, id, and rev.
			TERADYNE_ERROR_CALL(iFail = AE_create_dataset_with_id(tagDatasetType, cDatasetName, cDatasetDesc, NULL, NULL, &(*tNewDataset)));

			TERADYNE_ERROR_CALL(iFail = AE_ask_datasettype_def_tool(tagDatasetType, &tagDefaultToolTag));

			if ((*tNewDataset) != NULLTAG)
			{
				TERADYNE_ERROR_CALL(iFail = AE_set_dataset_format2((*tNewDataset), cFormatName));

				// Set the default tool for the new dataset
				TERADYNE_ERROR_CALL(iFail = AE_set_dataset_tool((*tNewDataset), tagDefaultToolTag));
				if (tc_strcmp(cFormatName, TER_BINARY_FORMATTYPE) == 0)
				{
					TERADYNE_ERROR_CALL(iFail = IMF_import_file(cDatasetRefName, NULL, SS_BINARY, &tagFileTag, &file_desc));
				}
				else
				{
					TERADYNE_ERROR_CALL(iFail = IMF_import_file(cDatasetRefName, NULL, SS_TEXT, &tagFileTag, &file_desc));
				}

				TERADYNE_ERROR_CALL(iFail = IMF_close_file(file_desc));


				TERADYNE_ERROR_CALL(iFail = AOM_save_without_extensions(tagFileTag));

				TERADYNE_ERROR_CALL(iFail = AOM_refresh((*tNewDataset), true));

				// adds named reference
				TERADYNE_ERROR_CALL(iFail = AE_add_dataset_named_ref2((*tNewDataset), cReferenceName, refType, tagFileTag));

				TERADYNE_ERROR_CALL(iFail = AOM_save_without_extensions((*tNewDataset)));

				TERADYNE_ERROR_CALL(iFail = AOM_refresh((*tNewDataset), false));

			}
		}
	}
	catch (...)
	{

	}
	return iFail;
}
/**************************************************************************************************
*   Function Name      :teradyne_find_execute_qry
*   Description            :This function will EXECUTE QUERY BASED on Entries and values passed by function
*   REQUIRED HEADERS    :
*   INPUT/OUTPUT PARAMS    :
*
*   RETURN VALUE        : iFail = error code
*   GLOBALS USED        :  None
*   FUNCTIONS CALLED    :  None
*   ALGORITHM           :
*******************************************************************************************************/
int teradyne_find_execute_qry(const char *pcQueryName, std::map<string, string> strQryEntriesMap, int* count, tag_t** tFldrTag)
{
	int iFail = ITK_ok;
	int iTypesize = 0;
	int iAttrCount = 0;

	tag_t tQuery = NULLTAG;

	try {
		*count = 0;
		*tFldrTag = NULL;
		TERADYNE_ERROR_CALL(iFail = QRY_find2(pcQueryName, &tQuery));
		if (tQuery != NULLTAG)
		{
			char** pcEntries = NULL;
			char** pcValues = NULL;

			iTypesize = (int)strQryEntriesMap.size();
			//Allocate memory to query entries and values
			pcEntries = (char**)MEM_alloc(iTypesize * sizeof(char*));
			pcValues = (char**)MEM_alloc(iTypesize * sizeof(char*));
			for (map<string, string>::iterator it = strQryEntriesMap.begin(); it != strQryEntriesMap.end(); it++)
			{
				pcEntries[iAttrCount] = (char*)MEM_alloc((int)((strlen(it->first.c_str()) + 1) * sizeof(char)));
				pcValues[iAttrCount] = (char*)MEM_alloc((int)((strlen(it->second.c_str()) + 1) * sizeof(char)));
				tc_strcpy(pcEntries[iAttrCount], it->first.c_str());
				tc_strcpy(pcValues[iAttrCount], it->second.c_str());
				iAttrCount++;
			}

			//Execute query
			TERADYNE_ERROR_CALL(iFail = QRY_execute(tQuery, iAttrCount, pcEntries, pcValues, count, tFldrTag));

			for (int i = 0; i < iTypesize; i++)
			{
				MEM_free(pcEntries[i]);
				MEM_free(pcValues[i]);
			}
			MEM_free(pcEntries);
			MEM_free(pcValues);
		}
	}
	catch (...)
	{

	}

	return iFail;
}
/***************************************************************************************
*   Function Name       : teradyne_create_folder
*   INPUT/OUTPUT PARAMS : char  *cFolderName
*                         tag_t  *tFolderTag
*
*
*   RETURN VALUE        :ITK_ok for success
*                        iFail value in case of failures
*   GLOBALS USED        :None
*   FUNCTIONS CALLED    :
*
*   ALGORITHM           :
*
*********************************************************************************************/
int teradyne_create_folder(const char *cFolderName, tag_t *tFolderTag)
{
	int iFail = ITK_ok;
	tag_t tFolderTypeTag = NULLTAG;
	tag_t tCreateInputTag = NULLTAG;
	try
	{
		TERADYNE_ERROR_CALL(iFail = TCTYPE_ask_type(TER_FOLDER_TYPE, &tFolderTypeTag));

		TERADYNE_ERROR_CALL(iFail = TCTYPE_construct_create_input(tFolderTypeTag, &tCreateInputTag));

		TERADYNE_ERROR_CALL(iFail = AOM_set_value_string(tCreateInputTag, TER_OBJECT_NAME, cFolderName));

		TERADYNE_ERROR_CALL(iFail = TCTYPE_create_object(tCreateInputTag, &(*tFolderTag)));

		TERADYNE_ERROR_CALL(iFail = AOM_save((*tFolderTag)));
	}
	catch (...) {}
	return iFail;
}
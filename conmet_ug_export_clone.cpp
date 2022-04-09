/*=================================================================================================
#                Copyright (c) 2015 Teradyne
#                Unpublished - All Rights Reserved
#  =================================================================================================
#      Filename        :           izn_ug_export_import_assy.cpp
#      Module          :           izn_ug_export_import_assy
#      Project         :
#      Author          :           Vikash B
#  =================================================================================================
#  Date                                  Name                               Description of Change
#  22-March-2022                         Vikash B                   	             Initial Code
#  =================================================================================================*/


#include "conmet_ug_export_clone.h"
#include<bom\bom.h>
#include<tc/folder.h>
#include<map>
#include <ctime>

tag_t createPrgVarientItem(char*sInputFileName, char* objectname, char* bomblRevId);
int createChild(tag_t createdTiPrgVarientItem, tag_t _inputItemtag);
char* findItemRev(char * userName);

int iCnt = 0;
map<string, string> checkExistItem;
map<string, string> itemReport;
map<string, string> mapItemType;


ofstream MyFile;
ofstream itemMap;

static void ECHO(char *format, ...)
{
	char msg[1000];
	va_list args;
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	printf(msg);
	TC_write_syslog(msg);
}

#define IFERR_ABORT(X)  (report_error( __FILE__, __LINE__, #X, X, TRUE))
#define IFERR_REPORT(X) (report_error( __FILE__, __LINE__, #X, X, FALSE))

static int report_error(char *file, int line, char *call, int status,
	logical exit_on_error)
{
	if (status != ITK_ok)
	{
		int
			n_errors = 0;
		const int
			*severities = NULL,
			*statuses = NULL;
		const char
			**messages;

		EMH_ask_errors(&n_errors, &severities, &statuses, &messages);
		if (n_errors > 0)
		{
			ECHO("\n%s\n", messages[n_errors - 1]);
			EMH_clear_errors();
		}
		else
		{
			char *error_message_string;
			//EMH_get_error_string (NULLTAG, status, &error_message_string);
			ECHO("\n%s\n", error_message_string);
		}

		ECHO("error %d at line %d in %s\n", status, line, file);
		ECHO("%s\n", call);
		if (exit_on_error)
		{
			ECHO("%s", "Exiting program!\n");
			exit(status);
		}
	}
	return status;
}





int ITK_user_main(int argc, char* argv[])
{
	int iFail = ITK_ok;
	int n_entry = 1;
	char*sInputFileName = NULL;
	char * OTofInputItem = NULL;
	char * itemIdofcreatedTiPrgVarientItem = NULL;
	char * itemIdofcreatedTiPrgVarientItem1 = NULL;
	char ** errorMsg = NULL;
	char * createdItemId = NULL;
	const char* modelyears = "1998";
	char* itemIdFromWF = NULL;

	char *cUserName = NULL,
		*cPassWord = NULL,
		*cUsergroup = NULL,
		*cFilePath = NULL;

	tag_t inputItemTag = NULLTAG;
	tag_t inputItemRevTag = NULLTAG;
	tag_t inputItemRevTypeTag = NULLTAG;
	tag_t inputItemRevTypeTag1 = NULLTAG;
	tag_t construcinputItemTypeTag = NULLTAG;
	tag_t construcinputItemTypeTag1 = NULLTAG;
	tag_t construcinputItemRevTypeTag = NULLTAG;
	tag_t construcinputItemRevTypeTag1 = NULLTAG;
	tag_t construcinputPrgVarRevMasTypeTag = NULLTAG;
	tag_t construcinputPrgVarRevMasTypeTag1 = NULLTAG;
	tag_t inputPrgVarRevMasTypeTag = NULLTAG;
	tag_t inputPrgVarRevMasTypeTag1 = NULLTAG;
	tag_t inputItemTypeTag = NULLTAG;
	tag_t inputItemTypeTag1 = NULLTAG;
	tag_t createdTiPrgVarientItem = NULLTAG;
	tag_t inputPrgVarMasTag = NULLTAG;
	tag_t construcinputPrgVarMasTag = NULLTAG;
	tag_t createdTiPrgVarientItemRevMasTag = NULLTAG;
	tag_t createdTiPrgVarientItemRevMasTag1 = NULLTAG;
	tag_t createdTiPrgVarientItemRevTag = NULLTAG;

	string sClonFilePath = "";

	try
	{
		cUserName = ITK_ask_cli_argument("-u=");
		cPassWord = ITK_ask_cli_argument("-p=");
		cUsergroup = ITK_ask_cli_argument("-g=");
		cFilePath = ITK_ask_cli_argument("-filepath=");


		MyFile << "Files can be tricky, but it is fun enough!" << endl;
		MyFile << "user : " << cUserName << endl;
		MyFile << "group : " << cUsergroup << endl;

		/*------DISPLAY HELP--------*/
		if (ITK_ask_cli_argument("-help") || ITK_ask_cli_argument("-h")) {

			util_display_help();
			return iFail;
		}

		if ((cUserName == NULL) || (cPassWord == NULL) || (cUsergroup == NULL) || (cFilePath == NULL))
		{
			assy_display_usage();
			return -1;
		}

		if (ITK_init_module(cUserName, cPassWord, cUsergroup) != ITK_ok)
		{
			printf(IZN_ERR_TC_LOGIN_FAIL);
			TC_write_syslog(IZN_ERR_TC_LOGIN_FAIL);

			exit(1);
		}
		TC_write_syslog(IZN_TC_LOGGEDIN_STATUS);
		cout << IZN_TC_LOGGEDIN_STATUS << endl;
		sClonFilePath = izn_mkdir(cFilePath);
		string slogFile = izn_log_file_path(sClonFilePath);

		MyFile.open(slogFile + "\\LogFile.txt");
		itemMap.open(slogFile + "\\ItemMap.txt");

		string sBatchFilePath = cFilePath;

		ITK_set_bypass(true);

		itemIdFromWF = findItemRev(cUserName);
		sInputFileName = itemIdFromWF;

		MyFile << "Item id : " << sInputFileName << endl;

		createdTiPrgVarientItem = createPrgVarientItem(sInputFileName, "parent", "01");
		ITEM_find_item(sInputFileName, &inputItemTag);
		createChild(createdTiPrgVarientItem, inputItemTag);

		vector<string> key, value;
		for (map<string, string>::iterator it = itemReport.begin(); it != itemReport.end(); ++it) {
			key.push_back(it->first);
			value.push_back(it->second);
			itemMap << it->first << ", " << it->second << endl;
			MyFile << "map check:" << it->first << "," << it->second << endl;
		}
		itemMap.close();
		MyFile.close();

		//-------------------
		map<string, string> tempItemReportMap;
		for (map<string, string>::iterator it = itemReport.begin(); it != itemReport.end(); ++it) {
			string key = it->first;
			string value = it->second;
			tempItemReportMap.insert(pair<string, string>("@DB/" + key, "@DB/" + value));
		}
		tempItemReportMap.insert(pair<string, string>("EXPORT_OPERATION", "IMPORT_OPERATION"));
		tempItemReportMap.insert(pair<string, string>("OVERWRITE", "USE_EXISTING"));
		tempItemReportMap.insert(pair<string, string>("AUTO_TRANSLATE", "USER_NAME"));
		//tempItemReportMap.insert(pair<string, string>("CMI4_NPIPart", "CMI4_EngPart"));
		string sTopAssy = sInputFileName;

		export_and_import_assy(itemReport, tempItemReportMap, sClonFilePath, sBatchFilePath, sTopAssy);

		//ug_import_assy(sBatchFilePath, sClonFilePath);
		//----------------------------
		//string ugexport = "conmet_ug_import.exe \"" + sBatchFilePath + "\"" + " " + "\"" + sClonFilePath + "\"";
		//string conmet_ug_import = "start /d "  "E:\\TCINX_TESTING\\TestDir\\conmet_ug_import.exe -u=infodba -p=infodba -g=dba -filepath=\"" + sBatchFilePath + "\"" + " " + "\"" + sClonFilePath + "\"";
       /*
		string sBatchFile = sBatchFilePath + "/import_utility.bat";
		string ugexport = "CMD.exe /C " + sBatchFile  + " " + sClonFilePath;
		system(ugexport.c_str());

		cout << "conmet_ug_import = " << ugexport << endl;
		//system(conmet_ug_import.c_str());*/

		//-------------------
	}
	catch (...) {

		TC_write_syslog("\n ##### In catch block for error values \n");
	}

	cout << IZN_SUCCESS_STATUS << endl;
	ITK_set_bypass(false);
	ITK_exit_module(true);
	return ITK_ok;
}

char* findItemRev(char * userName) {
	tag_t user_tag = NULLTAG;
	tag_t* task_list = NULLTAG;
	tag_t root_task_t = NULLTAG;
	tag_t * ptAttachments = NULLTAG;
	int task_count = NULL;
	int iNumAttachments = NULL;
	char * job_name = NULL;
	char * taskName = NULL;

	char * taskName1 = NULL;
	char * objStrOfAttachments = NULL;
	char *objTypeOfAttachments = NULL;
	char * objIdOfAttachments = NULL;


	//IFERR_REPORT( SA_find_user2("ext-rganesan", &user_tag));
	IFERR_REPORT(SA_find_user2(userName, &user_tag));
	EPM_ask_assigned_tasks(user_tag, EPM_inbox_query, &task_count, &task_list);
	for (int i = 0; i < task_count; i++)
	{
		AOM_ask_value_tag(task_list[i], "root_task", &root_task_t);
		AOM_ask_value_string(root_task_t, "object_name", &job_name);
		AOM_ask_value_string(task_list[i], "object_name", &taskName);
		MyFile << "root task" << root_task_t << "job name" << job_name << "object string" << taskName << endl;
		if (tc_strcasecmp(taskName, "Convert_NPI_Item") == 0) {
			if (tc_strcasecmp(job_name, "NPI_Item_Conversion") == 0) {
				EPM_ask_attachments(root_task_t, EPM_target_attachment, &iNumAttachments, &ptAttachments);
				for (int i = 0; i < iNumAttachments; i++)
				{
					AOM_ask_value_string(ptAttachments[i], "object_name", &objStrOfAttachments);
					MyFile << "object name of the attachments : " << objStrOfAttachments << endl;
					AOM_ask_value_string(ptAttachments[i], "object_type", &objTypeOfAttachments);
					MyFile << "object type of the attachments : " << objTypeOfAttachments << endl;
					if (tc_strcasecmp(objTypeOfAttachments, "CMI4_NPIPartRevision") == 0) {  
						AOM_ask_value_string(ptAttachments[i], "item_id", &objIdOfAttachments);
						cout << "object Id of the attachments : " << objIdOfAttachments << endl;
					}
				}
			}
			/*AOM_ask_value_string(task_list[i], "object_name", &taskName1);
			//cout << "taskName1 ************   " << taskName1 << endl;
			MyFile<<"The task name check for Convert_NPI_Item : "<<taskName1<<endl;
			//IFERR_REPORT(EPM_trigger_action_if_privileged(task_list[i], EPM_complete_action, "Completed"));
			//IFERR_REPORT(EPM_trigger_action(task_list[i], EPM_complete_action, "Completed"));
			
			IFERR_REPORT(AOM_refresh(task_list[i], true));
			IFERR_REPORT(AOM_set_value_string(task_list[i],"comments", "Completed"));
			IFERR_REPORT(AOM_save_without_extensions(task_list[i]));
			//IFERR_REPORT(AOM_refresh(task_list[i], false));
			IFERR_REPORT(EPM_set_task_result(task_list[i], EPM_RESULT_Completed));
			IFERR_REPORT(EPM_trigger_action(task_list[i], EPM_complete_action, "Completed"));
			IFERR_REPORT(AOM_refresh(task_list[i], false));*/
		}
	}
	return objIdOfAttachments;
}

int createChild(tag_t createdTiPrgVarientItem, tag_t _inputItemtag) {

	tag_t newbomwind = NULLTAG;
	tag_t itemRevTag = NULLTAG;
	tag_t view_type = NULLTAG;
	tag_t bom_view = NULLTAG;
	tag_t bvr = NULLTAG;
	tag_t newtopBomLineTag = NULLTAG;
	tag_t inputItemTag = NULLTAG;
	tag_t inputItemRevTag = NULLTAG;
	tag_t * bvrOnInputItemRev = NULLTAG;
	tag_t bomwind = NULLTAG;
	tag_t topBomLineTag = NULLTAG;
	tag_t * bomChildrenTags = NULLTAG;
	tag_t createdTiPrgVarientItem1 = NULLTAG;
	tag_t itemRevTag1 = NULLTAG;
	tag_t createChildItemTag = NULLTAG;
	tag_t latestRevTag = NULLTAG;
	tag_t *bvrsTag = NULLTAG;
	tag_t tChildRevTag = NULLTAG;
	tag_t *bvrssTag = NULLTAG;

	char* bomblItemRevId = NULL;
	char* bomblQuantity = NULL;
	char* cEngPartType = NULL;
	char* cRevId = NULL;
	int bvrcount;
	int ichildCount;
	int iBvrsCnt = 0;
	//int iCnt = 0;

	//--------------------------------------
	IFERR_REPORT(ITEM_ask_latest_rev(createdTiPrgVarientItem, &latestRevTag));
	IFERR_REPORT(AOM_ask_value_string(latestRevTag, "item_revision_id", &cRevId));
	//cout << "item_revision_id **************  " << cRevId << endl;
	IFERR_REPORT(ITEM_find_revision(createdTiPrgVarientItem, cRevId, &itemRevTag));
	//------------------------------
	//ITEM_find_revision(createdTiPrgVarientItem, "01", &itemRevTag);

	if (iCnt == 0)
	{
		PS_find_view_type("view", &view_type);
		PS_create_bom_view(view_type, "", "", createdTiPrgVarientItem, &bom_view);
		AOM_save(bom_view);
		PS_create_bvr(bom_view, "", "", FALSE, itemRevTag, &bvr);
		AOM_save(bvr);
		iCnt++;
	}



	IFERR_REPORT(BOM_create_window(&newbomwind));
	IFERR_REPORT(BOM_set_window_top_line(newbomwind, NULLTAG, itemRevTag, NULLTAG, &newtopBomLineTag));
	IFERR_REPORT(BOM_save_window(newbomwind));



	IFERR_REPORT(ITEM_find_revision(_inputItemtag, "01", &inputItemRevTag));
	IFERR_REPORT(ITEM_rev_list_all_bom_view_revs(inputItemRevTag, &bvrcount, &bvrOnInputItemRev));
	MyFile << "bvr count : " << bvrcount << endl;


	IFERR_REPORT(BOM_create_window(&bomwind));
	IFERR_REPORT(BOM_set_window_top_line(bomwind, NULLTAG, inputItemRevTag, NULLTAG, &topBomLineTag));
	IFERR_REPORT(BOM_line_ask_all_child_lines(topBomLineTag, &ichildCount, &bomChildrenTags));
	MyFile << "child line count : " << ichildCount << endl;


	for (int i = 0; i < ichildCount; i++) {

		char * bomchildid = NULL;
		IFERR_REPORT(AOM_ask_value_string(bomChildrenTags[i], "bl_item_item_id", &bomchildid));
		IFERR_REPORT(AOM_ask_value_string(bomChildrenTags[i], "bl_quantity", &bomblQuantity));
		IFERR_REPORT(AOM_ask_value_string(bomChildrenTags[i], "bl_rev_item_revision_id", &bomblItemRevId));


		ITEM_find_rev(bomchildid, bomblItemRevId, &tChildRevTag);
		IFERR_REPORT(ITEM_rev_list_bom_view_revs(tChildRevTag, &iBvrsCnt, &bvrsTag));
		//IFERR_REPORT(ITEM_list_bom_views(bomChildrenTags[i], &iBvrsCnt, &bvrsTag));


		MyFile << "bl quantity : " << bomblQuantity << " bom child id :" << bomchildid << endl;


		createdTiPrgVarientItem1 = createPrgVarientItem(bomchildid, "child", bomblItemRevId);

		IFERR_REPORT(ITEM_find_revision(createdTiPrgVarientItem1, "01", &itemRevTag1));

		//cout << "iBvrsCnt   =  " << iBvrsCnt << endl;
		if (iBvrsCnt > 0)
		{
			PS_find_view_type("view", &view_type);
			PS_create_bom_view(view_type, "", "", createdTiPrgVarientItem1, &bom_view);
			AOM_save(bom_view);
			PS_create_bvr(bom_view, "", "", FALSE, itemRevTag1, &bvr);
			AOM_save(bvr);
		}

		IFERR_REPORT(BOM_line_add(newtopBomLineTag, createdTiPrgVarientItem1, NULLTAG, NULLTAG, &topBomLineTag));
		IFERR_REPORT(AOM_set_value_string(topBomLineTag, "bl_quantity", bomblQuantity));
		IFERR_REPORT(BOM_save_window(newbomwind));

		ITEM_find_item(bomchildid, &createChildItemTag);
		createChild(createdTiPrgVarientItem1, createChildItemTag);
	}


	IFERR_REPORT(BOM_close_window(newbomwind));
	IFERR_REPORT(BOM_close_window(bomwind));

	return 0;

}

tag_t createSoftwareComp(char* objectname)
{
	tag_t inputEngPartItemTag = NULLTAG;
	tag_t construcinputEngPartItemTag = NULLTAG;
	tag_t createdEngPartItemTag = NULLTAG;
	tag_t inputEngPartItemrevTag = NULLTAG;
	tag_t construcinputEngPartItemrevTag = NULLTAG;
	tag_t createdEngPartItemrevTag = NULLTAG;


	TCTYPE_find_type("CMI4_Soft_CompRevision", "CMI4_Soft_CompRevision", &inputEngPartItemrevTag);
	TCTYPE_construct_create_input(inputEngPartItemrevTag, &construcinputEngPartItemrevTag);
	const char *swCode[1] = { "Code" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_category", 1, swCode));

	TCTYPE_find_type("CMI4_Soft_Comp", "CMI4_Soft_Comp", &inputEngPartItemTag);
	TCTYPE_construct_create_input(inputEngPartItemTag, &construcinputEngPartItemTag);
	const char *objName[1] = { objectname };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "object_name", 1, objName));
	IFERR_REPORT(AOM_set_value_tag(construcinputEngPartItemTag, "revision", construcinputEngPartItemrevTag));
	IFERR_REPORT(TCTYPE_create_object(construcinputEngPartItemTag, &createdEngPartItemTag));
	IFERR_REPORT(AOM_save(createdEngPartItemTag));

	return createdEngPartItemTag;

}

tag_t createElectronicComp(char* objectname, char* cAttriComp)
{
	tag_t inputEngPartItemTag = NULLTAG;
	tag_t construcinputEngPartItemTag = NULLTAG;
	tag_t createdEngPartItemTag = NULLTAG;
	tag_t inputEngPartItemrevTag = NULLTAG;
	tag_t construcinputEngPartItemrevTag = NULLTAG;
	tag_t createdEngPartItemrevTag = NULLTAG;


	TCTYPE_find_type("CMI4_Elect_CompRevision", "CMI4_Elect_CompRevision", &inputEngPartItemrevTag);
	TCTYPE_construct_create_input(inputEngPartItemrevTag, &construcinputEngPartItemrevTag);
	const char *partMatType[1] = { "HALB" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_part_material_type", 1, partMatType));

	if ((cAttriComp != NULL) && (cAttriComp[0] == '\0'))
	{
		const char *partMakeSrc[1] = { "Buy" };
		IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_part_source", 1, partMakeSrc));
	}
	else {
		const char *partMakeSrc[1] = { cAttriComp };
		IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_part_source", 1, partMakeSrc));
	}




	TCTYPE_find_type("CMI4_Elect_Comp", "CMI4_Elect_Comp", &inputEngPartItemTag);
	TCTYPE_construct_create_input(inputEngPartItemTag, &construcinputEngPartItemTag);
	const char *objName[1] = { objectname };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "object_name", 1, objName));
	const char *PartHand[1] = { "M" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hand", 1, PartHand));
	const char *partHier[1] = { "AC" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hierarchy", 1, partHier));
	IFERR_REPORT(AOM_set_value_tag(construcinputEngPartItemTag, "revision", construcinputEngPartItemrevTag));
	IFERR_REPORT(TCTYPE_create_object(construcinputEngPartItemTag, &createdEngPartItemTag));
	IFERR_REPORT(AOM_save(createdEngPartItemTag));

	return createdEngPartItemTag;

}

tag_t createEngProd(char* objectname)
{
	tag_t inputEngPartItemTag = NULLTAG;
	tag_t construcinputEngPartItemTag = NULLTAG;
	tag_t createdEngPartItemTag = NULLTAG;
	tag_t inputEngPartItemrevTag = NULLTAG;
	tag_t construcinputEngPartItemrevTag = NULLTAG;
	tag_t createdEngPartItemrevTag = NULLTAG;


	TCTYPE_find_type("CMI4_EngProductRevision", "CMI4_EngProductRevision", &inputEngPartItemrevTag);
	TCTYPE_construct_create_input(inputEngPartItemrevTag, &construcinputEngPartItemrevTag);
	const char *partMatType[1] = { "FERT" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_part_material_type", 1, partMatType));
	const char *parttool[1] = { "Tools Not Req'd" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_tool_required", 1, parttool));
	const char *PartSourc[1] = { "Make" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_part_source", 1, PartSourc));

	TCTYPE_find_type("CMI4_EngProduct", "CMI4_EngProduct", &inputEngPartItemTag);
	TCTYPE_construct_create_input(inputEngPartItemTag, &construcinputEngPartItemTag);
	const char *objName[1] = { objectname };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "object_name", 1, objName));
	const char *PartHand[1] = { "M" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hand", 1, PartHand));
	const char *partHier[1] = { "AC" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hierarchy", 1, partHier));
	IFERR_REPORT(AOM_set_value_tag(construcinputEngPartItemTag, "revision", construcinputEngPartItemrevTag));
	IFERR_REPORT(TCTYPE_create_object(construcinputEngPartItemTag, &createdEngPartItemTag));
	IFERR_REPORT(AOM_save(createdEngPartItemTag));

	return createdEngPartItemTag;

}

tag_t createBuyPart(char* objectname)
{
	tag_t inputEngPartItemTag = NULLTAG;
	tag_t construcinputEngPartItemTag = NULLTAG;
	tag_t createdEngPartItemTag = NULLTAG;
	tag_t inputEngPartItemrevTag = NULLTAG;
	tag_t construcinputEngPartItemrevTag = NULLTAG;
	tag_t createdEngPartItemrevTag = NULLTAG;


	TCTYPE_find_type("CMI4_BuyPartRevision", "CMI4_BuyPartRevision", &inputEngPartItemrevTag);
	TCTYPE_construct_create_input(inputEngPartItemrevTag, &construcinputEngPartItemrevTag);
	const char *partMatType[1] = { "HALB" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemrevTag, "cmi4_part_material_type", 1, partMatType));



	TCTYPE_find_type("CMI4_BuyPart", "CMI4_BuyPart", &inputEngPartItemTag);
	TCTYPE_construct_create_input(inputEngPartItemTag, &construcinputEngPartItemTag);
	const char *objName[1] = { objectname };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "object_name", 1, objName));
	const char *PartHand[1] = { "M" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hand", 1, PartHand));
	const char *partHier[1] = { "AC" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hierarchy", 1, partHier));

	IFERR_REPORT(AOM_set_value_tag(construcinputEngPartItemTag, "revision", construcinputEngPartItemrevTag));
	IFERR_REPORT(TCTYPE_create_object(construcinputEngPartItemTag, &createdEngPartItemTag));
	IFERR_REPORT(AOM_save(createdEngPartItemTag));

	return createdEngPartItemTag;

}

tag_t createEngineeringPart(char* objectname, char* cAttriValue)
{
	tag_t inputEngPartItemTag = NULLTAG;
	tag_t construcinputEngPartItemTag = NULLTAG;
	tag_t inputEngPartRevTag = NULLTAG;
	tag_t construcEngPartRevTag = NULLTAG;
	tag_t createdEngPartItemTag = NULLTAG;

	IFERR_REPORT(TCTYPE_find_type("CMI4_EngPart", "CMI4_EngPart", &inputEngPartItemTag));

	IFERR_REPORT(TCTYPE_construct_create_input(inputEngPartItemTag, &construcinputEngPartItemTag));


	const char *PartName[1] = { objectname };
	//cout << "PartName ***********************************************   " << PartName << "   objectname =  " << objectname << endl;
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "object_name", 1, PartName));
	const char *PartHand[1] = { "M" };

	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hand", 1, PartHand));
	const char *PartHier[1] = { "AC" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcinputEngPartItemTag, "cmi4_part_hierarchy", 1, PartHier));



	IFERR_REPORT(TCTYPE_find_type("CMI4_EngPartRevision", "CMI4_EngPartRevision", &inputEngPartRevTag));

	IFERR_REPORT(TCTYPE_construct_create_input(inputEngPartRevTag, &construcEngPartRevTag));

	if ((cAttriValue != NULL) && (cAttriValue[0] == '\0'))
	{
		const char *PartSourc[1] = { "Make" };
		IFERR_REPORT(TCTYPE_set_create_display_value(construcEngPartRevTag, "cmi4_part_source", 1, PartSourc));
	}
	else {
		const char *PartSour[1] = { cAttriValue };
		IFERR_REPORT(TCTYPE_set_create_display_value(construcEngPartRevTag, "cmi4_part_source", 1, PartSour));
	}


	const char *toolReq[1] = { "Tools Not Req'd" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcEngPartRevTag, "cmi4_tool_required", 1, toolReq));
	const char *MaterialType[1] = { "HALB" };
	IFERR_REPORT(TCTYPE_set_create_display_value(construcEngPartRevTag, "cmi4_part_material_type", 1, MaterialType));

	IFERR_REPORT(AOM_set_value_tag(construcinputEngPartItemTag, "revision", construcEngPartRevTag));

	IFERR_REPORT(TCTYPE_create_object(construcinputEngPartItemTag, &createdEngPartItemTag));
	IFERR_REPORT(AOM_save(createdEngPartItemTag));

	return createdEngPartItemTag;

}

tag_t createPrgVarientItem(char*sInputFileName, char* objectname, char* bomblRevId)
{
	tag_t inputEngPartItemTag = NULLTAG;
	tag_t construcinputEngPartItemTag = NULLTAG;
	tag_t createdTiPrgVarientItemRevMasTag1 = NULLTAG;
	tag_t inputItemRevTypeTag1 = NULLTAG;
	tag_t construcinputItemRevTypeTag1 = NULLTAG;
	tag_t inputItemTypeTag1 = NULLTAG;
	tag_t construcinputItemTypeTag1 = NULLTAG;
	tag_t createdTiPrgVarientItem1 = NULLTAG;
	tag_t _inputItemTag = NULLTAG;
	tag_t inputItemRevTag = NULLTAG;
	tag_t retruntag = NULLTAG;
	tag_t inputEngPartRevTag = NULLTAG;
	tag_t construcEngPartRevTag = NULLTAG;
	tag_t createdEngPartRevMasTag = NULLTAG;
	tag_t createdEngPartItemTag = NULLTAG;
	tag_t latesRevTag = NULLTAG;
	tag_t created_item_tag = NULLTAG;
	tag_t latestRevTag = NULLTAG;

	char *OTofInputItem = NULL;
	char * itemIdofcreatedTiPrgVarientItem1 = NULL;
	char * TypeOfCreatedObject = NULL;
	char* cEngPartType = NULL;
	char* cObjectName = NULL;
	char* cRevId = NULL;

	ITEM_find_item(sInputFileName, &_inputItemTag);
	ITEM_find_revision(_inputItemTag, "01", &inputItemRevTag);
	AOM_ask_value_string(_inputItemTag, "object_type", &OTofInputItem);
	MyFile << "object type : " << OTofInputItem << endl;


	if (tc_strcasecmp(OTofInputItem, "CMI4_NPIPart") == 0) { //  change  TI_Product
		MyFile << "The Given ItemId is NPI Product" << endl;

		std::string existItemcheck = checkExistItem.find(sInputFileName)->second;
		char* existingItem = const_cast<char*>(existItemcheck.c_str());

		MyFile << "Existing item check in map : " << existingItem << endl;
		if (!tc_strlen(existingItem) > 0) {


			AOM_ask_value_string(inputItemRevTag, "cmi4_eng_part_type", &cEngPartType);

			//cout << "cmi4_eng_part_type =========== ********** " << cEngPartType << endl;

			//--------------------------------------
			IFERR_REPORT(AOM_ask_value_string(_inputItemTag, "object_name", &cObjectName));

			if (tc_strcasecmp(cEngPartType, "Buy Part") == 0)
			{
				created_item_tag = createBuyPart(cObjectName);
			}
			else if (tc_strcasecmp(cEngPartType, "Electronic (Buy)") == 0) {

				created_item_tag = createElectronicComp(cObjectName, "Buy");
			}
			else if (tc_strcasecmp(cEngPartType, "Electronic (Make)") == 0) {

				created_item_tag = createElectronicComp(cObjectName, "Make");
			}
			else if (tc_strcasecmp(cEngPartType, "Electronic (Design-Purchase)") == 0) {

				created_item_tag = createElectronicComp(cObjectName, "Design-Purchase");
			}
			else if (tc_strcasecmp(cEngPartType, "Software Component") == 0) {
				created_item_tag = createSoftwareComp(cObjectName);
			}
			else if (tc_strcasecmp(cEngPartType, "Engineering Product") == 0) {
				created_item_tag = createEngProd(cObjectName);
			}
			else if (tc_strcasecmp(cEngPartType, "Engineering Part (Make)") == 0) {

				created_item_tag = createEngineeringPart(cObjectName, "Make");
			}
			else if (tc_strcasecmp(cEngPartType, "Engineering Part (Design-Purchase)") == 0) {

				created_item_tag = createEngineeringPart(cObjectName,"Design-Purchase");
			}
			else {
				created_item_tag = createEngineeringPart(cObjectName,"");
			}

			//IFERR_REPORT(AOM_ask_value_string(_inputItemTag, "object_name", &cObjectName));
			//created_item_tag = createEngineeringPart(cObjectName);
			if (created_item_tag != NULLTAG) {

				IFERR_REPORT(ITEM_ask_latest_rev(created_item_tag, &latestRevTag));
				IFERR_REPORT(AOM_ask_value_string(latestRevTag, "item_revision_id", &cRevId));
				//IFERR_REPORT(ITEM_find_revision(createdTiPrgVarientItem, cRevId, &itemRevTag));

				IFERR_REPORT(AOM_ask_value_string(created_item_tag, "item_id", &itemIdofcreatedTiPrgVarientItem1));
				IFERR_REPORT(AOM_ask_value_string(created_item_tag, "object_type", &TypeOfCreatedObject));
			}


			MyFile << "created item id : " << itemIdofcreatedTiPrgVarientItem1 << "and Type : " << TypeOfCreatedObject << endl;
			checkExistItem.insert(pair<string, string>(sInputFileName, itemIdofcreatedTiPrgVarientItem1));


			string a1 = sInputFileName;
			string a2 = bomblRevId;
			string a3 = a1 + "/" + a2;

			string b1 = itemIdofcreatedTiPrgVarientItem1;
			//string b2 = "01";
			string b2 = cRevId;
			string b3 = b1 + "/" + b2;

			mapItemType.insert(pair<string, string>(b3, TypeOfCreatedObject));


			MyFile << "map check in string : " << a3 << "," << b3 << endl;

			itemReport.insert(pair<string, string>(a3, b3));
			IFERR_REPORT(ITEM_find_item(itemIdofcreatedTiPrgVarientItem1, &retruntag));
			return retruntag;
		}
		else {
			IFERR_REPORT(ITEM_find_item(existingItem, &retruntag));
			return retruntag;
		}

	}
	else if (tc_strcasecmp(OTofInputItem, "CMI4_CADProxy") == 0) {

		return _inputItemTag;

	}
	else
	{
		return NULLTAG;
	}

}



//--------------------------------------------------------------------------------------------------------------------


int export_and_import_assy(map<string, string>itemReport, map<string, string> mapAssyParts, string sClonFilePath, string sBatchFilePath, string sTopAssy)
{
	int iFail = ITK_ok;

	try
	{

		ug_export_assy(sBatchFilePath, sClonFilePath, sTopAssy);

		namespace fs = std::filesystem;
		std::string sPath = sClonFilePath;

		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) == "clone")
			{
				string soutputfile = sFileName.substr((sFileName.find_last_of("\\") + 1));
				//soutputfile.erase(soutputfile.size() - 4);
				//finput_file.open(sFileName);

				//-----------------------------------------
				//izn_replace_dwg_prt(sPath);
				izn_replace_def_dir(sClonFilePath, sPath);
				izn_replace_part(sPath);
				izn_replace_clone_name(sPath);
				izn_replace_ass_dir(sPath);

				//-------------------------------------
		/*	for (map<string, string>::iterator it = itemReport.begin(); it != itemReport.end(); ++it)
			{
				string key = it->first;
				string value=it->second;
				tempItemReportMap.insert(pair<string, string>("@DB/"+key, "@DB/"+ value));
				//izn_replace_wit_new_id(itemReport,sPath, value);

			}*/
				izn_replace_wit_new_id(itemReport, sPath, "");

				//------------------------------------------
				//string newId = "CP001662/01";
				//izn_replace_wit_new_id(sPath, newId);

				for (auto it = mapAssyParts.cbegin(); it != mapAssyParts.cend(); ++it)
				{
					//cout << it->first << " key value " << it->second << endl;

					string  sKeyoldValue = it->first;
					string sNewValue = it->second;
					replace_values(sFileName, sKeyoldValue, sNewValue);

				}

				//----------------call ug_import_assy--------------------------

				//ug_import_assy(sBatchFilePath, sClonFilePath);
				replace_part_type(sPath);

			}
		}
	}
	catch (...) {}


	return iFail;
}


void ug_export_assy(string sBatchFilePath, string sFilePath, string sTopAssy)
{
	try
	{
		string sBatchFile = sBatchFilePath + "/ug_export.bat";
		string ugexport = "CMD.exe /C " + sBatchFile + " @DB/" + sTopAssy + "/01 " + sFilePath + " " + sFilePath;
		system(ugexport.c_str());
	}
	catch (...) {}

}


void ug_import_assy(string sBatchFilePath, string sClonFilePath)
{
	try
	{
		string line;
		ifstream ini_file{ sClonFilePath + "/exported_log.clone" };
		ofstream out_file{ sClonFilePath + "/new_exported.clone" };

		if (ini_file && out_file) {
			int i = 0;
			while (getline(ini_file, line)) {
				if (i == 1)
				{
					out_file << "&LOG Operation_Type: IMPORT_OPERATION" << "\n";
				}
				else if (i == 2) {

				}
				else if (i == 16)
				{
					out_file << "&LOG Default_Validation_Mode : OFF" << "\n";
					out_file << "&LOG" << "\n";
				}
				else
					out_file << line << "\n";


				i++;
			}

		}
		else {
			printf("Cannot read File");
		}

		//Closing file
		ini_file.close();
		out_file.close();

		string sBatchFile = sBatchFilePath + "/ug_import.bat";
		string ug_import = "CMD.exe /C " + sBatchFile + " " + sClonFilePath + "/new_exported.clone " + sClonFilePath + "/imported_clone_log.clone";
		//string sBatchFile = sBatchFilePath + "/ug_import.bat";
		//string ug_import = "CMD.exe /C /D" + sBatchFile + " " +"\""+sClonFilePath + "/exported_log.clone"+" \"" + "\""+sClonFilePath+"\"";

		//cout << "inside ug_import function =  " << ug_import << endl;
		system(ug_import.c_str());




	}
	catch (...) {}

}

void replace_values(string sFileName, string sOldValue, string sNewValue)
{
	try
	{
		std::fstream file(sFileName, std::ios::in);

		if (file.is_open()) {
			std::string replace = sOldValue;
			std::string replace_with = sNewValue;
			std::string line;
			std::vector<std::string> lines;

			while (std::getline(file, line)) {

				std::string::size_type pos = 0;

				while ((pos = line.find(replace, pos)) != std::string::npos) {
					//cout << " matched line ==  "<< line <<endl;
					line.replace(pos, line.size(), replace_with);
					pos += replace_with.size();
					//cout << " matched line  pos ****  " << pos << endl;
				}

				lines.push_back(line);
			}

			file.close();
			file.open(sFileName, std::ios::out | std::ios::trunc);

			for (const auto& i : lines) {
				file << i << std::endl;
			}
		}

	}
	catch (...) {}

}

string toknize(string sPartRev, string sDB)
{
	string temp1 = "", temp2 = "";
	try
	{
		char *cstr = new char[sPartRev.length() + 1];
		strcpy(cstr, sPartRev.c_str());

		char *token = strtok(cstr, "_");
		int i = 0;

		while (token != NULL)
		{
			if (i == 0)
				temp1 = token;
			//printf("%s\n", token);
			token = strtok(NULL, "_ .");
			i++;

			if (i == 1)
				temp2 = token;

			if (i == 2) {

				delete[] cstr;
				break;
			}

		}
	}
	catch (...) {}

	if (sDB == "DB")
	{
		return temp1 + "/" + temp2;
	}
	else
		return temp1 + "_" + temp2;

}


string izn_file_path(string sPath) {

	size_t found = 0, next = 0;

	try
	{
		while ((found = sPath.find('/', next)) != std::string::npos)
		{
			sPath.insert(found, "\\");
			next = found + 4;
		}
		sPath.erase(remove(sPath.begin(), sPath.end(), '/'), sPath.end());
	}
	catch (...) {}

	return sPath;
}


void izn_replace_part(string sPath)
{
	namespace fs = std::filesystem;
	map<string, string> mapDatasetLoc;

	try
	{
		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) != "clone")
			{
				string sPartN = sFileName.substr((sFileName.find_last_of("\\") + 1));
				if (sPartN.find("dwg") != string::npos)
				{
					string sDownPart = sPath + "/" + sPartN;
					string sPartIdRev = toknize(sPartN, "DB");
					string sPartRev = sPartIdRev;
					std::replace(sPartRev.begin(), sPartRev.end(), '/', '-');
					string sNxPartDir = "@DB/" + sPartIdRev + "/specification/" + sPartRev + "-dwg1";
					mapDatasetLoc.insert(pair<string, string>("\"" + sNxPartDir + "\"", "\"" + sDownPart + "\""));

				}
				else {
					string sPartIdRev = toknize(sPartN, "DB");
					string sDownPart = sPath + "/" + sPartN;
					string sPartRev = sPartIdRev;
					string sNxPartDir = "@DB/" + sPartRev;
					mapDatasetLoc.insert(pair<string, string>(sNxPartDir, "\"" + sDownPart + "\""));
				}

			}
		}

		izn_general_fun(sPath, mapDatasetLoc);

	}
	catch (...) {}

}



void izn_replace_clone_name(string sPath)
{
	namespace fs = std::filesystem;
	map<string, string> mapDatasetLoc;

	try
	{
		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) != "clone")
			{
				string sPartN = sFileName.substr((sFileName.find_last_of("\\") + 1));
				if (sPartN.find("dwg") != string::npos)
				{
					string sCompPath = izn_file_path(sFileName);
					string sPartIdRev = toknize(sPartN, "DB");
					string sPartRev = sPartIdRev;
					std::replace(sPartRev.begin(), sPartRev.end(), '/', '-');
					string sNxPartDir = "@DB/" + sPartIdRev + "/specification/" + sPartRev + "-dwg1";
					mapDatasetLoc.insert(pair<string, string>("\"" + sCompPath + "\"", "\"" + sNxPartDir + "\""));
				}
				else {
					string sPartIdRev = toknize(sPartN, "DB");
					string sDownPart = sPath + "/" + sPartN;
					string sCompPath = izn_file_path(sDownPart);
					string sPartRev = sPartIdRev;
					string sNxPartDir = "@DB/" + sPartRev;
					mapDatasetLoc.insert(pair<string, string>("\"" + sCompPath + "\"", sNxPartDir));
				}

			}
		}

		izn_general_fun(sPath, mapDatasetLoc);

	}
	catch (...) {}


}


void izn_replace_ass_dir(string sPath)
{
	namespace fs = std::filesystem;
	map<string, string> mapDatasetLoc;

	try
	{
		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) != "clone")
			{
				string sPartN = sFileName.substr((sFileName.find_last_of("\\") + 1));
				string sPartIdRev = toknize(sPartN, "");
				string sDownPart = sPath + "/" + sPartIdRev;
				string sCompPath = izn_file_path(sDownPart);
				mapDatasetLoc.insert(pair<string, string>(sCompPath, "\""));
			}
		}

		izn_general_fun(sPath, mapDatasetLoc);
	}
	catch (...) {}


}



void izn_replace_def_dir(string sClonFilePath, string sPath)
{
	namespace fs = std::filesystem;
	map<string, string> mapDatasetLoc;

	try
	{
		mapDatasetLoc.insert(pair<string, string>(sClonFilePath, "\""));
		izn_general_fun(sPath, mapDatasetLoc);
	}
	catch (...) {}


}

void izn_replace_wit_new_id(map<string, string> itemReport, string sPath, string sNewPartWithRev)
{
	namespace fs = std::filesystem;
	map<string, string> mapDatasetLoc;

	try
	{
		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) != "clone")
			{
				string sPartN = sFileName.substr((sFileName.find_last_of("\\") + 1));
				if (sPartN.find("dwg") != string::npos) {

					string sPartIdRev = toknize(sPartN, "DB");
					for (map<string, string>::iterator it = itemReport.begin(); it != itemReport.end(); ++it)
					{
						string key = it->first;
						string value = it->second;

						if (sPartIdRev == key)
							sNewPartWithRev = value;

					}

					string sTempPart = sNewPartWithRev;
					std::replace(sTempPart.begin(), sTempPart.end(), '/', '-');
					string snewPartId = "@DB/" + sNewPartWithRev + "/specification/" + sTempPart + "-dwg1";

					string sPartRev = sPartIdRev;
					std::replace(sPartRev.begin(), sPartRev.end(), '/', '-');
					string sNxPartDir = "@DB/" + sPartIdRev + "/specification/" + sPartRev + "-dwg1";
					mapDatasetLoc.insert(pair<string, string>("\"" + sNxPartDir + "\"", "\"" + snewPartId + "\""));

				}
				/*else {
					string newPartId = "@DB/"+sNewPartWithRev;
					string sPartIdRev = toknize(sPartN, "DB");
					string sNxPartDir = "@DB/" + sPartIdRev;
					mapDatasetLoc.insert(pair<string, string>(sNxPartDir, newPartId));
				}*/

			}
		}

		izn_general_fun(sPath, mapDatasetLoc);

	}
	catch (...) {}
}



void izn_general_fun(string sPath, map<string, string> mapDatasetLoc)
{
	namespace fs = std::filesystem;

	try
	{
		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) == "clone")
			{
				string soutputfile = sFileName.substr((sFileName.find_last_of("\\") + 1));

				for (auto it = mapDatasetLoc.cbegin(); it != mapDatasetLoc.cend(); ++it)
				{
					string  sKeyoldValue = it->first;
					string sNewValue = it->second;
					replace_values(sFileName, sKeyoldValue, sNewValue);
				}

			}
		}

	}
	catch (...) {}

}



/*******************************************************************************
 * Function Name			: Util_display_help
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
int util_display_help()
{
	cout << endl;
	cout << "Usage:" << endl;
	cout << "Assy_export_and_import_in_tc";
	cout << " -u=<Userid>";
	cout << " -p=<Password>";
	cout << " -p=<Group>";
	cout << "[-help]" << endl;
	cout << "Where" << endl;
	cout << "-u         - TC user ID should be \"admin\"." << endl;
	cout << "-p         - TC password." << endl;
	cout << "-g         - TC group." << endl;
	cout << "-filepath  - Please provide file directory path" << endl;

	return 0;
}


/*****************************************************************************************************
 * Function Name			: Assy_display_usage
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

int assy_display_usage(void)
{
	int iFail = ITK_ok;
	cout << "********************** Input_Command_Usage **********************\n";
	cout << "Error: Input command line arguments are less than expected!" << endl;
	cout << "[-u= username -p=password -g=group]\n";
	cout << "[- filepath - Please provide file directory path" << endl;
	cout << "********************** Input_Command_Usage **********************\n";
	return iFail;
}



/*****************************************************************************************************
 * Function Name			: izn_local_time
 * Description			    : It'll give the Local time.
 *
 * REQUIRED HEADERS :
 * INPUT/OUTPUT PARAMS      :
 * RETURN VALUE				:strDateReturn
 * GLOBALS USED				:
 * FUNCTIONS CALLED			:
 *
 * ALGORITHM				:
 *
 * NOTES					:
 *
 *******************************************************************************************************/
string izn_local_time(string sDateFormat)
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

	return strDateReturn;
}


/*******************************************************************************
 * Function Name			: izn_mkdir
 * Description				: making the assy files directory.
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

string izn_mkdir(char* sFilePath) {

	try
	{
		string sDateTime = izn_local_time("");
		string sAssyFilePath = sFilePath;

		string sPathDir = sAssyFilePath + "/" + sDateTime;
		if (!sPathDir.empty())
		{
			_mkdir(sPathDir.c_str());
			return sPathDir;
		}
		else {
			cout << "Directory is not created" << endl;
		}

	}
	catch (...) {}

}


string izn_log_file_path(string sPath) {

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



void replace_part_type(string sPath)
{
	namespace fs = std::filesystem;

	int iLine = 0;

	try
	{

		for (const auto & entry : fs::directory_iterator(sPath))
		{

			string sFileName(entry.path().string());

			if (sFileName.substr((sFileName.find_last_of(".") + 1)) == "clone")
			{
				string soutputfile = sFileName.substr((sFileName.find_last_of("\\") + 1));

				for (map<string, string>::iterator it = mapItemType.begin(); it != mapItemType.end(); ++it)
				{
					string sOldValue = it->first;
					string sNewValue = it->second;
					sNewValue = "&LOG Part_Type: " + sNewValue;

					std::fstream file(sFileName, std::ios::in);

					if (file.is_open()) {
						std::string replace = sOldValue;
						std::string replace_with = sNewValue;
						std::string line;
						std::vector<std::string> lines;

						int numLines = 0;
						int temp = 0;

						while (std::getline(file, line))
						{
							std::string::size_type pos = 0;
							numLines++;

							size_t found = line.find(replace);
							if (found != string::npos)
							{
								temp = numLines;
							}


							if (numLines == temp + 2)
							{
								if (iLine > 0)
								{
									line.replace(line.find(line), line.size(), replace_with);
								}
								iLine++;
							}

							lines.push_back(line);
						}

						file.close();
						file.open(sFileName, std::ios::out | std::ios::trunc);

						for (const auto& i : lines) {
							file << i << std::endl;
						}
					}

				}
			}
		}

	}
	catch (...) {}

}
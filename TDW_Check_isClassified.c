/**================================================================================================
 *            		    Copyright (c) TDWILLIAMSON 2016
 *                     Unpublished - All rights reserved
 * ================================================================================================
 * File Name        : TDW_Check_isClassified.c
 * File Description : Validate targets are classidied or not
#==================================================================================================
#Revision History
#==================================================================================================
#	Date         			Name           	   TC-Release              Description of Change
# ----------- 	----------------------------- -------------   ------------------------------------
#  02-06-2017			  DSA Team				TC10.1.5		          Initial creation
#==================================================================================================
 */

#include "iman_headers.h"
#include "TDW_Method_user_exits.h"
#include <time.h>
#include<epm/epm.h>
#define datasetType "TEXT"
/****************************************************************************
    Function:       TDW_Check_Classified
    Description:    Validate targets whether classified or not

    Input:          None

    Output:         None
    Return value:   status = 0 on success
	                status = Error Base on failure 
 ****************************************************************************/
int tdw_dataset_attachToreference(char *ObjectID,tag_t root_task ,tag_t job,FILE *fptr, char * temp,char** arraystr,int i,tag_t currenttask,char date_t[100]);
int TDW_SendmailtoTaskowner(tag_t root_task,char** arraystr,int i,tag_t currenttask,char date_t[100]);
char* TDW_Mail_format(char**arraystr,int i, char** overview,char date_t[100]); 

int TDW_Check_isClassified(EPM_action_message_t msg){

	int		status                           = 0,
			target_count                     = 0,
			pref_count                       = 0,
			targets                          = 0,
			i								 = 0,
			j								 = 0,
			k                                = 0,
			argument_count                   = 0,
			argument                         = 0,
			parsecnt                         = 0;

	char  *server_name                       = NULL,
			**prfvalues                        = NULL,
			*targetobjects                     = NULL,
			*isclassified                      = NULL,
			*objtype                           = NULL,
			*cObjectID                         = NULL,
			*cObjectIDString                   = NULL,
			*temp                              = NULL,
			**argument_name                    = NULL,
			**argument_values                  = NULL,
			*object                            = NULL,
			**parsevalues                      = NULL,
			*cTaskType						 = NULL,
			**arraystr;  


	tag_t   job                              = NULLTAG,
			root_task                        = NULLTAG,
			*attachments                   	 = NULLTAG,
			datasettype                      = NULLTAG,
			newdataset                       = NULLTAG,
			currenttask                      = NULLTAG;

	logical     isnotclassified = false;

	FILE *fptr;

	time_t timer;
	char date_time[100];
	struct tm tm_info;
	time(&timer);
	 localtime_s(&tm_info,&timer);
	strftime(date_time, 100, "%Y-%m-%d-%H-%M-%S", &tm_info);

	cObjectID = (char*) MEM_alloc((128)*sizeof(char));
	tc_strcpy(cObjectID, "");

	temp = (char *)TC_getenv( "TEMP" );
	tc_strcat(temp,"\\");
	tc_strcat(temp,date_time);

	if(temp == NULL || strlen( temp ) == 0) {

		TC_write_syslog(" \n Error: Temp -- environment variable is not found\n");

	}

	tc_strcat(temp,"-Notclassified_list.txt");

	TC_write_syslog("\n Text file dir is :%s",temp);

	currenttask=msg.task;
	ERROR_CALL(EPM_ask_job(msg.task, &job));
	ERROR_CALL( EPM_ask_root_task(job, &root_task));

	ERROR_CALL(EPM_ask_attachments(root_task, EPM_target_attachment, &target_count, &attachments));
	TC_write_syslog("\n Attachment count :%d", target_count);


	ERROR_CALL(EPM_args_process_args(msg.task, msg.arguments, &argument_count, &argument_name, &argument_values));
	for(argument; argument < argument_count; argument++) {
		if(tc_strcmp(argument_name[argument], "include_types") == 0 ) {
			object = (char*)MEM_alloc(((int)strlen(argument_values[argument])+1)*sizeof(char));
			tc_strcpy(object, argument_values[argument]);
		}
		else
			TC_write_syslog("\n Error: handler argument is wrong, it should be include_types");
	}

	EPM__parse_string(object,",",&parsecnt,&parsevalues); 


	fopen_s(&fptr,temp, "w" );
	// fptr = fopen(temp, "w");
	fprintf(fptr,"Below Objects are not classified:\n");
	arraystr = (char **) MEM_alloc (target_count * sizeof(char *));

	if(parsecnt>0)
	{      
		for(targets;targets<target_count;targets++)
		{
			ERROR_CALL(WSOM_ask_object_type2(attachments[targets],&targetobjects));
			TC_write_syslog("\n Target is :%s",targetobjects);

			for(k=0;k<parsecnt;k++)
			{
				if(strcmp(parsevalues[k],targetobjects)==0)
				{
					ERROR_CALL(AOM_ask_value_string(attachments[targets],"ics_classified",&isclassified));
					if(strcmp(isclassified,"")==0)
					{
						ERROR_CALL(AOM_ask_value_string(attachments[targets],"object_string",&cObjectIDString));
						isnotclassified=true;
						fprintf(fptr,"%s\n",cObjectIDString);
						arraystr[i] = (char *) MEM_alloc (128 * sizeof(char));
						//tc_strcpy(Storestring[i],"");
						tc_strcpy(arraystr[i],cObjectIDString);
						i++;
						TC_write_syslog("\n Target %s is not classified ",targetobjects);
					} else
						TC_write_syslog("\n Target %s is classified ",targetobjects);
				}

			}
		}
	} else 
		TC_write_syslog("\n Error:No Values in the included_types");

	fclose(fptr);

	ERROR_CALL(WSOM_ask_object_type2(currenttask, &cTaskType));

	if (tc_strcmp(cTaskType, "EPMConditionTask") == 0) {

		if(isnotclassified) {

			tdw_dataset_attachToreference(cObjectID,root_task,job,fptr,temp,arraystr,i,currenttask,date_time);

			if(cObjectID != NULL) {
				MEM_free(cObjectID);
			}
			ERROR_CALL(EPM_set_condition_task_result(currenttask, EPM_RESULT_FALSE));
		} else {
			remove(temp);
			ERROR_CALL(EPM_set_condition_task_result(currenttask, EPM_RESULT_TRUE));
		}
	}

	Custom_free_array(argument_name, argument_count);
	Custom_free(object);
	Custom_free(arraystr);

	return ITK_ok;
}


/****************************************************************************
    Function:    tdw_dataset_attachToreference
    Description: Write unclassified item details and attach it to Reference

    Input:         

    Output:         None
    Return value:   
 ****************************************************************************/
int tdw_dataset_attachToreference(char *ObjectID,tag_t root_task ,tag_t job,FILE *fptr, char * temp,char** arraystr,int i,tag_t currenttask,char date_t[100])
{
	int		refCounter                        = 0,
			count                             = 1,
			attachment_types1;

	char    **dataSetReferenceList             = NULL;

	tag_t datasettype                         = NULLTAG,
			newdataset                          = NULLTAG,
			file_tag                            = NULLTAG;

	IMF_file_t file_descriptor;

	ERROR_CALL(AE_find_datasettype(datasetType,&datasettype));
	ERROR_CALL(AE_create_dataset(datasettype,"unclassifieditems","This file contains unclassified items information",&newdataset));
	ERROR_CALL(AOM_save(newdataset));
	ERROR_CALL(AE_ask_datasettype_refs( datasettype,&refCounter,&dataSetReferenceList));
	TC_write_syslog("\n named ref is :%s",dataSetReferenceList[0]);
	ERROR_CALL(IMF_import_file(temp,NULL,SS_TEXT,&file_tag,&file_descriptor));
	ERROR_CALL(AOM_save(file_tag));

	ERROR_CALL(AOM_refresh(newdataset,TRUE));
	ERROR_CALL(AE_add_dataset_named_ref(newdataset,"Text",AE_ASSOCIATION,file_tag));
	ERROR_CALL(AOM_save(newdataset));
	ERROR_CALL(AOM_unload(file_tag));
	ERROR_CALL(AOM_refresh(newdataset,FALSE));

	attachment_types1 = EPM_reference_attachment;
	ERROR_CALL(EPM_add_attachments(root_task,count, &newdataset, &attachment_types1));

	remove(temp);
	TDW_SendmailtoTaskowner(root_task,arraystr,i,currenttask,date_t);

	return ITK_ok;
}
/****************************************************************************
    Function:    TDW_SendmailtoTaskowner
    Description: Notify-mail to task owner

    Input:         

    Output:         None
    Return value:   
 ****************************************************************************/

int TDW_SendmailtoTaskowner(tag_t root_task,char** arraystr,int i,tag_t currenttask,char date_t[100]) {


	int    number_of_tasks             = 0,
			j                          = 0,
			subcnt                     = 0,
			l                          = 0,
			i_NoOfReviewers            = 0,
			k                          = 0,
			pref_count                 = 0,
			attchcnt                   = 0;

	char	*tcRoot                    = NULL,
			*nametask                  = NULL,
			*process_name              = NULL,
			*intstruction              = NULL,
			*comments                  = NULL,
			*s_Reviewer_Id             = NULL,
			*mailaddrs                 = NULL,
			*server_name               = NULL,
			*mail_file_name            = NULL;

	tag_t process_owner              =NULLTAG;


	char* Overview_details[4];
	char command_mail[BUFSIZ +1];



	TC_preference_search_scope_t scope =TC_preference_site;

	command_mail[0] = '\0';
	//Get TC_ROOT
	tcRoot = (char *)TC_getenv( "TC_ROOT" );

	if(tcRoot == NULL || strlen( tcRoot ) == 0) {

		TC_write_syslog(" \n Error: TC_ROOT -- environment variable is not found\n");

	}
	tc_strcpy(command_mail,tcRoot);
	tc_strcat(command_mail,"\\bin\\tc_mail_smtp");

	ERROR_CALL(AOM_ask_value_string(currenttask,"current_name",&nametask));
	ERROR_CALL(AOM_ask_value_string(currenttask,"parent_name",&process_name));
	ERROR_CALL(AOM_ask_value_string(currenttask,"object_desc",&intstruction));

	Overview_details[0]=nametask;
	Overview_details[1]=process_name;
	Overview_details[2] = "";
	Overview_details[3]=intstruction;


	TC_write_syslog("\n current Task :%s",nametask);
	TC_write_syslog("\n process_name :%s",process_name);

	ERROR_CALL(AOM_ask_value_tag(root_task,"owning_user",&process_owner));

	ERROR_CALL(SA_ask_user_identifier2 (process_owner, &s_Reviewer_Id));
	ERROR_CALL(EPM_get_user_email_addr(process_owner,&mailaddrs));
	if(mailaddrs != NULL &&  strlen( mailaddrs ) != 0){

		tc_strcat(command_mail," -to=");
		tc_strcat(command_mail,mailaddrs);
		TC_write_syslog("\n -To= :%s",mailaddrs);

	}

	tc_strcat(command_mail," -subject=");
	tc_strcat(command_mail,"\"");
	tc_strcat(command_mail,"Classification Notification:");
	tc_strcat(command_mail,nametask);
	tc_strcat(command_mail,"\"");
	TC_write_syslog("\n -subject= :%s",nametask);

	PREF_initialize();
	PREF_ask_search_scope( &scope);
	PREF_set_search_scope( TC_preference_site);
	PREF_ask_value_count( "Mail_server_name", &pref_count );
	PREF_ask_char_value( "Mail_server_name", 0, &server_name );

	tc_strcat(command_mail," -server=");
	tc_strcat(command_mail,server_name);
	TC_write_syslog("\n -server= :%s",server_name);
	tc_strcat(command_mail," -port=25");
	TC_write_syslog("\n -port=25");

	mail_file_name=TDW_Mail_format(arraystr,i,Overview_details,date_t);
	tc_strcat(command_mail," -body=");

	tc_strcat(command_mail,mail_file_name);

	TC_write_syslog("\n Command is :%s\n",command_mail);
	system(command_mail);
	remove(mail_file_name);

	return ITK_ok;
}

/****************************************************************************
    Function:     TDW_Mailformat
    Description:  content of tha mail. 

    Input:         

    Output:         None
    Return value:  
 ****************************************************************************/
char* TDW_Mail_format( char**arraystr,int i, char** overview,char date_t[100]) {

	int		m                            = 0,
			n                            = 0,
			o                            = 0;

	char	*mail_file_name             = NULL,
			*temp_dir                   = NULL;

	FILE	*mail_file                  = NULL;

	char* OverviewTitle[] = {"Current Task:", "Process Name: ", "Comments: ","Instruction: "};

	temp_dir = (char *)TC_getenv( "TEMP" );
	if(temp_dir == NULL || strlen( temp_dir ) == 0) {

		TC_write_syslog(" \n Error: Temp -- environment variable is not found\n");

	}
	tc_strcat(temp_dir,"\\");
	tc_strcat(temp_dir,date_t);
	tc_strcat(temp_dir,"-Notify_mail.htm");


	fopen_s(&mail_file,temp_dir, "w" );

	// mail_file=fopen(temp_dir, "w" );
	fprintf(mail_file, "<!DOCTYPE html><html><head/><body><div style=\"color:#448da6;font-family:arial; font-weight:bold; margin-bottom:3px\">Overview:</div><table style=\"font-family:arial\"><tbody>");

	for (m=0; m<4; m++) {
		fprintf(mail_file,"<tr>");
		fprintf(mail_file, "<td style=\"vertical-align:top; text-align:left; color:#808080\">");
		fprintf(mail_file, "%s", OverviewTitle[m] );
		fprintf(mail_file, "</td><td style=\"vertical-align:top; text-align:left\">");
		fprintf(mail_file, "%s", overview[m]);
		fprintf(mail_file, "</td>");
		fprintf(mail_file,"</tr>");
	}
	fprintf(mail_file, "</tbody>");
	fprintf(mail_file, "</table>");
	fprintf(mail_file, "<br>");
	fprintf(mail_file, "<div style=\"color:#448da6;font-family:arial; font-weight:bold; margin-bottom:3px\">Unclassified Objects:</div><table style=\"font-family:arial; width:50%%; border:1px solid #808080; border-collapse:collapse; padding:2px\">");


	fprintf(mail_file,"<tbody><tr>");
	fprintf(mail_file, "<td style=\"font-weight:bold; color:#808080;text-align:left; border:1px solid #808080; border-collapse:collapse; padding:2px\">Objects");
	fprintf(mail_file, "</td>");
	fprintf(mail_file,"</tr>");

	for (n=0; n<i; n++) {
		fprintf(mail_file,"<tr>");
		fprintf(mail_file, "<td style=\"vertical-align:top; text-align:left; border:1px solid #808080; border-collapse:collapse; padding:2px\">");
		fprintf(mail_file, "%s", arraystr[n]);
		fprintf(mail_file, "</td>");
		fprintf(mail_file,"</tr>");
	}
	fprintf(mail_file, "</tbody></table>");
	fprintf(mail_file, "\n<div style=\"font-weight:bold;color:#808080\">This email was sent from Teamcenter</div>");

	fprintf(mail_file, "</body></html>");

	fclose(mail_file);
	return temp_dir;
}
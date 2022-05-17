
//#include<audit.h>
#include<epm.h>
#include <ss_const.h>
#include <imanfile.h>
#include<sa.h>
#include <time.h>
#include <locale.h>
#include <iman_headers.H>



void throw_error(int errorno,char *log_filename,char *itemid)
{
	 char *err_string;
	 FILE *flog=NULL;
	 EMH_get_error_string (NULLTAG, errorno, &err_string);   
	 flog=fopen(log_filename,"a+");
		
	  fprintf (flog,"Error processing id %s \n",itemid);
	  fprintf (flog,"ERROR: %d ERROR MSG: %s.\n", errorno, err_string); 
	  fprintf(flog,"FUNCTION: AOM_ask_value_string \nFILE: %s LINE: %d\n",__FILE__, __LINE__);

      fclose(flog);

	  if (err_string!=NULL)MEM_free(err_string);
}

// void ARCELIK_display_help();
void initialize(char **userName,char **passWord,char **userGroup,char **itemtype, char **status_name1,char **logfilename1,char **export_path,char **from_date,char **to_date);
int AC_export_cgm_dataset(tag_t itemrev, tag_t cgm, char datasettype[32],char exportdir[250],char *timestamp,char *status_name);
int AC_get_timestamp(char **timestamp);

logical file_exists(char *filename);
int AC_Create_Csv_CGM(tag_t itemrev,char name[250],char exportdir[250]);
logical INTLZ_Is_Allowed_Disallowed_Type(tag_t ObjTag,  char *ClassName, int No_of_objects, char **Object_list);

int watermark_counter=0;
int ITK_user_main(int argc,char * argv[])
{
	char *status_name	= NULL,	*templog= NULL; 
    char *logFilePath	= NULL,typename3[100]="DocumentRevision",*statusListValue=NULL,
		*item_id=NULL,*item_revision_id=NULL,*AC_Supplier=NULL,*AC_Plant=NULL,*AC_MRP_Resp=NULL,*owning_group=NULL;

	FILE *fp=NULL,*fout=NULL,*flog=NULL;

    int  result,i=0,entry_count,num_found;

    char *id_string=NULL,*user=NULL,
		 *pwd=NULL, *grp=NULL, *object_id=NULL,*itemtype=NULL,*log_filename=NULL,*from_date=NULL,*to_date=NULL,
	     **entries,**values,**ent,**val,nowstr[100],object_type[20],objectType[129]="";
	
	tag_t query_tag, *object_tags;
  	struct tm *nowstruct; 
	time_t time_now;
		tag_t	*attachments,				rootTask		= NULLTAG,			    itemTag         = NULLTAG,
		        relation_type   = NULLTAG,				Iman_type	    = NULLTAG,				*primaries      = {NULLTAG},	
				*referencers    = {NULLTAG},				IMANSpec_Relation=NULLTAG,				*secondaries    = {NULLTAG},
				CGM_Relation    = NULLTAG,				*cgms           = {NULLTAG};
		
		
		int		noAttachment	= 0,iCnt			= 0,typeCnt         = 0, no_of_args      = 0, attachment_type = 0, jCnt   = 0,checkType		= 0,
				n_primaries     = 0,n_referencers   = 0,kCnt            = 0,lCnt            = 0,family_member_found=0,	*levels,n_secondary=0,n_cgms=0;
		
		char	typeName[129],					 
                 *tempName                     = NULL, *tempVal  = NULL,**listOfTypes  = NULL, relation[50]	= {'\0'},*className = NULL,
				 flName[129]                  = {'\0'},	 **relations,export_path[250]   = "", objType[129] = "", *timestamp  = NULL,
				  *tc_data_path = NULL,*export_path1=NULL,*rev_id_string;

          

	 if (time(&time_now) == (time_t) - 1)
		{
	       printf("Could not get time of day from time()\n");
		}
	 nowstruct = localtime(&time_now);
     strftime(nowstr, 80, "%m%d%Y%H%M%S",nowstruct);

     if ( argc < 1)
	   {
		   printf("No Argument supplied \n");
		   //ARCELIK_display_help();
		   exit(1);
	   }

	   from_date=MEM_alloc(50 * sizeof(char)); strcpy(from_date,"");
	   to_date=MEM_alloc(50 *sizeof(char));  strcpy(to_date,"");

       initialize(&user,&pwd,&grp,&itemtype,&status_name,&log_filename,&export_path1,&from_date,&to_date);
     strcpy(export_path,export_path1);

       logFilePath=MEM_alloc(200);
       //strcpy(logFilePath,templog);
       //strcat(logFilePath,"\\");
	   // strcat(logFilePath,argv[0]);
	   // strcat(logFilePath,nowstr);
	   // strcat(logFilePath,".syslog");

	   /*lineCount=0;
	   user=ITK_ask_cli_argument("-u=");
       pwd=ITK_ask_cli_argument("-p=");
	   grp=ITK_ask_cli_argument("-g=");*/

 
	if (ITK_init_module(user,pwd,grp) == ITK_ok )
 	{
		  result=QRY_find("Item Revision...",&query_tag);
         result=QRY_find_user_entries(query_tag,&entry_count,&entries,&values);
		  printf("Query Ran Succesfully %d\n",result);
		  
		/*for(i=0;i<entry_count;i++)
		{
		printf("Entry name %d is %s and Entry value %d is %s",i,entries[i],i,values[i]);
        } */
	

          ent=MEM_alloc(sizeof(char*) *20); 
		  val=MEM_alloc(sizeof(char*) *20);
          ent[0]=MEM_alloc(sizeof(char)*100);
		 // ent[1]=MEM_alloc(sizeof(char*)*100);
	
		  val[0]=MEM_alloc(sizeof(char)*100);
		   // val[1]=MEM_alloc(sizeof(char*)*100);
       
		  ent[0]=entries[11];		  val[0]=itemtype;
		  //  ent[1]=entries[12];		  val[1]=workflow_name;
		  if(strlen(from_date)>0 && strlen(to_date)>0)
		  {
			  ent[1]=MEM_alloc(sizeof(char)*100);  ent[2]=MEM_alloc(sizeof(char)*100); ent[3]=MEM_alloc(sizeof(char)*100); 
			  ent[1]=entries[18];  ent[2]=entries[19]; ent[3]=entries[20];
			  val[1]=MEM_alloc(sizeof(char)*50); val[2]=MEM_alloc(sizeof(char)*50); val[3]=MEM_alloc(sizeof(char)*50); 
			  val[1]=from_date;  val[2]=to_date; val[3]=status_name;

			   result=QRY_execute(query_tag,4,ent,val,&num_found,&attachments);
			   printf("Called date range \n");
			   MEM_free(ent); MEM_free(val);
		  
		  }
		  else
		  {
			  printf("Called all date \n");
		  result=QRY_execute(query_tag,1,ent,val,&num_found,&attachments);
		  MEM_free(ent); MEM_free(val);
		   
		  }
	       fprintf(stdout, "\n\t %d is the total objects Found...\n",num_found);
           flog=fopen(log_filename,"a+");
		   fprintf(flog,"No of item revisoins found are %d \n",num_found);
		   printf("No of item revisoins found are %d \n",num_found);
		   fclose(flog);
		  
		 ERROR_CALL(GRM_find_relation_type(relation, &relation_type ));

				 	for(iCnt=0;iCnt<num_found;iCnt++)
							{  
								family_member_found=0;
								checkType = 0;
								watermark_counter=0;
								
								result=WSOM_ask_object_type(attachments[iCnt],typeName );
							  
								result=WSOM_ask_id_string(attachments[iCnt],&rev_id_string);	

								if(result!=0) continue;
								printf("Processing %s \n",rev_id_string);
								

								ERROR_CALL(GRM_list_primary_objects_only(itemTag,relation_type, &n_primaries, &primaries));
								if(n_primaries>0)
								{
									MEM_free(primaries); 
									continue;
								}
								WSOM_where_referenced(itemTag,1,&n_referencers,&levels,&referencers,&relations); 

								for(kCnt=0; kCnt<n_referencers; kCnt++)
									{
										POM_class_of_instance(referencers[kCnt],&Iman_type);
										POM_name_of_class(Iman_type,&className);
										if(stricmp(className,"Folder")==0)
											{
											if(className!=NULL) MEM_free(className);
											WSOM_ask_name(referencers[kCnt],flName);	
											if(stricmp(flName,"Family Members")==0)family_member_found=1;									
											}

									}
								if(n_referencers>0){MEM_free(referencers);}
								if (family_member_found==1) continue;
								AC_get_timestamp(&timestamp);

								result=GRM_list_secondary_objects_only(attachments[iCnt],IMANSpec_Relation, &n_secondary, &secondaries);
								if (result!=0) continue;

								for(kCnt=0;kCnt<n_secondary;kCnt++)
								{
										POM_class_of_instance(secondaries[kCnt],&Iman_type);
										POM_name_of_class(Iman_type,&className);
										if(stricmp(className,"Dataset")!=0)continue;
										result=WSOM_ask_object_type(secondaries[kCnt],objType);
										if(result!=0) continue;
										
										if(stricmp(objType,"UGPART")==0)
										{
											result=GRM_find_relation_type("IMAN_Drawing",&CGM_Relation);
											result= GRM_list_secondary_objects_only(secondaries[kCnt],CGM_Relation, &n_cgms, &cgms);
											for(lCnt=0;lCnt<n_cgms;lCnt++)
											{
												POM_class_of_instance(cgms[lCnt],&Iman_type);
												POM_name_of_class(Iman_type,&className);
												if(stricmp(className,"Dataset")!=0)continue;
												 WSOM_ask_object_type(cgms[lCnt],objType) ;
												 if(result!=0) continue;
												if(stricmp(objType,"DrawingSheet")!=0) continue;
												
												   flog=fopen(log_filename,"a+");
													fprintf(flog,"Creating report for %s \n",rev_id_string);
													printf("Creating report for %s \n",rev_id_string);
													fclose(flog);

												AC_export_cgm_dataset(attachments[iCnt],cgms[lCnt],"DrawingSheet",export_path,timestamp,status_name);
											   
											}
											if(n_cgms>0)MEM_free(cgms);
										
										}
										else if(stricmp(objType,"IdeasDrawing")==0)
										{
										ERROR_CALL(GRM_find_relation_type("IMAN_capture",&CGM_Relation));
										ERROR_CALL( GRM_list_secondary_objects_only(secondaries[kCnt],CGM_Relation, &n_cgms, &cgms));
											for(lCnt=0;lCnt<n_cgms;lCnt++)
											{
												POM_class_of_instance(cgms[lCnt],&Iman_type);
												POM_name_of_class(Iman_type,&className);
												if(stricmp(className,"Dataset")!=0)continue;
												ERROR_CALL(WSOM_ask_object_type(cgms[lCnt],objType));
												if(stricmp(objType,"Image")!=0) continue;
											

												AC_export_cgm_dataset(attachments[iCnt],cgms[lCnt],"Image",export_path,timestamp,status_name);
											    
											}
											if(n_cgms>0)MEM_free(cgms);
										}
								  
								}
								 
								if(n_secondary>0) MEM_free(secondaries);
							}
 printf("Finished running utility\n");
	}
	return ITK_ok;
}





void initialize(char **userName,
			    char **passWord,
				char **userGroup,char **itemtype,char **status_name,char **log_filename,char **export_path,char **from_date,char **to_date)
{
	//char arcelikRel[50];
	int  ifail				= ITK_ok;
	char *temp=NULL;
	/* Read User Input Values */

    /* Display help */
    if ( ITK_ask_cli_argument("-help") || ITK_ask_cli_argument("-h") )
    {
        //ARCELIK_display_help();
        exit(1);
    }

    /* Gets the user ID */
    *userName = ITK_ask_cli_argument( "-u=" );
    if ( ( *userName == NULL ) || ( strlen( *userName ) == 0 ) )
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The userd id is not specified. which is required. \n");
        fprintf(stderr , "\nThe userd id is not specified which is required." );
        //ARCELIK_display_help();
        exit(1);
    }

    /* Gets the user Password */
    *passWord = ITK_ask_cli_argument( "-p=" );
    if ( ( *passWord == NULL ) )
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The user password is not specified. which is required. \n");
        fprintf(stderr , "\nThe user password is not specified which is required." );
      //  ARCELIK_display_help();
        exit(1);
    }

    /* Gets the user group */
    *userGroup = ITK_ask_cli_argument( "-g=" );
    if ( ( *userGroup == NULL ) || ( strlen( *userGroup ) == 0 ) )
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The user group is not specified. which is required. \n");
        fprintf(stderr , "\nThe group is not specified which is required." );
      //  ARCELIK_display_help();
        exit(1);
    }

	*itemtype = ITK_ask_cli_argument( "-itemtype=" );
    if ( ( *itemtype == NULL ) || ( strlen( *itemtype ) == 0 ) )
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The user group is not specified. which is required. \n");
        fprintf(stderr , "\nThe itemtype is not specified which is required." );
      //  ARCELIK_display_help();
        exit(1);
    }

    /* Gets the input file name */

	 *status_name = ITK_ask_cli_argument( "-status_name=" );
    if (( *status_name== NULL ) || ( strlen( *status_name) == 0 ))
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The input file is not specified. which is required. \n");
        fprintf(stderr , "\nThe exportfilename is not specified which is required." );
       // ARCELIK_display_help();
        exit(1);
    }

	 *log_filename = ITK_ask_cli_argument( "-log_filename=" );
    if (( *log_filename== NULL ) || ( strlen( *log_filename) == 0 ))
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The input file is not specified. which is required. \n");
        fprintf(stderr , "\nThe log_filename is not specified which is required." );
       // ARCELIK_display_help();
        exit(1);
    }
	 *export_path = ITK_ask_cli_argument( "-export_path=" );
    if (( *export_path== NULL ) || ( strlen( *export_path) == 0 ))
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The input file is not specified. which is required. \n");
        fprintf(stderr , "\nThe export path is not specified which is required." );
       // ARCELIK_display_help();
        exit(1);
    }

	 temp = ITK_ask_cli_argument( "-from_date=" );
    if (( temp== NULL ) || ( strlen( temp) == 0 ))
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The input file is not specified. which is required. \n");
        fprintf(stderr , "\n From date not specified will process query for all date" );
       // ARCELIK_display_help();
      
    }
	else
	{
		sprintf((*from_date),"%s 23:59",temp);
	}


	 temp= ITK_ask_cli_argument( "-to_date=" );
    if (( temp== NULL ) || (strlen(temp) == 0 ))
    {
        //IMAN_write_syslog( "ARCELIK ERROR MESSAGE: The input file is not specified. which is required. \n");
        fprintf(stderr , "\n To Date not specified");
       // ARCELIK_display_help();
     
    }
	else
	{
		sprintf((*to_date),"%s 23:59",temp);
	}

 
}




int AC_export_cgm_dataset(tag_t itemrev, tag_t cgm, char datasettype[32],char exportdir[250],char *timestamp,char *status_name)
 {
	 int	ifail				= ITK_ok,
			iCnt				= 0,
			result              =0;
			
	 tag_t	tgzdatasetType		= NULLTAG;

	 char	*objName			= NULL,
			*item_id			= NULL,
			*rev_id				= NULL,
			**dataSetReferenceList	= {NULL},
			*reference_name=NULL;
	 
	 char	name[250]	= "",
			watermark_name[250]	= "",
		    cgm_filename[250]="";

	tag_t	referenced_object	= NULLTAG;

	logical	fexist				= FALSE;

	FILE *watermark_file=NULL;

	AE_reference_type_t reference_type;

	ERROR_CALL(AE_find_datasettype(datasettype,&tgzdatasetType));


	ERROR_CALL(AE_ask_datasettype_refs( tgzdatasetType,&iCnt,&dataSetReferenceList));
	
	ERROR_CALL(AOM_ask_value_string(itemrev,"item_id",&item_id));

	ERROR_CALL(AOM_ask_value_string(itemrev,"item_revision_id",&rev_id));

	ERROR_CALL(AOM_ask_name(cgm,&objName));
          
	
				
	if(iCnt==0)
	{
		fprintf(stdout,"\n\n   ERROR: No Named Reference found");
		return ITK_ok;
	}

	ERROR_CALL(AE_ask_dataset_named_ref(cgm,dataSetReferenceList[0],&reference_type,&referenced_object));

		
	if(referenced_object==NULLTAG)
	{
		fprintf(stdout,"\n\n   ERROR: No Named Reference found");
	    return ITK_ok;
	}
            
			sprintf(name, "%s_%s_%s",item_id,rev_id,timestamp);
			AOM_UIF_ask_value(cgm,"ref_list",&reference_name);
	
			sprintf(watermark_name,"%s\\%s_%s_%s.watermark",exportdir,item_id,rev_id,timestamp);
			        
		if(strstr(reference_name,".cgm")!=NULL)
		{
			sprintf(cgm_filename,"%s\\%s_%s_%s_%d.cgm",exportdir,item_id,rev_id,timestamp,watermark_counter);
			//fprintf(stdout,"\n   Exporting \"%s\"",cgm_filename);
			ERROR_CALL(IMF_export_file(referenced_object, cgm_filename));
			if(watermark_counter==0)
			{
			fexist = file_exists(watermark_name);
			if(fexist==TRUE){ fprintf(stdout,"\n     INFO: File already in the process ");watermark_counter++; return ITK_ok; }
			watermark_file=fopen(watermark_name,"w");
			fprintf(watermark_file,"CgmTable.exe|%s|%s|%s|%s|%s|%s.csv|#NX#%s#%s|%s",item_id,rev_id,exportdir,objName,name,name,rev_id,status_name,datasettype);
			fclose(watermark_file);
			result= AC_Create_Csv_CGM(itemrev,name,exportdir);
			
			}
			
		}
		else if(strstr(reference_name,".zip")!=NULL)
		{
			sprintf(cgm_filename,"%s\\%s_%s_%s_%d.zip",exportdir,item_id,rev_id,timestamp,watermark_counter);
			//fprintf(stdout,"\n   Exporting \"%s\"",cgm_filename);
			ifail = IMF_export_file(referenced_object, cgm_filename);
			if(watermark_counter==0)
			{
			fexist = file_exists(watermark_name);
			if(fexist==TRUE){ fprintf(stdout,"\n     INFO: File already in the process "); watermark_counter++;return ITK_ok; }
			watermark_file=fopen(watermark_name,"w");
			fprintf(watermark_file,"CgmTable.exe|%s|%s|%s|%s|%s|%s.csv|#IDEAS#%s#%s|%s",item_id,rev_id,exportdir,objName,name,name,rev_id,status_name,datasettype);
			fclose(watermark_file);
			AC_Create_Csv_CGM(itemrev,name,exportdir);
			}
			
		}
 
		watermark_counter++;

     if(objName!=NULL) MEM_free(objName);
	 if(reference_name!=NULL) MEM_free(reference_name);
	 return ITK_ok;
 }

   int AC_Create_Csv_CGM(tag_t itemrev,char name[250],char exportdir[250])
   {
	   tag_t itemTag                         = NULLTAG,
		     AC_Variant_Relation			 = NULLTAG,
			 *variant_tags					 = NULLTAG,
			 *revTagList                     = NULLTAG,
			 Iman_type                       = NULLTAG,
			 variant_latest_rev_tag          = NULLTAG,
			 *folder_tags                     = NULLTAG;

       char csv_name[255]="",
		    *Rev_Reason=NULL,
			Base_revID[ITEM_id_size_c + 1]   = {'\0'},
			revID[ITEM_id_size_c + 1]   = {'\0'},
			itemId[ITEM_id_size_c + 20]		 = {'\0'},
			*StatusNames                     = NULL,
			*className                       = NULL,
			flName[129]={'\0'};

	   int n_variants = 0,
		   i          = 0,
		   revCount   = 0,
		   n_folders  = 0,
		   fl_index   = 0;

	   FILE *csv_file;

	   logical	fexist				= FALSE;

	   FL_sort_criteria_t sort_criteria;

	   ERROR_CALL(ITEM_ask_item_of_rev(itemrev, &itemTag));
	   ERROR_CALL(AOM_ask_value_string(itemrev,"Rev_Reason_C", &Rev_Reason));
	   ERROR_CALL(ITEM_ask_id(itemTag, itemId));
	   ERROR_CALL(ITEM_ask_rev_id(itemrev, Base_revID));
	   ERROR_CALL(ITEM_list_all_revs(itemTag, &revCount, &revTagList));

	   ERROR_CALL(GRM_find_relation_type("AC_Variants",&AC_Variant_Relation));

	   
	   sprintf(csv_name,"%s\\%s.csv",exportdir,name);

	   fexist = file_exists(csv_name);

	   if(fexist==TRUE){ fprintf(stdout,"\n     INFO: CSV File already Created "); return ITK_ok; }

	   //Print Base Item Information first in the table
		
	   csv_file=fopen(csv_name,"w");
	   fprintf(csv_file,"Base,%s,%s,",itemId,Base_revID);
	  
	   fprintf(csv_file,"%s,\n",Rev_Reason);
	   fclose(csv_file);

	   ERROR_CALL( GRM_list_secondary_objects_only(itemTag,AC_Variant_Relation, &n_variants, &variant_tags));
	   for(i=0;i<n_variants;i++)
	   {
	    
		   POM_class_of_instance(variant_tags[i],&Iman_type);
		   POM_name_of_class(Iman_type,&className);
		   if(stricmp(className,"Item")!=0) continue;
			if(className!=NULL) MEM_free(className);
		    ITEM_ask_latest_rev(variant_tags[i],&variant_latest_rev_tag);
			ERROR_CALL(AOM_ask_value_string(variant_latest_rev_tag,"Rev_Reason_C", &Rev_Reason));
			ERROR_CALL(ITEM_ask_id(variant_tags[i], itemId));
			ERROR_CALL(ITEM_ask_rev_id(variant_latest_rev_tag,revID));
			ERROR_CALL(ITEM_list_all_revs(variant_tags[i], &revCount, &revTagList));

		     csv_file=fopen(csv_name,"a+");
			 fprintf(csv_file,"Variant,%s,%s,",itemId,revID);

	      	 if(strcmp(revID,Base_revID)==0)  fprintf(csv_file,"%s,",Rev_Reason);
			 else  fprintf(csv_file,"No Rev,");

			 ERROR_CALL(AOM_UIF_ask_value(variant_tags[i],"release_status_list",&StatusNames));

	         if(strstr(StatusNames,"Obsolete")!=NULL) fprintf(csv_file,"Obsolete");
			 fprintf(csv_file,"\n");
		     fclose(csv_file);
			 if(StatusNames!=NULL) MEM_free(StatusNames);
					
		   
	   }

	  
	   //Family Members
    	//ERROR_CALL( GRM_list_secondary_objects_only(itemTag,NULLTAG, &n_variants, &variant_tags));
	   ERROR_CALL( GRM_list_secondary_objects_only(itemTag,NULLTAG, &n_folders, &folder_tags));
		for(fl_index=0;fl_index<n_folders;fl_index++)
				{
					POM_class_of_instance(folder_tags[fl_index],&Iman_type);
					 POM_name_of_class(Iman_type,&className);
					 if(stricmp(className,"Folder")==0)
				     {
						if(className!=NULL) MEM_free(className);
					    WSOM_ask_name(folder_tags[fl_index],flName);
						if(stricmp(flName,"Family Members")==0)
						{
							 sort_criteria = FL_fsc_as_ordered;
							 FL_ask_references(folder_tags[fl_index],sort_criteria,&n_variants,&variant_tags);
							  for(i=0;i<n_variants;i++)
								{
								    
									POM_class_of_instance(variant_tags[i],&Iman_type);
									POM_name_of_class(Iman_type,&className);
									if(stricmp(className,"Item")!=0) continue;
									 if(className!=NULL) MEM_free(className);
										ITEM_ask_latest_rev(variant_tags[i],&variant_latest_rev_tag);
										ERROR_CALL(AOM_ask_value_string(variant_latest_rev_tag,"Rev_Reason_C", &Rev_Reason));
										ERROR_CALL(ITEM_ask_id(variant_tags[i], itemId));
										ERROR_CALL(ITEM_ask_rev_id(variant_latest_rev_tag,revID));
										ERROR_CALL(ITEM_list_all_revs(variant_tags[i], &revCount, &revTagList));

										csv_file=fopen(csv_name,"a+");
										fprintf(csv_file,"Family Member,%s,%s,",itemId,revID);

	      								if(strcmp(revID,Base_revID)==0)  fprintf(csv_file,"%s,",Rev_Reason);
										else  fprintf(csv_file,"No Rev,");

										ERROR_CALL(AOM_UIF_ask_value(variant_tags[i],"release_status_list",&StatusNames));

										if(strstr(StatusNames,"Obsolete")!=NULL) fprintf(csv_file,"Obsolete",Rev_Reason);
										fprintf(csv_file,"\n");
										fclose(csv_file);
										if(StatusNames!=NULL) MEM_free(StatusNames);
														
									   
								}

						 
						}

					 }
				}


   
if(n_folders>0) MEM_free(folder_tags);
   if(n_variants>0) MEM_free(variant_tags);
   if(Rev_Reason!=NULL) MEM_free(Rev_Reason);
   if(revCount>0) MEM_free(revTagList); 

   return ITK_ok;
   }

 

int AC_get_timestamp(char **timestamp)
{
	/* declaration */
	time_t time_now;
	//char *nowstr;
	struct tm *nowstruct;

	/*************Getting system time for generating log file*************/
	if (time(&time_now) == (time_t) - 1)
	{
		fprintf(stdout,"\n\n   ERROR: Could not get time of day from time()\n");
	}

	nowstruct = localtime(&time_now);

	*timestamp = (char *) MEM_alloc(33);

    strftime((*timestamp), 80, "%m%d%Y%H%M%S",nowstruct);

	return ITK_ok;
}



logical file_exists(char *filename)
{
	FILE * file;

	file = fopen(filename, "r");

    if(file) //I'm sure, you meant for READING =)
    {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}



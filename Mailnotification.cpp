int TDW_SendMailNotificationtoTaskowner(tag_t root_task, vector<string> vPartswithNoRelatedObjects, tag_t currenttask) {
 
    int        pref_count                                = 0;
 
    char    *tcRoot                                    = NULL,
            *nametask                                  = NULL,
            *process_name                              = NULL,
            *intstruction                           = NULL,
            *comments                               = NULL,
            *s_Reviewer_Id                          = NULL,
            *mailaddrs                                 = NULL,
            *server_name                               = NULL,
            *mail_file_name                            = NULL;
 
    tag_t    tag_performer                              = NULLTAG,
            process_owner                              = NULLTAG;
 
    char*    Overview_details[4];
    char    command_mail[BUFSIZ +1];
 
    TC_preference_search_scope_t scope = TC_preference_site;
 
    command_mail[0] = '\0';
 
    // Get TC_ROOT
    tcRoot = (char *)TC_getenv( "TC_ROOT" );
 
    if(tcRoot == NULL || strlen( tcRoot ) == 0) {
 
        TC_write_syslog(" \n Error: TC_ROOT -- environment variable is not found\n");
 
    }
    tc_strcpy(command_mail, tcRoot);
    tc_strcat(command_mail, "\\bin\\tc_mail_smtp");
 
    ERROR_CALL(AOM_ask_value_string(currenttask, "current_name", &nametask));
    ERROR_CALL(AOM_ask_value_string(currenttask, "parent_name", &process_name));
    ERROR_CALL(AOM_ask_value_string(currenttask, "object_desc", &intstruction));
 
    Overview_details[0] = nametask;
    Overview_details[1] = process_name;
    Overview_details[2] = "";
    Overview_details[3] = intstruction;
 

    TC_write_syslog("\n current Task :%s", nametask);
    TC_write_syslog("\n process_name :%s", process_name);
    TC_write_syslog("\n comments :%s", comments);
 
    ERROR_CALL(AOM_ask_value_tag(root_task, "owning_user", &process_owner));
 
    ERROR_CALL(SA_ask_user_identifier2(process_owner, &s_Reviewer_Id));
    ERROR_CALL(EPM_get_user_email_addr(process_owner, &mailaddrs));
    if(mailaddrs != NULL &&  strlen( mailaddrs ) != 0){
 
        tc_strcat(command_mail, " -to=");
        tc_strcat(command_mail, mailaddrs);
        TC_write_syslog("\n -To= :%s", mailaddrs);
 
    }
 
    tc_strcat(command_mail, " -subject=");
    tc_strcat(command_mail, "\"");
    tc_strcat(command_mail, "Related Objects Notification:");
    tc_strcat(command_mail, nametask);
    tc_strcat(command_mail, "\"");
    TC_write_syslog("\n -subject= :%s", nametask);
 

    PREF_initialize();
    PREF_ask_search_scope(&scope);
    PREF_set_search_scope(TC_preference_site);
    PREF_ask_value_count("Mail_server_name", &pref_count );
    PREF_ask_char_value("Mail_server_name", 0, &server_name );
 
    tc_strcat(command_mail, " -server=");
    tc_strcat(command_mail, server_name);
    TC_write_syslog("\n -server= :%s", server_name);
    tc_strcat(command_mail, " -port=25");
    TC_write_syslog("\n -port=25");
 
    mail_file_name = TDW_MailNotification_Format(vPartswithNoRelatedObjects, Overview_details);
    tc_strcat(command_mail, " -body=");
 
    tc_strcat(command_mail, mail_file_name);
 
    TC_write_syslog("\n Command is :%s", command_mail);
    system(command_mail);
    remove(mail_file_name);
    return ITK_ok;
}
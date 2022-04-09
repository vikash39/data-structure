#include<iostream>
#include<vector>
#include<string>

using namespace std;

int main() {
    string sMailBodyFile = "";
    
    string sMsg = "\nErrors while updating cost data for the following part number(s):\n\n";
	string sErpSysName = "\n erpSystemName Part Number\n";
	string sLine = "\n-----------------------------------------------------\n";
    
   std::vector<string> a = {"Part1","part2","part3","part4","part5"};
   
   string mailBody = "";
   
      for(int i=0; i < a.size(); i++){
          
         // mailBody =mailBody.append(a.at(i));
         mailBody = mailBody+"\n"+a.at(i);
      }
      sMailBodyFile = sMsg+sErpSysName+sLine+mailBody+sLine;
      std::cout <<sMailBodyFile  << endl;
      
      char buffer[100];
      
			string sMsgs = "\nErrors while updating cost data for the following part number(s):\n\n";

			sprintf(buffer, " %s \t  %s ", "NA - BPCS 8.2 Auburn Hills Part Number","TI Product");
			string sLines = "\n--------------------------------------------------------------------------------\n";
			string str(buffer);
			
			string mailBodys = sMsgs+str+sLines;
      cout<<mailBodys<<endl;
      
  
   return 0;
}


//--------------------------------------------------------------------------------------

/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    struct StandardCostResponse
{ 
    std::string costID;
    std::string itemID;
    std::string itemRevID;
    std::string bpcsItemID;
    std::string importStatus;
    std::string reasonForFailure;
};

            vector<StandardCostResponse> partsErrored;
            
            partsErrored.push_back(StandardCostResponse());
            partsErrored[0].costID="123";
            partsErrored[0].itemID="00035";
            partsErrored[0].itemRevID="AA";
            partsErrored[0].bpcsItemID="00054";
            partsErrored[0].importStatus="Fail";
            partsErrored[0].reasonForFailure="TI:ERROR: While updating attributes value in tc";
            
            partsErrored.push_back(StandardCostResponse());
            partsErrored[1].costID="123";
            partsErrored[1].itemID="00039";
            partsErrored[1].itemRevID="AA";
            partsErrored[1].bpcsItemID="00085";
            partsErrored[1].importStatus="Fail";
            partsErrored[1].reasonForFailure="TI:ERROR: While updating attributes value in tc";
            
            
            char buffer[100];
            string sMailBodyFile="";
    		string sMsg = "\nErrors while updating cost data for the following part number(s):\n\n";

			sprintf(buffer, " %s \t  %s ", "NA - BPCS 8.2 Auburn Hills Part Number","TI Product");
			string str(buffer);
			string sLine = "\n--------------------------------------------------------------------------------\n";

			vector<StandardCostResponse>::iterator errItr;
			string sErrorStr ="";
			for( errItr=partsErrored.begin();errItr != partsErrored.end();errItr++) {
				string sCostId =(*errItr).costID;
				string sItemId =(*errItr).itemID;
				string sItemRevId =(*errItr).itemRevID;
				string sbpcsId = (*errItr).bpcsItemID;
				string sImportStatus = (*errItr).importStatus;
				string sReasonForFailure = (*errItr).reasonForFailure;

				sErrorStr =sErrorStr+"\n"+ "costID = "+sCostId+"\n"+"itemID ="+sItemId+"\n"+"itemRevID = "+sItemRevId+"\n"+"bpcsItemID = "+sbpcsId+"\n"+"importStatus = "+sImportStatus+"\n"+"reasonForFailure = "+sReasonForFailure+"\n";
				
			}
			
			sMailBodyFile = sMsg+str+sLine+sErrorStr+sLine;
			
			cout<<sMailBodyFile<<endl;

    return 0;
}

